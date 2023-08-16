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
#include "wmem.h"
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

static int
wputs(
	wchar_t const *ws
) {
	if(fputws(ws, out) != WEOF) {
		return fputwc(L'\n', out);
	}
	return WEOF;
}

//------------------------------------------------------------------------------

static wchar_t *
wstrdup(
	char const *cs
) {
	size_t   len = strlen(cs);
	wchar_t *ws  = wmalloc(len+1);
	if(!ws) {
		perror();
		fail();
	}
	len = mbstowcs(ws, cs, len);
	ws[len] = L'\0';
	return ws;
}

static wchar_t *
wcsappend(
	wchar_t       *s,
	size_t         n,
	wchar_t const *cs,
	size_t         m,
	size_t        *z
) {
	if((SIZE_MAX - n) < m) {
		errno = ENOMEM;
		perror();
		fail();
	}
	size_t e = n + m;
	if((*z - n) < m) {
		s = wrealloc(s, e + 1);
		if(!s) {
			perror();
			fail();
		}
		*z = e;
	}
	wcsncpy(s + n, cs, m);
	s[e] = L'\0';
	return s;
}

//------------------------------------------------------------------------------

struct list {
	wchar_t const *data;
	struct list   *next;
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
	p->data = wstrdup(cs);
	p->next = *list;
	*list   = p;
}

struct list *exclusions = NULL;

static inline void
add_exclusion(
	char const *cs
) {
	add_list(&exclusions, cs);
}

struct list *directory_commands = NULL;

static inline void
add_directory_command(
	char const *cs
) {
	add_list(&directory_commands, cs);
}

struct list *file_commands = NULL;

static inline void
add_file_command(
	char const *cs
) {
	add_list(&file_commands, cs);
}

//------------------------------------------------------------------------------

enum {
	HIDDEN   = 1 << 0,
	SYSTEM   = 1 << 1,
	ALTPATH  = 1 << 3,
	NOINVAL  = 1 << 4,
	QUOTED   = 1 << 5,
	ECHO     = 1 << 6,
	DEBUG    = 1 << 15
};

static int
recursive_execute__command(
	wchar_t       *cwd,
	int            cwdlen,
	int            cwdrel,
	wchar_t const *cs,
	int            flags
) {
	static size_t   cmdsiz = 0;
	static wchar_t *cmd    = NULL;
	if(cs) {
		for(size_t n = 0; *cs; ) {
			size_t m = wcscspn(cs, L"%");
			if(m > 0) {
				cmd = wcsappend(cmd, n, cs, m, &cmdsiz);
				n  += m;
				cs += m;
			}
			if(!*cs++) break;
			switch(*cs) {
			default:
				cmd = wcsappend(cmd, n, cs, 1, &cmdsiz);
				n  += 1;
				cs++;
				break;
			case L'R':
				if(flags & QUOTED) {
					cmd = wcsappend(cmd, n, L"\"", 1, &cmdsiz);
					n  += 1;
				}
				cmd = wcsappend(cmd, n, cwd + cwdrel, cwdlen - cwdrel, &cmdsiz);
				n  += cwdlen - cwdrel;
				if(flags & QUOTED) {
					cmd = wcsappend(cmd, n, L"\"", 1, &cmdsiz);
					n  += 1;
				}
				cs++;
				break;
			case L'P':
				if(flags & QUOTED) {
					cmd = wcsappend(cmd, n, L"\"", 1, &cmdsiz);
					n  += 1;
				}
				cmd = wcsappend(cmd, n, cwd, cwdlen, &cmdsiz);
				n  += cwdlen;
				if(flags & QUOTED) {
					cmd = wcsappend(cmd, n, L"\"", 1, &cmdsiz);
					n  += 1;
				}
				cs++;
				break;
			case L'Q':
				cmd = wcsappend(cmd, n, L"\"", 1, &cmdsiz);
				n  += 1;
				cs++;
				break;
			case L'\0':
				cmd = wcsappend(cmd, n, L"%", 1, &cmdsiz);
				n  += 1;
				break;
			}
		}
		if(flags & (ECHO | DEBUG)) {
			wputs(cmd);
		}
		if(!(flags & DEBUG)) {
			return _wsystem(cmd);
		}
	}
	return 0;
}

static bool
allowed(
	int            flags,
	unsigned       attrib,
	wchar_t const *name
) {
	if(attrib & _A_HIDDEN) {
		if(!(flags & _A_HIDDEN)) {
			return false;
		}
	}
	if(attrib & _A_SYSTEM) {
		if(!(flags & SYSTEM)) {
			return false;
		}
	}
	if(wcscmp(name, L".") && wcscmp(name, L"..")) {
		for(struct list *p = exclusions; p; p = p->next) {
			if(wfnmatch(name, p->data)) {
				return false;
			}
		}
		return true;
	}
	return false;
}

static void
recursive_execute__actual(
	wchar_t **cwd,
	int      *cwdsiz,
	int       cwdlen,
	int       cwdrel,
	int       levels,
	int       flags
) {
	if(levels != 0) {
		if((*cwdsiz - cwdlen) < (FILENAME_MAX+1)) {
			if((INT_MAX - *cwdsiz) < (FILENAME_MAX+1)) {
				errno = ERANGE;
				perror();
				fail();
			}
			int   z = *cwdsiz + (FILENAME_MAX+1);
			void *p = wrealloc(*cwd, z + 1);
			if(!p) {
				perror();
				fail();
			}
			*cwdsiz = z;
			*cwd = p;
		}

		int cwdrev = cwdlen;
		if((cwdlen > 0)
			&& ((*cwd)[cwdlen - 1] == L'/')
		) {
			flags |= ALTPATH;
		} else if((cwdlen > 0)
			&& ((*cwd)[cwdlen - 1] != L'\\')
			&& ((*cwd)[cwdlen - 1] != L':')
			&& ((*cwd)[cwdlen - 1] != L'/')
		) {
			(*cwd)[cwdlen++] = (flags & ALTPATH) ? L'/' : L'\\';
		}

		int cwdrem = *cwdsiz - cwdlen;
		int cwdoff = cwdlen;
		if(!cwdrel) {
			cwdrel = cwdlen;
		}
		(*cwd)[cwdlen++] = L'*';
		(*cwd)[cwdlen]   = L'\0';

		struct _wfinddata_t d;
		intptr_t            h = _wfindfirst(*cwd, &d);
		if(h != -1) {
			while(!gSignal) {
				if(allowed(flags, d.attrib, d.name)) {
					int n = wcslen(d.name);
					if(n > cwdrem) {
						errno = ERANGE;
						perror();
						fail();
					}
					wcscpy(&(*cwd)[cwdoff], d.name);
					if(d.attrib & _A_SUBDIR) {
						for(struct list *p = directory_commands; p; p = p->next) {
							recursive_execute__command(*cwd, cwdoff + n, cwdrel, p->data, flags);
						}
						recursive_execute__actual(cwd, cwdsiz, cwdoff + n, cwdrel, levels - 1, flags);
					} else {
						for(struct list *p = file_commands; p; p = p->next) {
							recursive_execute__command(*cwd, cwdoff + n, cwdrel, p->data, flags);
						}
					}
				}
			next:
				if(_wfindnext(h, &d) != 0) {
					if(errno && (errno != ENOENT)) {
						if(!((flags & NOINVAL) && (errno = EINVAL))) {
							perror();
							fail();
						}
						goto next;
					}
					break;
				}
			}
			_findclose(h);
		} else {
			if(errno && (errno != ENOENT)) {
				if(!((flags & NOINVAL) && (errno = EINVAL))) {
					perror();
					fail();
				}
			}
		}

		(*cwd)[cwdrev] = (*cwd)[cwdoff] = L'\0';
	}
	return;
}

static void
recursive_execute(
	int         levels,
	int         flags,
	char const *wd
) {
	wchar_t *cwd = NULL;
	if(wd) {
		cwd = wstrdup(wd);
	} else  {
		cwd = _wgetcwd(NULL, FILENAME_MAX);
	}
	if(!cwd) {
		perror();
		fail();
	}
	int cwdlen = wcslen(cwd);
	int cwdsiz = cwdlen;
	recursive_execute__actual(&cwd, &cwdsiz, cwdlen, 0, levels, flags);
	wfree(cwd);
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
		"-D",
		"-d", "echo DIR %P %R",
		"-f", "echo FILE %P %R",
		"-f", "echo %R %P",
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
		{  0, "usage: %s [OPTION]... [DIRECTORY]...",   NULL },
		{  0, "options:",                  NULL },
		{  1, "-h, --help",                "display help" },
		{  2, "-o, --output FILE",         "output to FILE" },
		{ 10, "-H, --hidden",              "allow hidden files" },
		{ 11, "-S, --system",              "allow system files" },
		{ 13, "-a, --alternate-separator", "paths separated with '/'" },
		{ 15, "-q, --quoted",              "enclose paths in quotes" },
		{ 16, "-e, --echo",                "echo commands" },
		{ 20, "-l, --levels COUNT",        "set level limit to COUNT" },
		{ 21, "-1",                        "set level limit to 1" },
		{ 22, "-2",                        "set level limit to 2" },
		{ 23, "-3",                        "set level limit to 3" },
		{ 30, "-x, --exclude PATTERN",     "exclude files matching PATTERN" },
		{ 31, "-f, --file CMD",            "execute CMD on files" },
		{ 32, "-d, --directory CMD",       "execute CMD on directories" },
		{ 14, "-E, --enable-invalid",      "enable invalid path errors" },
		{ 90, "-D, --debug",               "enable debug mode (no execution)" },
		{ 99, "-I, --ignore-interrupts",   "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	out = stdout;

	int  flags             = NOINVAL;
	int  levels            = -1;
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
				out        = out_ispipe ? popen(out_name, "b") : fopen(out_name, "w");
				if(!out) {
					perror(out_name);
					fail();
				}
				break;
			case 10:
				flags |= HIDDEN;
				break;
			case 11:
				flags |= SYSTEM;
				break;
			case 13:
				flags |= ALTPATH;
				break;
			case 14:
				flags &= ~NOINVAL;
				break;
			case 15:
				flags |= QUOTED;
				break;
			case 16:
				flags |= ECHO;
				break;
			case 20:
				levels = streval(argv[argi], NULL, 0);
				break;
			case 21:
				levels = 1;
				break;
			case 22:
				levels = 2;
				break;
			case 23:
				levels = 3;
				break;
			case 90:
				flags |= DEBUG;
				break;
			case 30:
				add_exclusion(argv[argi]);
				break;
			case 31:
				add_file_command(argv[argi]);
				break;
			case 32:
				add_directory_command(argv[argi]);
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
	if(ignore_interrupts) {
		signal(SIGINT, SIG_IGN);
	} else {
		signal(SIGINT, signal_handler);
	}
#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	if(argi < argc) {
		do {
			recursive_execute(levels, flags, argv[argi++]);
		} while(argi < argc)
			;
	} else {
		recursive_execute(levels, flags, NULL);
	}

	return 0;
}
