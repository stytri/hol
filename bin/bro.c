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

#include <hol/holibc.h>
#include <signal.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

static volatile sig_atomic_t gSignal = 0;

static void
signal_handler(
	int signal
) {
	gSignal = signal;
}

//------------------------------------------------------------------------------

static FILE       *in;
static char const *in_name   = "";
static bool        in_ispipe = false;

static inline size_t
ingest(
	uint64_t *w,
	size_t    n
) {
	return fread(w, sizeof(*w), n, in);
}

//------------------------------------------------------------------------------

static FILE       *out;
static char const *out_name   = "";
static bool        out_ispipe = false;

static inline size_t
egest(
	uint64_t const *w,
	size_t          n
) {
	return fwrite(w, sizeof(*w), n, out);
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
	int    argc,
	char **argv
) {
#endif
	static struct optget options[] = {
		{  0, "usage: %s [OPTION]... [START1.]STEP1 [START2.]STEP2 [FILE]",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "output to FILE" },
		{  3, "-b, --buffer SIZE",       "set buffer SIZE" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	size_t bufsize           = BUFSIZE;
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
				bufsize = streval(argv[argi], NULL, 0);
				if(!bufsize) bufsize = BUFSIZE;
				break;
			case 99:
				ignore_interrupts = true;
				break;
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				fail();
			}
			argi += params;
		} while(argp)
			;
	}

	if((argc - argi) < 2) {
		optuse(n_options, options, argv[0], stderr);
		fail();
	}

	char           *s;
	size_t    const start1 = strtoz(argv[argi++], &s, 0) % 64;
	size_t    const step1  = (*s =='.') ? (strtoz(s+1, NULL, 0) % 64) : start1;
	size_t    const start2 = strtoz(argv[argi++], &s, 0) % 64;
	size_t    const step2  = (*s =='.') ? (strtoz(s+1, NULL, 0) % 64) : start2;
	size_t    const buflen = bufsize / sizeof(uint64_t);
	uint64_t *const buffer = calloc(buflen, sizeof(*buffer));
	if(!step1 || !step2 || !buflen) {
		errno = ERANGE;
		perror();
		fail();
	}
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
		for(size_t n; !gSignal && ((n = ingest(buffer, buflen)) > 0);) {
			for(size_t i = 0; !gSignal && (i < n); i++) {
				uint64_t u  = buffer[i];
				uint64_t v  = 0;
				for(size_t j = 0, k1 = start1, k2 = start2; j < 64; j++) {
					uint64_t m1 = UINT64_C(1) << k1;
					uint64_t m2 = UINT64_C(1) << k2;
					k1 = incwrap(k1, step1, 63);
					k2 = incwrap(k2, step2, 63);
					if(u & m1) {
						v |= m2;
					}
				}
				buffer[i] = v;
			}
			if(gSignal) break;
			if(!egest(buffer, n)) {
				perror(out_name);
				fail();
			}
		}
	} while(!gSignal && (argi < argc))
		;
	return 0;
}
