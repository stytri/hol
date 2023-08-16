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

enum {
	HIDDEN   = 1 << 0,
	SYSTEM   = 1 << 1,
	RELATIVE = 1 << 2,
	ALTPATH  = 1 << 3,
	NOINVAL  = 1 << 4,
};

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
	return wcscmp(name, L".") && wcscmp(name, L"..");
}

static void
list_sub_directories__actual(
	wchar_t **cwd,
	int      *cwdsiz,
	int       cwdlen,
	int       cwdrel,
	int       levels,
	int       flags
) {
	size_t nsub = 0;
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
				if(d.attrib & _A_SUBDIR) {
					if(allowed(flags, d.attrib, d.name)) {
						int n = wcslen(d.name);
						if(n > cwdrem) {
							errno = ERANGE;
							perror();
							fail();
						}
						wcscpy(&(*cwd)[cwdoff], d.name);
						list_sub_directories__actual(cwd, cwdsiz, cwdoff + n, cwdrel, levels - 1, flags);
						nsub++;
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
	if(nsub == 0) {
		if(flags & RELATIVE) {
			wputs(&(*cwd)[cwdrel]);
		} else {
			wputs(*cwd);
		}
	}
	return;
}

static void
list_sub_directories(
	int         levels,
	int         flags,
	char const *wd
) {
	wchar_t *cwd = NULL;
	size_t   len = wd ? strlen(wd) : 0;
	if(len > INT_MAX) {
		errno = ERANGE;
	} else if(wd) {
		cwd = wmalloc(len+1);
		if(cwd) {
			len = mbstowcs(cwd, wd, len);
		}
	} else  {
		cwd = _wgetcwd(NULL, FILENAME_MAX);
		len = wcslen(cwd);
	}
	if(!cwd) {
		perror();
		fail();
	}

	if(len > INT_MAX) {
		errno = ERANGE;
		wfree(cwd);
		cwd = NULL;
	}
	int cwdlen = (int)len;
	int cwdsiz = cwdlen;
	list_sub_directories__actual(&cwd, &cwdsiz, cwdlen, 0, levels, flags);
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
		{ 10, "-H, --hidden",              "show hidden files" },
		{ 11, "-S, --system",              "show system files" },
		{ 12, "-r, --relative",            "show relative paths" },
		{ 13, "-a, --alternate-separator", "show paths separated with '/'" },
		{ 20, "-l, --levels COUNT",        "set level limit to COUNT" },
		{ 21, "-1",                        "set level limit to 1" },
		{ 22, "-2",                        "set level limit to 2" },
		{ 23, "-3",                        "set level limit to 3" },
		{ 14, "-E, --enable-invalid",      "enable invalid path errors" },
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
			case 12:
				flags |= RELATIVE;
				break;
			case 13:
				flags |= ALTPATH;
				break;
			case 14:
				flags &= ~NOINVAL;
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
			list_sub_directories(levels, flags, argv[argi++]);
		} while(argi < argc)
			;
	} else {
		list_sub_directories(levels, flags, NULL);
	}

	return 0;
}
