#ifndef HOL_OPTGET_H__INCLUDED
#define HOL_OPTGET_H__INCLUDED 1
/*
MIT License

Copyright (c) 2015 Tristan Styles

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
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

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

//------------------------------------------------------------------------------

#define OPTGET  struct optget const
struct optget {
	int  const  x;
	/* optget() return value:
	**          > 0 : option code
	**            0 : non-option argument,
	**                also used to indicate help-only messages
	**
	** optget() may also return:
	**           -1 : invalid option
	*/
	char const *s;
	/* option specification, and, help
	**
	**        Options are placed at the front of the string,
	**        they always start with -', and, may contain '-'
	**        and alpha-numeric characters. Multiple options
	**        in the same string options are separated by ','.
	**
	**        Following the options are one, or more parameter
	**        place-holders; their only use is to enumerate
	**        required parameters, they are separated by space.
	**        Parameters apply to all options in the same string.
	**
	**        Text in a negative-coded entry is aligned as per
	**        > 0 option text, but is otherwise non-functional.
	**
	**        Text in a 0-coded entry is non-functional.
	*/
	char const *t;
	/* Help text describing the option.
	*/
};

//------------------------------------------------------------------------------

extern int  optget(size_t optc, OPTGET optv[optc], char const **argp, char const *args, int argn, int *params);
extern void optuse(size_t optc, OPTGET optv[optc], char const  *prog, FILE *outf);

//------------------------------------------------------------------------------

#if 0
int
main(
	int   argc,
	char *argv[argc + 1]
) {
	static OPTGET options[] = {
		{ 0, "usage: %s [options] [FILE...]", NULL },
		{ 0, "options:",                      NULL },
		{ 1, "-h, --help",                    "display help" },
		{ 2, "-o, --output FILE",             "output to FILE" },
		{ 3, "-e, --error FILE",              "errors to FILE" },

		{ 0, "FILE",                          "FILE" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	for(int i = 1; i < argc; ) {
		char const *args = argv[i++];
		char const *argp = NULL;

		do {
			int argn   = argc - i;
			int params = 0;

			switch(optget(n_options - 3, options + 2, &argp, args, argn, &params)) {
			case 0: // FILE "args"
				if(!freopen(args, "r", stdin)) {
					perror(args);
					exit(EXIT_FAILURE);
				}
				break;
			case 1:
				optuse(n_options, options, argv[0], stdout);
				break;
			case 2:
				if(!freopen(argv[i], "w", stdout)) {
					perror(argv[i]);
					exit(EXIT_FAILURE);
				}
				break;
			case 3:
				if(!freopen(argv[i], "w", stderr)) {
					perror(argv[i]);
					exit(EXIT_FAILURE);
				}
				break;
			default:
				if(params > 0) {
					fprintf(stderr, "invalid option: %s %s\n", args, argv[i]);
				} else {
					fprintf(stderr, "invalid option: %s\n", args);
				}
				optuse(n_options, options, argv[0], stderr);
				exit(EXIT_FAILURE);
			}

			i += params;

		} while(argp)
			;
	}

done:
	return EXIT_SUCCESS;
}
#endif

//------------------------------------------------------------------------------

#endif//ndef HOL_OPTGET_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_OPTGET_H__IMPLEMENTATION
#undef HOL_OPTGET_H__IMPLEMENTATION

//------------------------------------------------------------------------------

static bool
optget__optcmp(
	bool        onec,
	char const *opts,
	char const *args,
	int        *params
) {
	bool         found = false;
	size_t       i     = 0;
	size_t const n     = onec ? 1 : strlen(args);

	for(; isspace(opts[i]); ++i)
		;
	while(isgraph(opts[i])) {
		if('-' == opts[i]) {
			i += onec;

			size_t const x = i++;
			for(; isalnum(opts[i]) || ('-' == opts[i]); ++i)
				;
			if(!found) {
				found = ((i - x) == n) && (0 == memcmp(opts + x, args, n));
			}
			for(; isspace(opts[i]); ++i)
				;
		}

		if(',' != opts[i]) break;

		for(++i; isspace(opts[i]); ++i)
			;
	}

	for(; isgraph(opts[i]); ++*params) {
		for(++i; isgraph(opts[i]); ++i)
			;
		for(; isspace(opts[i]); ++i)
			;
	}

	return found;
}

static int
optget__match(
	size_t         optc,
	OPTGET         optv[optc],
	char   const **argp,
	char   const  *args,
	int            argn,
	int           *params
) {
	int  r    = -1;
	bool onec = (argp && *argp);

	*params = 0;

	if(onec) {
		args = *argp;

	} else if(argp) {
		int const c = *(args + 1);
		if((onec = isalnum(c))) {
			*argp = ++args;
		}
	}

	for(size_t i = 0; optc > i; ++i) {
		OPTGET *const optp = &optv[i];
		if(optp->x > 0) {
			if(!optp->s) break;

			if(optget__optcmp(onec, optp->s, args, params)) {
				if(!*params || (argn >= *params)) {
					r = optp->x;
				}

				if(onec) {
					++*argp;
					if(!**argp) {
						*argp = NULL;
					}
				}

				break;
			}

			*params = 0;
		}
	}

	return r;
}

static void
optuse__optputs(
	OPTGET     *optp,
	char const *prog,
	size_t      stop,
	FILE       *outf
) {
	size_t n = 0;

	if(optp->s) {
		if(optp->x != 0) {
			fputs("  ", outf);
			n += 2;
		}

		n += fprintf(outf, optp->s, prog);
	}

	if(optp->t) {
		if(!optp->s) {
			stop += 2;
		}

		for(stop += 2; stop > n; --stop) {
			fputc(' ', outf);
		}

		fputs("  ", outf);
		fputs(optp->t, outf);
	}

	fputc('\n', outf);
}

static char const *
optuse__progname(
	char const *prog
) {
	size_t m = strlen(prog);
	size_t n = m;
	for(; n > 0; n--) {
		int c = prog[n-1];
		if(c == '/') break;
#ifdef _WIN32
		if(c == '\\') break;
		if(c == ':') break;
#endif
		if(c == '.') m = n - 1;
	}
	size_t z = m - n;
	char *name = malloc(z+1);
	if(name) {
		memcpy(name, prog+n, z);
		name[z] = '\0';
	}
	return name;
}

//------------------------------------------------------------------------------

int
optget(
	size_t         optc,
	OPTGET         optv[optc],
	char   const **argp,
	char   const  *args,
	int            argn,
	int           *params
) {
	int r = -1;

	if((optc > 0) && optv && args && (argn >= 0) && params) {
		r = 0;

		if(('-' == *args) && *(args + 1)) {
			if(*(args + 2) || ('-' != *(args + 1))) {
				r = optget__match(optc, optv, argp, args, argn, params);
			}
		}
	}

	return r;
}

void
optuse(
	size_t      optc,
	OPTGET      optv[optc],
	char const *prog,
	FILE       *outf
) {
	if((optc > 0) && optv && outf) {
		prog = optuse__progname(prog);

		size_t stop = 0;
		for(size_t i = 0; optc > i; ++i) {
			if(optv[i].x) {
				char const *const s = optv[i].s;
				size_t      const n = s ? strlen(s) : 0;
				if(n > stop) {
					stop = n;
				}
			}
		}

		for(size_t i = 0; optc > i; ++i) {
			optuse__optputs(&optv[i], prog, stop, outf);
		}

		free((void *)prog);
	}

	return;
}

//------------------------------------------------------------------------------

#endif//def HOL_OPTGET_H__IMPLEMENTATION
