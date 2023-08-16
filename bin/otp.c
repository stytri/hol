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
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

static void
usage(
	FILE *out
) {
	fputs("usage: otp [options] [COL-by-ROW] [PAGES [ALPHABET]]\n", out);
	fputs("options:\n", out);
	fputs("\t--codeform    output a form for encoding/decoding\n", out);
	fputs("\t--formfeed    output form feed between pages\n", out);
	fputs("\t--help        output this help message\n", out);
	fputs("\nThe random number stream is read as 64-bit binary from stdin.\n", out);
}

//------------------------------------------------------------------------------

struct rand {
	uint64_t x;
	size_t   m;
};
#define RAND  struct rand

static inline size_t
rand_next(
	RAND   *r,
	size_t  n,
	size_t  m
) {
	if(r->m == 0) {
		if(!fread(&r->x, 1, sizeof(r->x), stdin)) {
			perror();
			fail();
		}
		r->m = m;
	}
	size_t x = r->x % n;
	r->x /= n;
	r->m--;
	return x;
}

//------------------------------------------------------------------------------

static size_t
get_letters_per_word(
	size_t n
) {
	size_t m = 0;
	for(uint64_t v = UINT64_MAX; v >= n; v /= n) {
		m++;
	}
	return m;
}

//------------------------------------------------------------------------------

static void
print_alphabet(
	char const *A,
	size_t      N
) {
	for(size_t k = 0; k < N; k++) {
		putchar(' ');
		putchar(A[k]);
	}
}

static void
print_reversed_alphabet(
	char const *A,
	size_t      N
) {
	for(size_t k = 0, M = N - 1; k < N; k++) {
		putchar(' ');
		putchar(A[M-k]);
	}
}

static void
print_randomized_alphabet(
	char const *A,
	size_t      N,
	size_t      M,
	char       *a,
	size_t      i,
	RAND       *r
) {
	for(size_t n = N; n-- != 1; ) {
		size_t x = rand_next(r, N, M) % n;
		char   t = a[x];
		a[x] = a[n];
		a[n] = t;
	}
	putchar(' ');
	putchar(A[N-1-i]);
	putchar(' ');
	for(size_t j = 0; j < N; j++) {
		putchar(' ');
		putchar(a[j]);
	}
	putstr("  ");
	putchar(A[i]);
}

static void
print_code_form(
	size_t      N,
	size_t      x
) {
	putchar('\n');
	for(size_t i = 0, j, k; ; i++) {
		for(j = 0; j < x; j++) {
			if(j > 0) putstr("     ");
			putstr("   ");
			if((i != 1) && (i != 5)) {
				for(k = 0; k < N; k++) {
					putchar('+');
					putchar('-');
				}
			} else {
				for(k = 0; k < N; k++) {
					putchar('+');
					putchar('=');
				}
			}
			putchar('+');
		}
		putchar('\n');
		if(i == 6) break;
		for(j = 0; j < x; j++) {
			if(j > 0) putstr("     ");
			putstr("   ");
			for(k = 0; k < N; k++) {
				putchar('|');
				putchar(' ');
			}
			putchar('|');
		}
		putchar('\n');
	}
}

static char const *
alphabet(
	char const *cs
) {
	static const char international_morse_code[] = "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if(streq(cs, ":alpha:")) return international_morse_code+11;
	if(streq(cs, ":alnum:")) return international_morse_code+1;
	if(streq(cs, ":morse:")) return international_morse_code;
	return cs;
}

//------------------------------------------------------------------------------

#ifndef NDEBUG
int
main(
	int    argc,
	char **argv
) {
	char *argv__debug[] = {
		argv[0],
		"1", ":alpha:", "[0]",
		NULL
	};
	argv = argv__debug;
	argc = (sizeof(argv__debug) / sizeof(argv__debug[0])) - 1;
#else
int
main(
	int   argc,
	char *argv[]
) {
#endif
	size_t x = 1, y = 1;
	bool  cf = false;
	int   ff = '\n';
	for(; (argc > 1) && (strncmp(argv[1], "--", 2) == 0); argc--, argv++) {
		char const *args = argv[1] + 2;
		if(streq(args, "help")) {
			usage(stdout);
			exit();
		} else if(streq(args, "codeform")) {
			cf = true;
		} else if(streq(args, "formfeed")) {
			ff = '\f';
		} else {
			usage(stderr);
			fail();
		}
	}
#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
#endif
	size_t xx, yy;
	if(sscanf(argv[1], "%zu-by-%zu", &xx, &yy) == 2) {
		argc--, argv++;
		x = xx, y = yy;
	}
	if(argc > 5) {
		usage(stderr);
		fail();
	}
	size_t        z = 1;
	size_t const  P = ((argc > 1) ? streval(argv[1], NULL, 0) : 1) * x * y;
	char   const *A = alphabet((argc > 2) ? argv[2] : ":alpha:");
	size_t const  N = strlen(A);
	size_t const  M = get_letters_per_word(N);
	int    const  D = ndigits(P);
	int    const  W = 7 + (2 * N) - (2 * D);
	char         *a = strcpy(malloc(N + 1), A);
	RAND          r = { 0, 0 };
	for(size_t pi = x, p1 = 1, p2 = pi; p1 <= P; p1 += pi, p2 += pi) {
		if(x > 1) {
			if(z == 1) {
				putchar('\n');
			}
			for(size_t j = 0, n = x-1; j < n; j++) {
				printf("%-*zu%*s%*zu  ", D, p1+j, W, "", D, p1+j);
			}
		}
		printf("%-*zu%*s%*zu\n   ", D, p2, W, "", D, p2);
		for(size_t j = 0, n = x-1; j < n; j++) {
			print_alphabet(A, N);
			putstr("         ");
		}
		print_alphabet(A, N);
		putstr("\n\n");
		for(size_t i = 0; i < N; i++) {
			for(size_t j = 0, n = x-1; j < n; j++) {
				print_randomized_alphabet(A, N, M, a, i, &r);
				putstr("   ");
			}
			print_randomized_alphabet(A, N, M, a, i, &r);
			putchar('\n');
		}
		putchar('\n');
		putstr("   ");
		for(size_t j = 0, n = x-1; j < n; j++) {
			print_reversed_alphabet(A, N);
			putstr("         ");
		}
		print_reversed_alphabet(A, N);
		putchar('\n');
		if(cf) {
			print_code_form(N, x);
		}
		for(size_t j = 0, n = x-1; j < n; j++) {
			printf("%-*zu%*s%*zu  ", D, p1+j, W, "", D, p1+j);
		}
		int e = (z < y) ? '\n' : ff;
		printf("%-*zu%*s%*zu\n%c", D, p2, W, "", D, p2, e);
		z = (z % y) + 1;
	}
	return 0;
}
