/*
MIT License

Copyright (c) 2022 Tristan Styles

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

//------------------------------------------------------------------------------

static char *
re_format(
	char const *cs,
	char const *cf,
	size_t     *np
) {
	size_t nf = strlen(cf);
	size_t n  = 0;

	for(char const *ct = cs; (ct = strchr(ct, '%')) != NULL; n++) {
		for(int c = *++ct; c && !isalpha(c); c = *++ct);
	}
	if(np) {
		*np = n;
	}

	size_t m = strlen(cs);
	size_t z = m + (nf * n) + 1;
	char  *s = malloc(z);
	if(!s) {
		perror();
		abort();
	}

	char *t = s;
	for(char const *ct; (ct = strchr(cs, '%')) != NULL; cs = ct) {
		for(int c = *++ct; c && !isalpha(c); c = *++ct);
		n  = ct - cs;
		memcpy(t, cs, n);
		t += n;
		memcpy(t, cf, nf);
		t += nf;
	}
	strcpy(t, cs);

	return s;
}

static void
do_commands(
	int                argi,
	int                argc,
	char              *argv[],
	unsigned long long seed
) {
	while(argi < argc) {
		char  *args = argv[argi++];
		size_t n    = 0;
		char  *fmt  = re_format(args, "ll", &n);

		if(n > 64) {
			fprintf(stderr, "too many field specifiers in: \"%s\"\n", args);
			free(fmt);
			abort();
		}

		size_t m = strlen(args);
		size_t z = (BITS(seed) * n) + m + 1;
		char  *s = malloc(z);
		if(!s) {
			perror();
			free(fmt);
			abort();
		}

		int r = sprintf(s, fmt,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed,
			seed, seed, seed, seed, seed, seed, seed, seed
		);
		if(r < 0) {
			perror(fmt);
			free(fmt);
			free(s);
			abort();
		}

		system(s);
		free(fmt);
		free(s);
	}
}

static XRAND64 R;

static uint64_t
strtoseed(
	char const  *cs,
	char       **endp,
	int          base
) {
	while(isspace(*cs)) cs++;

	if(*cs == '$') {
		static char const s_rand[] = "$RAND";
		static char const s_seed[] = "$SEED";
		static char const s_time[] = "$TIME";

		uint64_t seed = 0;
		size_t   len;

		if(streqn(cs, s_rand, (len = strlen(s_rand)))) {
			seed = xrand64(&R);
		} else if(streqn(cs, s_seed, (len = strlen(s_seed)))) {
			seed = xrandseed();
		} else if(streqn(cs, s_time, (len = strlen(s_time)))) {
			seed = (uint64_t)time();
		} else {
			len = 1;
		}

		for(cs += len; isalnum(*cs); ++cs);
		if(endp) *endp = (char *)cs;
		return seed;
	}

	return strtoulls(cs, endp, base);
}

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
	int   argi = 1;
	char *args;

	uint64_t    M = UINT64_C(~0);
	uint64_t    N = 1;
	char const *F = "0x%.16"PRIx64"\n";

	xseed64(&R, xrandseed());

	while(argi < argc) {
		args = argv[argi];
		if(streq(args, "--64")) {
			M = UINT64_C(~0);
			F = "0x%.16"PRIx64"\n";
			argi++;
		} else if(streq(args, "--32")) {
			M = UINT64_C(~0) >> 32;
			F = "0x%.8"PRIx64"\n";
			argi++;
		} else if(streq(args, "--16")) {
			M = UINT64_C(~0) >> 48;
			F = "0x%.4"PRIx64"\n";
			argi++;
		} else if(streq(args, "--8")) {
			M = UINT64_C(~0) >> 56;
			F = "0x%.2"PRIx64"\n";
			argi++;
		} else if(streq(args, "-D")) {
			F = "%"PRIu64"\n";
			argi++;
		} else if(streq(args, "-S")) {
			argi++;
			if(argi < argc) {
				xinit64(&R, argv[argi]);
				argi++;
			}
		} else if(streq(args, "-K")) {
			argi++;
			if(argi < argc) {
				xkeys64(&R, argv[argi]);
				argi++;
			}
		} else {
			break;
		}
	}

	if(argi < argc) {
		args = argv[argi++];
		bool const is_var = (*args == '$');

		static char const file[] = "$file:";
		if(streqn(args, file, strlen(file))) {
			FILE *in = fopen(args + strlen(file), "r");
			if(!in) {
				perror(args);
				abort();
			}
			for(int c;;) {
				do {
					if((c = fgetc(in)) == EOF) goto done;
				} while(!isgraph(c) || (c == ','))
					;
				ungetc(c, in);

				int r = fscanf(in, "%"PRIu64, &N);
				if(r != 1) goto done;

				do_commands(argi, argc, argv, N & M);
			}
done:
			if(ferror(in)) {
				perror(args);
				fclose(in);
				abort();
			}
			fclose(in);
			return 0;
		}

		N = strtoseed(args, &args, 0);
		switch(*args) {
		default:
			if(is_var) {
				do_commands(argi, argc, argv, N & M);
				return 0;
			}
			if(argi < argc) {
				while(N--) {
					uint64_t seed = xrand64(&R) & M;
					do_commands(argi, argc, argv, seed);
				}
				return 0;
			}
			break;
		case '-':
		case_range: {
				uint64_t const end  = strtoulls(args + 1, &args, 0);
				uint64_t const step = (*args == '+') ? (
					strtoulls(args += 1, &args, 0)
				):(
					1
				);
				if(N < end) {
					for(; N <= end; N += step) {
						do_commands(argi, argc, argv, N & M);
					}
				} else if(N > end) {
					for(; N >= end; N -= step) {
						do_commands(argi, argc, argv, N & M);
					}
				} else {
					do_commands(argi, argc, argv, N & M);
				}
				if(*args == '/') {
					N = strtoseed(args + 1, &args, 0);
					if(*args == '-') goto case_range;
					if(*args == '+') goto case_rangeplus;
					goto case_list;
				}
				return 0;
			}
		case '+':
		case_rangeplus: {
				uint64_t const end  = N + strtoulls(args + 1, &args, 0);
				uint64_t const step = (*args == '+') ? (
					strtoulls(args += 1, &args, 0)
				):(
					1
				);
				if(N < end) {
					for(; N <= end; N += step) {
						do_commands(argi, argc, argv, N & M);
					}
				} else if(N > end) {
					for(; N >= end; N -= step) {
						do_commands(argi, argc, argv, N & M);
					}
				} else {
					do_commands(argi, argc, argv, N & M);
				}
				if(*args == '/') {
					N = strtoseed(args + 1, &args, 0);
					if(*args == '-') goto case_range;
					if(*args == '+') goto case_rangeplus;
					goto case_list;
				}
				return 0;
			}
		case '/':
		case_list: {
				for(;;) {
					do_commands(argi, argc, argv, N & M);
					if(*args != '/') break;

					N = strtoseed(args + 1, &args, 0);
					if(*args == '-') goto case_range;
					if(*args == '+') goto case_rangeplus;
				}
				return 0;
			}
		}
	}

	while(N--) {
		uint64_t const seed = xrand64(&R) & M;
		printf(F, seed);
	}

	return 0;
}
