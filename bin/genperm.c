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

static FILE       *out;
static char const *out_name   = "";
static bool        out_ispipe = false;

static inline int
outchar(
	int c
) {
	return fputc(c, out);
}

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

static void
print_permutationN(
	size_t permutation,
	size_t size
) {
	switch(size) {
	case  4: print("0x%2.2"  PRIX8 "\n", genperm4 (permutation)); break;
	case  8: print("0x%6.6"  PRIX32"\n", genperm8 (permutation)); break;
	case 16: print("0x%16.16"PRIX64"\n", genperm16(permutation)); break;
	}
}

static void
print_permutation(
	uint64_t permutation[2],
	size_t   size,
	size_t   index[size]
) {
	uugenperm(permutation[1], permutation[0], size, index);
	for(size_t i = 0; i < size; i++) {
		print("%zu ", index[i]);
	}
	outchar('\n');
}

static size_t
strtoperm(
	char const *cs,
	char      **endp,
	int         base,
	uint64_t   *permutation
) {
	if(permutation) {
		if(*cs == '#') {
			if(endp) *endp = (char *)cs + 1;
			permutation[0] = xrandseed();
			permutation[1] = nlis64(permutation[0], IPOW(UINT64_C(211),8), 11);
		} else {
			char *s;
			strtouus(cs, &s, base, &permutation[1], &permutation[0]);
			if(endp) *endp = s;
		}
		return permutation[0];
	}
	if(*cs == '#') {
		if(endp) *endp = (char *)cs + 1;
		return xrandseed();
	}
	return strtozs(cs, endp, base);
}
#define strtoperm(strtoperm__cs,strtoperm__endp,strtoperm__base,...) \
	(strtoperm)(strtoperm__cs,strtoperm__endp,strtoperm__base,__VA_ARGS__+0)

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
		{  0, "usage: %s [OPTION]... SIZE [PERMUTATION]...",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "output to FILE" },
		{ 10, "-w, --word",              "ouput as a single integer word" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	static char s[256];

	out = stdout;

	bool output_integer_word = false;
	bool ignore_interrupts   = false;

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
			case 10:
				output_integer_word = true;
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

	if(argi == argc) {
		optuse(n_options, options, argv[0], stderr);
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
	if(output_integer_word) {
		size_t       permutation;
		size_t const size  = strtozs(argv[argi++], NULL, 0);
		if(argi == argc) {
			while(!gSignal && (scanf("%255s", s) == 1)) {
				permutation = strtoperm(s, NULL, 0);
				print_permutationN(permutation, size);
			}
		} else while(!gSignal && (argi < argc)) {
			char *args = argv[argi++];
			permutation = strtoperm(args, &args, 0);
			switch(*args) {
			default:
				print_permutationN(permutation, size);
				continue;
			case '-':
			case_range1: {
					size_t const end  = strtoperm(args + 1, &args, 0);
					size_t const step = (*args == '+') ? (
						strtozs(args += 1, &args, 0)
					):(
						1
					);
					if(permutation < end) {
						for(; !gSignal && (permutation < end); permutation += step) {
							print_permutationN(permutation, size);
						}
					} else if(permutation > end) {
						for(; !gSignal && (permutation > end); permutation -= step) {
							print_permutationN(permutation, size);
						}
					} else {
						print_permutationN(permutation, size);
					}
					if(*args == '/') {
						permutation = strtoperm(args + 1, &args, 0);
						if(*args == '-') goto case_range1;
						if(*args == '+') goto case_range1plus;
						goto case_list1;
					}
					continue;
				}
			case '+':
			case_range1plus: {
					size_t const end  = permutation + strtoperm(args + 1, &args, 0);
					size_t const step = (*args == '+') ? (
						strtozs(args += 1, &args, 0)
					):(
						1
					);
					if(permutation < end) {
						for(; !gSignal && (permutation < end); permutation += step) {
							print_permutationN(permutation, size);
						}
					} else if(permutation > end) {
						for(; !gSignal && (permutation > end); permutation -= step) {
							print_permutationN(permutation, size);
						}
					} else {
						print_permutationN(permutation, size);
					}
					if(*args == '/') {
						permutation = strtoperm(args + 1, &args, 0);
						if(*args == '-') goto case_range1;
						if(*args == '+') goto case_range1plus;
						goto case_list1;
					}
					continue;
				}
			case '/':
			case_list1: {
					while(!gSignal) {
						print_permutationN(permutation, size);
						if(*args != '/') break;

						permutation = strtoperm(args + 1, &args, 0);
						if(*args == '-') goto case_range1;
						if(*args == '+') goto case_range1plus;
					}
					continue;
				}
			}
		}
	} else {
		uint64_t      permutation[2];
		size_t  const size  = strtozs(argv[argi++], NULL, 0);
		size_t *const index = calloc(size, sizeof(*index));
		if(argi == argc) {
			while(!gSignal && (scanf("%255s", s) == 1)) {
				strtoperm(s, NULL, 0, permutation);
				print_permutation(permutation, size, index);
			}
		} else while(!gSignal && (argi < argc)) {
			char *args = argv[argi++];
			strtoperm(args, &args, 0, permutation);
			switch(*args) {
			default:
				print_permutation(permutation, size, index);
				continue;
			case '-':
			case_range2: {
					uint64_t const end  = strtoulls(args + 1, &args, 0);
					uint64_t const step = (*args == '+') ? (
						strtoulls(args += 1, &args, 0)
					):(
						1
					);
					if(permutation[0] < end) {
						for(; !gSignal && (permutation[0] < end); permutation[0] += step) {
							print_permutation(permutation, size, index);
						}
					} else if(permutation[0] > end) {
						for(; !gSignal && (permutation[0] > end); permutation[0] -= step) {
							print_permutation(permutation, size, index);
						}
					} else {
						print_permutation(permutation, size, index);
					}
					if(*args == '/') {
						strtoperm(args + 1, &args, 0, permutation);
						if(*args == '-') goto case_range2;
						if(*args == '+') goto case_range2plus;
						goto case_list2;
					}
					continue;
				}
			case '+':
			case_range2plus: {
					uint64_t const end  = permutation[0] + strtoulls(args + 1, &args, 0);
					uint64_t const step = (*args == '+') ? (
						strtoulls(args += 1, &args, 0)
					):(
						1
					);
					if(permutation[0] < end) {
						for(; !gSignal && (permutation[0] < end); permutation[0] += step) {
							print_permutation(permutation, size, index);
						}
					} else if(permutation[0] > end) {
						for(; !gSignal && (permutation[0] > end); permutation[0] -= step) {
							print_permutation(permutation, size, index);
						}
					} else {
						print_permutation(permutation, size, index);
					}
					if(*args == '/') {
						strtoperm(args + 1, &args, 0, permutation);
						if(*args == '-') goto case_range2;
						if(*args == '+') goto case_range2plus;
						goto case_list2;
					}
					continue;
				}
			case '/':
			case_list2: {
					while(!gSignal) {
						print_permutation(permutation, size, index);
						if(*args != '/') break;

						strtoperm(args + 1, &args, 0, permutation);
						if(*args == '-') goto case_range2;
						if(*args == '+') goto case_range2plus;
					}
					continue;
				}
			}
		}
	}

	return 0;
}
