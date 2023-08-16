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
	return buf;
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

static uint8_t
strto_byte(
	char const *cs,
	char      **endp
) {
	uint8_t b = 0;
	for(; *cs && isspace(*cs); cs++);
	if(isxdigit(*cs)) {
		b = toxdigit(*cs);
		cs++;
		if(isxdigit(*cs)) {
			b = (b << 4) | toxdigit(*cs);
			cs++;
		}
	}
	*endp = (char *)cs;
	return b;
}

static char *
byte_tostr(
	uint8_t b,
	char   *s
) {
	static char const hex[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	*s++ = hex[(b >> 4) & 0xFu];
	*s++ = hex[ b       & 0xFu];
	return s;
}

static size_t
countNbit(
	uint64_t u,
	int      b
) {
	size_t n = 0;
	for(uint64_t const m = ~(~UINT64_C(0) << b); (u & m) != 0; u >>= b) {
		n++;
	}
	return n;
}

//------------------------------------------------------------------------------

static uint64_t
strto_vlqbe(
	char const *cs,
	char      **endp
) {
	size_t   n = countNbit(UINT64_MAX, 7), j = 0;
	uint64_t u = 0, v = 0;
	uint8_t  b;
	do {
		if(j == n) {
			errno = EILSEQ;
			u = UINT64_MAX;
			break;
		}
		b  = strto_byte(cs, endp);
		cs = *endp;
		v  = b & 0xFFu;
		u  = (u << 7) | (v & 0x7Fu);
		j++;
	} while(*cs && (v & 0x80u))
		;
	if(v & 0x80) {
		errno = EILSEQ;
		u = UINT64_MAX;
	}
	*endp = (char *)cs;
	return u;
}

static uint64_t
strto_vlqle(
	char const *cs,
	char      **endp
) {
	size_t   n = countNbit(UINT64_MAX, 7), j = 0;
	uint64_t u = 0, v = 0;
	uint8_t  b;
	do {
		if(j == n) {
			errno = EILSEQ;
			u = UINT64_MAX;
			break;
		}
		b  = strto_byte(cs, endp);
		cs = *endp;
		v  = b & 0xFFu;
		u |= (v & 0x7Fu) << (7 * j);
		j++;
	} while(*cs && (v & 0x80u))
		;
	if(v & 0x80) {
		errno = EILSEQ;
		u = UINT64_MAX;
	}
	*endp = (char *)cs;
	return u;
}

static char *
vlqbe_tostr(
	uint64_t u,
	char    *s
) {
	uint64_t v;
	uint8_t  b;
	size_t   j = countNbit(u, 7);
	while(j-- > 1) {
		v = (u >> (7 * j)) & 0x7Fu;
		b = 0x80u | v;
		s = byte_tostr(b, s);
		*s++ = ' ';
	}
	b = u & 0x7Fu;
	s = byte_tostr(b, s);
	return s;
}

static char *
vlqle_tostr(
	uint64_t u,
	char    *s
) {
	uint64_t v;
	uint8_t  b;
	size_t   j = countNbit(u, 7);
	while(j-- > 1) {
		v = u & 0x7Fu;
		b = 0x80u | v;
		s = byte_tostr(b, s);
		*s++ = ' ';
		u >>= 7;
	}
	b = u & 0x7Fu;
	s = byte_tostr(b, s);
	return s;
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
		{  0, "usage: %s [OPTION]... [FILE]...",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "output to FILE" },
		{ 10, "-e, --encode",            "encode values" },
		{ 11, "-l, --little-endian",     "little-endian format" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	bool       encode                            = false;
	uint64_t (*strto_vlq)(char const *, char **) = strto_vlqbe;
	char    *(*vlq_tostr)(uint64_t, char *)      = vlqbe_tostr;
	bool       ignore_interrupts                 = false;

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
				encode = true;
				break;
			case 11:
				strto_vlq = strto_vlqle;
				vlq_tostr = vlqle_tostr;
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
		bool interactive = isatty(fileno(out)) && isatty(fileno(in));
		for(char const *cs; !gSignal; ) {
			if(interactive) outchar('>');
			cs = inln();
			if(!cs) {
				perror();
				fail();
			}
			for(; !gSignal && *cs; cs++) {
				if(!isspace(*cs)) {
					if(*cs == '.') {
					again:
						cs++;
						switch(toupper(*cs)) {
						case 'D':
							encode = false;
							goto again;
						case 'E':
							encode = true;
							goto again;
						case 'B':
							strto_vlq = strto_vlqbe;
							vlq_tostr = vlqbe_tostr;
							goto again;
						case 'L':
							strto_vlq = strto_vlqle;
							vlq_tostr = vlqle_tostr;
							goto again;
						case 'Q':
							goto quit;
						}
					}
					break;
				}
			}
			if(gSignal) break;
			if(encode) {
				uint64_t u = streval(cs, NULL, 0);
				char     s[BITS(u)];
				*vlq_tostr(u, s) = '\0';
				if(!gSignal) {
					if(interactive) outchar(' ');
					print("%s\n", s);
				}
			} else {
				errno = 0;
				char    *s = NULL;
				uint64_t u = strto_vlq(cs, &s);
				if(!gSignal) {
					if(errno) {
						perror();
					} else {
						if(interactive) outchar(' ');
						print("%"PRIu64"\n", u);
					}
				}
			}
		}
	} while(!gSignal && (argi < argc))
		;
quit:
	return 0;
}
