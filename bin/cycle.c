/*
MIT License

Copyright (c) 2023 Tristan Styles

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and atsociated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//------------------------------------------------------------------------------

#include "process.h"
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

static FILE       *in;
static char const *in_name   = "";
static bool        in_ispipe = false;

//------------------------------------------------------------------------------

static FILE       *out;
static char const *out_name   = "";
static bool        out_ispipe = false;

static inline int
print(
	char const *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vfprintf(out, fmt, va);
	va_end(va);
	return r;
}

//------------------------------------------------------------------------------

static int             timed = NO_TIMESTAMP;
static struct timespec t1, t2;

static bool report_info = false;

static size_t word_count = 0;

static bool restarting = false;

static size_t    cylim = 64;
static size_t    cymin = 63;
static uint64_t *cyref;
static size_t    cysiz =  0;
static uint64_t *cybuf;
static size_t    cylen =  0;
static size_t    cycle =  0;

static ALWAYS_INLINE bool
is_cyclic(
	uint64_t u,
	size_t   size,
	size_t   bits,
	size_t   n
) {
	(void)size;
	(void)bits;

	if(cysiz < cylim) {
		cyref[cysiz++] = u;
	} else if(cylen == 0) {
		if(u == cyref[0]) {
			cybuf[cylen++] = u;
			cycle = n;
		}
	} else if(u != cyref[cylen]) {
		bool recycle = false;
		for(size_t i = 1; i < cylen; i++) {
			recycle = cybuf[i] == cyref[0];
			if(recycle) {
				for(size_t j = i; j < cylen; j++) {
					recycle = cybuf[j] == cyref[j];
					if(!recycle) {
						break;
					}
				}
				if(recycle) {
					cycle += i;
					cylen -= i;
					memmove(cybuf, cybuf + i, cylen * sizeof(cybuf[0]));
					i = 0;
				}
			}
		}
		if(!recycle) {
			cycle = cylen = 0;
		}
	} else if(cylen < cymin) {
		cybuf[cylen++] = u;
	} else {
		int cydigits = 0;
		for(size_t i = 1; i < cylen; i++) {
			int nd = ndigits(cyref[i]);
			if(cydigits < nd) {
				cydigits = nd;
			}
		}
		print("cycle detected at %zu iterations\n", cycle);
		for(size_t i = 1; i < cylen; i++) {
			print("%*"PRIu64" : %-*"PRIu64"\n", cydigits, cyref[i], cydigits, cybuf[i]);
		}
		return true;
	}

	word_count++;
	if(at_capacity(word_count) && (word_count >= 1024)) {
		if(report_info) {
			timestamp(timed, &t2);
			static const char     prefix[] = { 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };
			static const size_t n_prefixes = sizeof(prefix) / sizeof(prefix[0]);
			size_t i = 0, w = word_count;
			do {
				w /= 1024;
			} while((w >= 1024) && (++i < (n_prefixes-1)))
				;
			fprintf(stderr, "%3zu%ci words", w, prefix[i]);
			if(timed) {
				fputs(" processed in ", stderr);
				time_interval(&t1, &t2, &t2);
				fprint_time_interval(stderr, t2.tv_sec, t2.tv_nsec);
			}
			fputc('\n', stderr);
		}
		if(restarting && (word_count >= cylim)) {
			cycle = cylen = cysiz = 0;
		}
	}
	return false;
}

//------------------------------------------------------------------------------

#ifndef NDEBUG
int
main(
	int   argc,
	char *argv__actual[]
) {
	char *argv[] = {
		argv__actual[0],
		"-I",
		NULL
	};
	argc = (sizeof(argv) / sizeof(argv[0])) - 1;
#else
int
main(
	int   argc,
	char *argv[]
) {
#endif
	static struct optget options[] = {
		{  0, "usage: %s [options] [BITS] [FILE]", NULL },
		{  0, "options:",                  NULL },
		{  1, "-h, --help",                "display help" },
		{  2, "-o, --output FILE",         "write output to FILE" },
		{  3, "-b, --buffer-size SIZE",    "set buffer SIZE" },
		{  4, "-c, --cycle-length LENGTH", "detect cycles of LENGTH words" },
		{  5, "-r, --restarting",          "cycle detection restarts on power-of-two boundaries" },
		{ 98, "-E, --info-to-stderr",      "output information to stderr" },
		{ 80, "-T, --utc-time",            "timed execution" },
#ifdef TIMESTAMP_REALTIME
		{ 81, "    --realtime",          NULL },
#endif
#ifdef TIMESTAMP_MONOTIME
		{ 82, "    --monotime",          NULL },
#endif
#ifdef TIMESTAMP_CPU_TIME
		{ 83, "    --cpu-time",          NULL },
#endif
		{ 99, "-I, --ignore-interrupts",   "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	if(argc == 1) {
		optuse(n_options, options, argv[0], stdout);
		return 0;
	}

	in  = stdin;
	out = stdout;

	size_t sizeof_buffer     = BUFSIZE;
	bool   ignore_interrupts = false;

	int argi = 1;
	while((argi < argc) && (*argv[argi] == '-')) {
		char const *args = argv[argi++];
		char const *argp = NULL;
		do {
			int argn   = argc - argi;
			int params = 0;
			switch(optget(n_options - 2, options + 2, &argp, args, argn, &params)) {
			case 1:
				optuse(n_options, options, argv[0], stdout);
				return 0;
			case 2:
				if(out != stdout) {
					out_ispipe ? pclose(out) : fclose(out);
				}
				out_name   = argv[argi];
				out_ispipe = (*out_name == '=');
				out_name  += out_ispipe;
				out        = out_ispipe ? popen(out_name, "wb") : fopen(out_name, "wb");
				if(!out) {
					perror(out_name);
					fail();
				}
				break;
			case 3:
				sizeof_buffer = streval(argv[argi], NULL, 0);
				if(sizeof_buffer == 0) sizeof_buffer = BUFSIZE;
				break;
			case 4:
				cylim = streval(argv[argi], NULL, 0);
				if(cylim == 0) cylim = 64;
				cymin = cylim - 1;
				break;
			case 5:
				restarting = true;
				break;
			case 98:
				report_info = true;
				break;
			case 99:
				ignore_interrupts = true;
				break;
			case 80: timed = TIMESTAMP_UTC_TIME; break;
#ifdef TIMESTAMP_REALTIME
			case 81: timed = TIMESTAMP_REALTIME; break;
#endif
#ifdef TIMESTAMP_MONOTIME
			case 82: timed = TIMESTAMP_MONOTIME; break;
#endif
#ifdef TIMESTAMP_CPU_TIME
			case 83: timed = TIMESTAMP_CPU_TIME; break;
#endif
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				return EXIT_FAILURE;
			}
			argi += params;
		} while(argp)
			;
	}

	cyref = calloc(cylim, sizeof(*cyref));
	cybuf = calloc(cylim, sizeof(*cybuf));
	if(!(cyref || cybuf)) {
		perror();
		fail();
	}

	size_t const B = ((argi < argc) && isdigit(*argv[argi])) ? streval(argv[argi++], NULL, 0) : 64;
	size_t const Z = sizeof_underlying_data_type(B);
	size_t const N = sizeof_buffer / Z;

	void *buffer = calloc(N, Z);
	if(!buffer) {
		perror();
		fail();
	}

	if(ignore_interrupts) {
		signal(SIGINT, SIG_IGN);
	} else {
		signal(SIGINT, signal_handler);
	}
#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	do {
		if(argi < argc) {
			if(in != stdin) {
				in_ispipe ? pclose(in) : fclose(in);
			}
			in_name   = argv[argi++];
			in_ispipe = (*in_name == '=');
			in_name  += in_ispipe;
			in        = in_ispipe ? popen(in_name, "rb") : fopen(in_name, "rb");
			if(!in) {
				perror(in_name);
				fail();
			}
		}
		bool no_cycle = true;
		PROCESS(in, is_cyclic, no_cycle = false, buffer, Z, N, B);
		if(no_cycle) {
			print("no cycle detected\n");
		}
	} while(!gSignal && (argi < argc))
		;
	return 0;
}
