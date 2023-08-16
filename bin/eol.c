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

static inline int
inchar(
	void
) {
	return fgetc(in);
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

//------------------------------------------------------------------------------

#define	CR			0
#define	LF			1
#define	CRLF		2
#define	LFCR		3

static int
geteol(
	char *arg
) {
	if(streq(arg, "crlf") || streq(arg, "CRLF")) {
		return CRLF;
	}
	if(streq(arg, "lfcr") || streq(arg, "LFCR")) {
		return LFCR;
	}
	if(streq(arg, "cr") || streq(arg, "CR")) {
		return CR;
	}
	if(streq(arg, "lf") || streq(arg, "LF")) {
		return LF;
	}

	errorf("unknown option %s\n", arg);
	fail();
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
		{  0, "usage: %s [OPTION]... CR|LF|CRLF|LFCR [FILE]...",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "output to FILE" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	bool ignore_interrupts = false;

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

	int const eol = geteol(argv[argi++]);

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
		bool was_eol = false;
		for(int c1, c2; !gSignal && ((c1 = inchar()) != EOF); ) {
			was_eol = (c1 == '\n') || (c1 == '\r');
			if(was_eol) {
				int const cc = ((c1 == '\n') ? '\r' : '\n');
				do {
					switch(eol) {
					case LFCR: outchar('\n'); // fallthrough
					case   CR: outchar('\r'); break;
					case CRLF: outchar('\r'); // fallthrough
					case   LF: outchar('\n'); break;
					}
					c2 = inchar();
				} while(c2 == c1)
					;
				if((c2 != cc) && (c2 != EOF)) {
					outchar(c2);
				}
			} else {
				outchar(c1);
			}
		}
		if(!was_eol) {
			switch(eol) {
			case LFCR: outchar('\n'); // fallthrough
			case   CR: outchar('\r'); break;
			case CRLF: outchar('\r'); // fallthrough
			case   LF: outchar('\n'); break;
			}
		}
	} while(!gSignal && (argi < argc))
		;
	return 0;
}
