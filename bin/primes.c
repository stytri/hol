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

#define UINTMAX_BIT  (CHAR_BIT * sizeof(uintmax_t))

//------------------------------------------------------------------------------

static inline uintmax_t
sqrtumax(
	uintmax_t v
) {
	uintmax_t r  = 0;
	uintmax_t b = (uintmax_t)1 << (UINTMAX_BIT - 2);

	while(b > v) {
		b >>= 2;
	}

	while(b != 0) {
		uintmax_t const rb = r + b;
		if(v >= rb) {
			uintmax_t const b1 = b << 1;
			r += b1;
			v -= rb;
		}

		r >>= 1;
		b >>= 2;
	}

	return r;
}

//------------------------------------------------------------------------------

static FILE *outf = NULL;

static struct array primes = ARRAY();

static const char index_suffix[][3] = {
	/* 0 */ "th",
	/* 1 */ "st",
	/* 2 */ "nd",
	/* 3 */ "rd",
	/* 4 */ "th",
	/* 5 */ "th",
	/* 6 */ "th",
	/* 7 */ "th",
	/* 8 */ "th",
	/* 9 */ "th"
};

//------------------------------------------------------------------------------

static inline bool
add_prime(
	uintmax_t v
) {
	bool status = true;

	uintmax_t *const p = array_push(uintmax_t, &primes);
	if(likely(p)) {
		*p = v;

	} else {
		size_t      const i  = primes.len;
		char const *const th = index_suffix[i % 10];
		errorf("unable to add %zu%s prime %"PRIuMAX, i, th, v);
		status = false;
	}

	return status;
}

static bool
is_prime__foreach(
	uintmax_t v
) {
	uintmax_t lim = sqrtumax(v);

	FOREACH(uintmax_t, p, &primes,
		uintmax_t const u = *p;
		if(u > lim) return true;

		uintmax_t const m = (v % u);
		if(m == 0) return false;
	);

	return true;
}

static bool
is_prime__for(
	uintmax_t v
) {
	uintmax_t const lim = sqrtumax(v);

	for(size_t i = 1; i < primes.len; ++i) {
		uintmax_t const u = *array_at(uintmax_t, &primes, i);
		if(u > lim) return true;

		uintmax_t const m = (v % u);
		if(m == 0) return false;
	}

	return true;
}

//------------------------------------------------------------------------------

static int
process (
	bool    (*const is_prime)(
		uintmax_t v
	),
	uintmax_t const range_min,
	uintmax_t const range_max,
	uintmax_t const ends_with,
	size_t          n_primes,
	bool      const as_hex,
	int       const timed,
	bool      const list
) {
	bool status = (n_primes < SIZE_MAX) ? (
		array_reserve(uintmax_t, &primes, n_primes)
	):(
		array_reserve(uintmax_t, &primes, SIZE_BIT)
	);
	if(!status) {
		return status;
	}

	char const* const fmt = as_hex ? "%"PRIxMAX"\n" : "%"PRIuMAX"\n";

	uintmax_t v = 0;

	if(as_hex) {
		if(ends_with > 0) {
			v = 1;
			for(uintmax_t t = ends_with; t != 0; t >>= 4) {
				v <<= 4;
			}
		}

	} else {
		if(ends_with > 0) {
			v = 1;
			for(uintmax_t t = ends_with; t != 0; t /= 10) {
				v *= 10;
			}
		}
	}

	uintmax_t const end_digits = v;

	static uintmax_t const primer[] = { 2, 3, 5, 7, 11 };
	static size_t    const n_primer = sizeof(primer) / sizeof(primer[0]);
	for(size_t i = 0; n_primer > i; ++i) {
		v = primer[i];
		if(v > range_max) {
			n_primes = 0;
			break;
		}

		status = add_prime(v);
		if(unlikely(!status)) {
			goto error_return;
		}

		if(v >= range_min) {
			if(end_digits > 0) {
				uintmax_t const ending = v % end_digits;
				if(ending != ends_with) {
					continue;
				}
			}

			if(list && unlikely(fprintf(outf, fmt, v) < 0)) {
				errorf("unable to write to output");
				status = false;
				goto error_return;
			}

			--n_primes;
			if(n_primes == 0) {
				break;
			}
		}
	}
	if(n_primes > 0) {
		uintmax_t last_v = v;
		unsigned  step   = 2;
		bool      paired = false;
		struct timespec t1;
		timestamp(timed, &t1);
		if(range_min > 0) {
			for(v += step; (v > last_v) && (v < range_min); v += step) {
				if(is_prime(v)) {
					last_v = v;

					status = add_prime(v);
					if(unlikely(!status)) {
						goto error_return;
					}

					step   =  paired ? 4 : 2;
					paired = !paired;
				} else {
					step   = 2;
					paired = false;
				}
			}

			if(end_digits > 0) {
				for(; (v > last_v) && (v <= range_max); v += step) {
					if(is_prime(v)) {
						last_v = v;

						status = add_prime(v);
						if(unlikely(!status)) {
							goto error_return;
						}

						uintmax_t const ending = v % end_digits;
						if(ending != ends_with) {
							continue;
						}

						if(list && unlikely(fprintf(outf, fmt, v) < 0)) {
							errorf("unable to write to output");
							status = false;
							goto error_return;
						}

						--n_primes;
						if(unlikely(n_primes == 0)) {
							break;
						}

						step   =  paired ? 4 : 2;
						paired = !paired;
					} else {
						step   = 2;
						paired = false;
					}
				}

			} else /* if(end_digits > 0) */ {
				for(; (v > last_v) && (v <= range_max); v += step) {
					if(is_prime(v)) {
						last_v = v;

						status = add_prime(v);
						if(unlikely(!status)) {
							goto error_return;
						}

						if(list && unlikely(fprintf(outf, fmt, v) < 0)) {
							errorf("unable to write to output");
							status = false;
							goto error_return;
						}

						--n_primes;
						if(unlikely(n_primes == 0)) {
							break;
						}

						step   =  paired ? 4 : 2;
						paired = !paired;
					} else {
						step   = 2;
						paired = false;
					}
				}
			}

		} else /* if(range_min > 0) */ {
			if(end_digits > 0) {
				for(v += step; (v > last_v) && (v <= range_max); v += step) {
					if(is_prime(v)) {
						last_v = v;

						status = add_prime(v);
						if(unlikely(!status)) {
							goto error_return;
						}

						uintmax_t const ending = v % end_digits;
						if(ending != ends_with) {
							continue;
						}

						if(list && unlikely(fprintf(outf, fmt, v) < 0)) {
							errorf("unable to write to output");
							status = false;
							goto error_return;
						}

						--n_primes;
						if(unlikely(n_primes == 0)) {
							break;
						}

						step   =  paired ? 4 : 2;
						paired = !paired;
					} else {
						step   = 2;
						paired = false;
					}
				}

			} else /* if(end_digits > 0) */ {
				for(v += step; (v > last_v) && (v <= range_max); v += step) {
					if(is_prime(v)) {
						last_v = v;

						status = add_prime(v);
						if(unlikely(!status)) {
							goto error_return;
						}

						if(list && unlikely(fprintf(outf, fmt, v) < 0)) {
							errorf("unable to write to output");
							status = false;
							goto error_return;
						}

						--n_primes;
						if(unlikely(n_primes == 0)) {
							break;
						}

						step   =  paired ? 4 : 2;
						paired = !paired;
					} else {
						step   = 2;
						paired = false;
					}
				}
			}
		}
		if(timed) {
			struct timespec t2;
			timestamp(timed, &t2);
			time_interval(&t1, &t2, &t2);
			time_per(&t2, primes.len, &t1);
			print_time_interval(t2.tv_sec, t2.tv_nsec);
			putstr(" = ");
			print_time_interval(t1.tv_sec, t1.tv_nsec);
			puts(" per prime");
		}
	}
error_return:
	array_clear(&primes);

	return status;
}

//------------------------------------------------------------------------------

#ifndef NDEBUG
int main(void)
{
	static char *argv[] = {
		"primes",
		"-I",
		NULL
	};
	static int argc = (sizeof(argv) / sizeof(argv[0])) - 1;
#else
int main(int argc, char *argv[argc + 1])
{
#endif
	static struct optget options[] = {
		{  0, "usage: %s [options] [all|NUM...]", NULL },
		{  0, "options:",                         NULL },
		{  1, "-h, --help",                       "display help" },
		{  2, "-o, --output FILE",                "send output to FILE" },
		{  3, "-r, --range MIN MAX",              "list primes between MIN and MAX" },
		{  4, "-e, --ends-with DIGITS",           "list only primes ending in DIGITS" },
		{  5, "-x, --hex",                        "print values in hexadecimal"},
		{  6, "-d, --dec",                        "print values in decimal"},
		{ 11, "-f, --for",                        "use for loop"},
		{ 12, "-F, --foreach",                    "use foreach"},
		{ 19, "-n, --no-output",                  "do not list prime numbers"},
		{ 80, "-T, --utc-time",                   "timed execution" },
#ifdef TIMESTAMP_REALTIME
		{ 81, "    --realtime",                   NULL },
#endif
#ifdef TIMESTAMP_MONOTIME
		{ 82, "    --monotime",                   NULL },
#endif
#ifdef TIMESTAMP_CPU_TIME
		{ 83, "    --cpu-time",                   NULL },
#endif
		{  0, "NUM",                              "NUMber of primes to list" },
		{  0, "all",                              "list ALL primes" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[1]));

	int exit_status = EXIT_SUCCESS;

	outf = stdout;

	size_t    n_primes  = 0;
	uintmax_t ends_with = 0;
	uintmax_t range_min = 0;
	uintmax_t range_max = UINTMAX_MAX;
	bool      as_hex    = false;
	int       timed     = NO_TIMESTAMP;
	bool      list      = true;
	bool    (*is_prime)(uintmax_t v) = is_prime__foreach;

	for(int i = 1; i < argc; ) {
		char const *args = argv[i++];
		char const *argp = NULL;

		do {
			int argn   = argc - i;
			int params = 0;

			switch(optget(n_options - 3, options + 2, &argp, args, argn, &params)) {
			case 0: // NUM "args"
				if(streq(args, "all")) {
					n_primes = SIZE_MAX;
				} else {
					n_primes = streval(args, NULL, 0);
					if(n_primes == 0) {
						goto invalid_option;
					}
				}
				if(0 > process(is_prime, range_min, range_max, ends_with, n_primes, as_hex, timed, list)) {
					exit_status = EXIT_FAILURE;
					goto end;
				}
				break;
			case 1:
				optuse(n_options, options, argv[0], stdout);
				goto end;
			case 2:
				if(outf != stdout) {
					fclose(outf);
				}
				outf = fopen(argv[i], "w");
				if(!outf) {
					errorf("unable to open output file: %s", argv[i]);
					exit_status = EXIT_FAILURE;
					goto end;
				}
				list = true;
				break;
			case 3:
				range_min = streval(argv[i], NULL, 0);
				if(streq(argv[i + 1], "max")) {
					range_max = UINTMAX_MAX;
				} else {
					range_max = streval(argv[i + 1], NULL, 0);
				}
				if((2 > range_max)
					|| (range_min >= range_max)
				) {
					goto invalid_option;
				}
				break;
			case 4:
				ends_with = streval(argv[i], NULL, 0);
				if(ends_with == 0) {
					goto invalid_option;
				}
				break;
			case 5:
				as_hex = true;
				break;
			case 6:
				as_hex = false;
				break;
			case 11:
				is_prime = is_prime__for;
				break;
			case 12:
				is_prime = is_prime__foreach;
				break;
			case 19:
				list = false;
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
			invalid_option:
				errorf("\ninvalid option: %s\n", args);
				optuse(n_options, options, argv[0], stderr);
				exit_status = EXIT_FAILURE;
				goto end;
			}

			i += params;

		} while(argp)
			;
	}

	if(0 == n_primes) {
		n_primes = SIZE_MAX;

		if(0 > process(is_prime, range_min, range_max, ends_with, n_primes, as_hex, timed, list)) {
			exit_status = EXIT_FAILURE;
		}
	}
end:
	if(outf != stdout) {
		fclose(outf);
	}

	return exit_status;
}
