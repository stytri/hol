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
outstr(
	char const *cs
) {
	return fputs(cs, out);
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
		{  0, "usage: %s [OPTION]... SELECTION...",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "output to FILE" },
		{  3, "-f, --file",              "read selections from SELECTION files" },
		{ 10, "-s, --spaced",            "space between selections" },
		{ 11, "-c, --column NUM",        "set NUMber of columns" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	out = stdout;

	bool spaced            = false;
	int  columns           = 0;
	bool load_from_files   = false;
	bool ignore_interrupts = false;

	size_t argz = 0;
	char **arga = NULL;
	int    argi = 1;
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
				load_from_files = true;
				break;
			case 10:
				spaced = true;
				break;
			case 11:
				columns = (int)streval(argv[argi], NULL, 0);
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

	if(load_from_files) {
		arga = malloc(1 * sizeof(char *));
		if(!arga) {
			perror();
			fail();
		}
		for(; argi < argc; argi++) {
			char *s = loadfile(argv[argi], NULL, NULL);
			if(!s) {
				perror(argv[argi]);
				fail();
			}
			for(;;) {
				while(*s == '\n') s++;
				if(!*s) break;
				if((argz & (argz - 1)) == 0) {
					size_t new_z = argz + argz + !argz;
					char **new_a = realloc(arga, (new_z + 1) * sizeof(char *));
					if(!new_a) {
						perror();
						fail();
					}
					arga = new_a;
				}
				for(arga[argz++] = s; *s; s++) {
					if(*s == '\n') {
						*s++ = '\0';
						break;
					}
				}
			}
			arga[argz] = NULL;
		}
	} else {
		arga = argv + argi;
		argz = argc - argi;
	}
	if(argz < 1) {
		errorf("need selectable arguments");
		fail();
	}
	if(columns < 0) columns = argz;

	if(ignore_interrupts) {
		signal(SIGINT, SIG_IGN);
	} else {
		signal(SIGINT, signal_handler);
	}
#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	int c = 0;
	for(size_t i; !gSignal;) {
		if(scanf("%zi", &i) != 1) {
			if(ferror(stdin)) {
				perror();
				fail();
			}
			break;
		}
		i %= argz;
		if(c++ > 0) {
			if(spaced) {
				outchar(' ');
			}
		}
		outstr(arga[i]);
		if(c == columns) {
			outchar('\n');
			c = 0;
		}
	}
	if(c > 0) outchar('\n');

	return 0;
}
