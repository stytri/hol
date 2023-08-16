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
#include <stdarg.h>
#include <errno.h>
#include <math.h>
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

static uint64_t last_value = 0;
static uint64_t sum[2]     = { 0 };

static size_t word_count          = 0;
static size_t even_odd  [2]       = { 0 };
static size_t bit_count [2]       = { 0 };
static size_t pop_count    [64+1] = { 0 };
static size_t bit_diff  [2][64+1] = { { 0 } };
static size_t histogram [2][64+1] = { { 0 } };
static size_t run_length[2][64+1] = { { 0 } };
static size_t run_count    [64+1] = { 0 };
static size_t hexogram     [16]   = { 0 };

static void
output_statistics(
	size_t const n
) {
	size_t b = bit_count[0] + bit_count[1];
	int    d = ndigits(b);
	if(d < 3) d = 3;

	uint64_t avg = UINT64_C(~0) >> (64 - n);
	uint64_t avg_quot[2], avg_rem = uudivu(sum[1], sum[0], word_count, &avg_quot[1], &avg_quot[0]);
	print("words   : %*zu\n",             d, word_count);
	print("even    : %*zu  odd : %*zu\n", d, even_odd[0], d, even_odd[1]);
	print("bits    : %*zu\n",             d, b);
	print("zeroes  : %*zu  ones: %*zu\n", d, bit_count[0], d, bit_count[1]);
	print("average : %"PRIu64".%d\n", avg_quot[0], 5 * (avg_rem >= (word_count / 2)));
	print("expected: %"PRIu64".%d\n", (avg - 1) / 2, 5 * (int)((avg - 1) & 1));

	print("bit: ");
	print("%*s | "    , d, "pop");
	print("%*s %*s | ", d+(d/2)-1, "bit", d-(d/2)+1, " ");
	print("%*s %*s | ", d+(d/2)-1, "dif", d-(d/2)+1, " ");
	print("%*s %*s | ", d+(d/2)-1, "run", d-(d/2)+1, " ");
	print("%-*s\n" , d, "cnt");
	print("     ");
	print("%*s | "     , d, " ");
	print("%*s %-*s | ", d, "0", d, "1");
	print("%*s %-*s | ", d, "pop", d, "bit");
	print("%*s %-*s | ", d, "0", d, "1");
	print("\n");
	for(size_t i = 0; i < (n+1); i++) {
		print("[%2zu] "      , i);
		print("%*zu | "      , d, pop_count    [i]);
		print("%*zu %-*zu | ", d, histogram [0][i], d, histogram [1][i]);
		print("%*zu %-*zu | ", d, bit_diff  [0][i], d, bit_diff  [1][i]);
		print("%*zu %-*zu | ", d, run_length[0][i], d, run_length[1][i]);
		print("%-*zu\n"      , d, run_count    [i]);
	}

	print("hex:\n");
	for(size_t i = 0; i < 16; i++) {
		print("[%2zu] %*zu\n", i, d, hexogram[i]);
	}
}

static ALWAYS_INLINE bool
process_word(
	uint64_t u,
	size_t   size,
	size_t   bits,
	size_t   n
) {
	(void)size;
	(void)n;

	bool     b, q;
	size_t   i, r, c, h, x;
	uint64_t d, m;

	word_count++;

	sum[0] += u;
	sum[1] += (sum[0] < u);

	even_odd[u&1]++;

	i = popcount(u);
	pop_count[i]++;

	d = u ^ last_value;
	i = popcount(d);
	bit_diff[0][i]++;

	i = 0;
	m = 1;

	x = u & 0xf;
	hexogram[x]++;
	h = 4;

	q = b = !!(u & m);
	r = 1;
	c = 1;

	bit_count[b]++;
	histogram[b][i]++;

	b = d & m;
	bit_diff[1][i] += b;

	for(i++, m <<= 1; i < bits; i++, m <<= 1) {
		if((i & 3) == 0) {
			x = (u >> h) & 0xf;
			hexogram[x]++;
			h += 4;
		}

		b = !!(u & m);
		if(q != b) {
			run_length[q][r]++;
			q = b;
			r = 0;
			c++;
		}
		r++;

		bit_count[b]++;
		histogram[b][i]++;

		b = d & m;
		bit_diff[1][i] += b;
	}

	run_length[q][r]++;
	run_count[c]++;

	last_value = u;

	if(report_info) {
		if(at_capacity(word_count) && (word_count >= 1024)) {
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
		{  0, "usage: %s [OPTIONS] [BITS] [FILE]...", NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "write output to FILE" },
		{  3, "-b, --buffer-size SIZE",  "set buffer SIZE" },
		{ 98, "-E, --info-to-stderr",    "output information to stderr" },
		{ 80, "-T, --utc-time",          "timed execution" },
#ifdef TIMESTAMP_REALTIME
		{ 81, "    --realtime",          NULL },
#endif
#ifdef TIMESTAMP_MONOTIME
		{ 82, "    --monotime",          NULL },
#endif
#ifdef TIMESTAMP_CPU_TIME
		{ 83, "    --cpu-time",          NULL },
#endif
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
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

	size_t const B = ((argi < argc) && str_is(argv[argi], isdigit)) ? streval(argv[argi++], NULL, 0) : 64;
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
	timestamp(timed, &t1);
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
		PROCESS(in, process_word, , buffer, Z, N, B);
	} while(!gSignal && (argi < argc))
		;
	if(word_count > 0) {
		output_statistics(B);
	}
	return 0;
}
