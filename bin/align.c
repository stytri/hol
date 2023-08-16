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

static char const *
inln(
	void
) {
	static char   static_buf[BUFSIZ];
	static char  *buf = static_buf;
	static size_t siz = BUFSIZ - 1;

	size_t n = 0;
	for(int c; !gSignal && ((c = inchar()) != EOF);) {
		if(n == siz) {
			size_t z = (2 * (siz + 1)) - 1;
			void  *p = (buf != static_buf) ? realloc(buf, siz + 1) : malloc(siz + 1);
			if(!p) {
				return NULL;
			}
			buf = p;
			siz = z;
		}
		buf[n++] = c;
		if(c == '\n') break;
	}
	buf[n] = '\0';
	return (n == 0) ? NULL : buf;
}

//------------------------------------------------------------------------------

static FILE       *out;
static char const *out_name   = "";
static bool        out_ispipe = false;

static inline int
outstr(
	char const *cs
) {
	return fputs(cs, out);
}

//------------------------------------------------------------------------------

struct list {
	char const  *data;
	struct list *next;
};

static void
add_list(
	struct list **list,
	char const   *cs
) {
	struct list *p = malloc(sizeof(*p));
	if(!p) {
		perror();
		fail();
	}
	for(struct list *l = *list; l; l = l->next) {
		list = &l->next;
	}
	p->data = cs;
	p->next = *list;
	*list   = p;
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
		{  0, "usage: %s [OPTION]... POSITION STRING [FILE]...",   NULL },
		{  0, "options:",                   NULL },
		{  1, "-h, --help",                "display help" },
		{  2, "-o, --output FILE",         "output to FILE" },
		{ 10, "-l, --last",                "align on the last STRING" },
		{ 11, "-i, --ignore-after STRING2","ignore any STRING after STRING2" },
		{ 99, "-I, --ignore-interrupts",   "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	bool         last              = false;
	struct list *ignore_after      = NULL;
	bool         ignore_interrupts = false;

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
				last = true;
				break;
			case 11:
				add_list(&ignore_after, argv[argi]);
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

	size_t      const pos = streval(argv[argi++], NULL, 0);
	char const *const str = argv[argi++];

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
		for(char const *cs;
			!gSignal && ((cs = inln()) != NULL);
		) {
			size_t len = strlen(cs);
			char  *s   = strstr(cs, str);
			if(s) {
				char const *ct = cs + len;
				for(struct list *l = ignore_after; l != NULL; l = l->next) {
					char const *cx = strstr(cs, l->data);
					if(cx != NULL) {
						if(cx < ct) {
							ct = cx;
						}
					}
				}
				if(last) {
					for(char *p;
						((p = strstr(s+1, str)) != NULL) && (p < ct);
						s = p
					);
				}
				if(s <= ct) {
					size_t off = s - cs;
					if(off < pos) {
						size_t pad = pos - off;
						s = malloc(len + pad + 1);
						if(!s) {
							perror();
							fail();
						}
						memcpy(s, cs, off);
						memset(s + off, ' ', pad);
						memcpy(s + off + pad, cs + off, (len - off) + 1);
						if(outstr(s) < 0) {
							free(s);
							perror();
							fail();
						}
						free(s);
						continue;
					}
				}
			}
			if(outstr(cs) < 0) {
				perror();
				fail();
			}
		}
	} while(!gSignal && (argi < argc))
		;
	return 0;
}
