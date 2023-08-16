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
scan(
	char const *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vfscanf(in, fmt, va);
	va_end(va);
	return r;
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

static inline int isccntrl(int c) { return !iscntrl(c); }

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
		{  0, "usage: %s [OPTION]... [BITS] [FILE]...", NULL },
		{  0, "options:",                    NULL },
		{  1, "-h, --help",                  "display help" },
		{  2, "-o, --output FILE",           "output to FILE" },
		{  3, "-b, --buffer SIZE",           "set buffer SIZE" },
		{  9, "-u, --unhex",                 "hex to binary" },
		{ 10, "-s, --spaced",                "space between numbers" },
		{ 11, "-p, --prefix TEXT",           "prefix numbers with TEXT" },
		{ 12, "-x, --suffix TEXT",           "suffix numbers with TEXT" },
		{ 13, "-c, --column NUM",            "set NUMber of columns" },
		{ 14, "-a, --array-format",          "format as array data" },
		{ 15, "-l, --line-format",           "format as lines" },
		{ 16, "-L, --alternate-line-format", "format as lines" },
		{ 17, "-t, --end-of-line CHAR",      "CHARacter terminates lines" },
		{ 18, "-v, --side-view",             "with side-view" },
		{ 19, "-V, --extended-side-view",    "with extended side-view" },
		{ 20, "-e, --enumerate WIDTH",       "enumerated output (WIDTH digits)" },
		{ 99, "-I, --ignore-interrupts",     "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	int    eol                 = '\n';
	bool   ashex               = true;
	bool   spaced              = false;
	char  *prefix              = "";
	char  *suffix              = "";
	bool   line_view           = false;
	bool   alternate_line_view = false;
	bool   side_view           = false;
	bool   extended_side_view  = false;
	bool   calculate_column    = false;
	int    enum_width          = 0;
	size_t column              = 0;
	size_t bufsize             = BUFSIZE;
	bool   ignore_interrupts   = false;

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
			case 3:
				bufsize = streval(argv[argi], NULL, 0);
				bufsize = (bufsize + 7 + !bufsize) & ~(size_t)7;
				break;
			case 9:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = false;
				ashex = false;
				break;
			case 10:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = false;
				spaced = true;
				ashex = true;
				break;
			case 11:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = false;
				prefix = argv[argi];
				ashex = true;
				break;
			case 12:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = false;
				suffix = argv[argi];
				ashex = true;
				break;
			case 13:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = false;
				calculate_column = false;
				column = streval(argv[argi], NULL, 0);
				ashex = true;
				break;
			case 14:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = false;
				calculate_column = true;
				spaced = true;
				prefix = "0x";
				suffix = ",";
				ashex = true;
				break;
			case 15:
				line_view = true;
				alternate_line_view = false;
				side_view = extended_side_view = false;
				calculate_column = false;
				column = 0;
				spaced = true;
				prefix = "";
				suffix = "";
				ashex = true;
				break;
			case 16:
				alternate_line_view = true;
				line_view = false;
				side_view = extended_side_view = false;
				calculate_column = false;
				column = 0;
				spaced = true;
				prefix = "";
				suffix = "";
				ashex = true;
				break;
			case 17:
				eol = argv[argi][0];
				if(argv[argi][1] != '\0') {
					if(eol == '\\') switch((eol = argv[argi][1])) {
					default :             break;
					case 'n': eol = '\n'; break;
					case 'r': eol = '\r'; break;
					case 's': eol = ' ' ; break;
					case 't': eol = '\t'; break;
					} else if(isdigit(eol)) {
						eol = (int)strtol(argv[argi], NULL, 0);
					}
				}
				ashex = true;
				break;
			case 18:
				line_view = alternate_line_view = false;
				side_view = true;
				extended_side_view = false;
				calculate_column = false;
				column = 16;
				spaced = true;
				prefix = "";
				suffix = "";
				ashex = true;
				break;
			case 19:
				line_view = alternate_line_view = false;
				side_view = extended_side_view = true;
				calculate_column = false;
				column = 16;
				spaced = true;
				prefix = "";
				suffix = "";
				ashex = true;
				break;
			case 20:
				enum_width = (int)streval(argv[argi], NULL, 0);
				ashex = true;
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

	size_t  const bits   = (line_view || alternate_line_view || side_view) ? (
		8
	):(
		((argi < argc) && str_is(argv[argi], isdigit)) ? (
			streval(argv[argi++], NULL, 0)
		):(
			64
		)
	);
	size_t  const bytes  = bits / 8;
	size_t  const COUNT  = bufsize / bytes;
	void   *const buffer = malloc(bufsize = COUNT * bytes);
	if(!buffer) {
		perror();
		fail();
	}
	char *format;
	{
		int n_digits   = bits / 4;
		int prefix_len = strlen(prefix);
		int suffix_len = strlen(suffix);
		format = malloc(prefix_len + suffix_len + 16);
		if(!format) {
			perror();
			fail();
		}
		char const *fmt;
		if(ashex) {
			switch(bits) {
			case  8: fmt = PRIX8 ; break;
			case 16: fmt = PRIX16; break;
			case 32: fmt = PRIX32; break;
			default: fmt = PRIX64; break;
			}
			sprintf(format, "%s%%%i.%i%s%s", prefix, n_digits, n_digits, fmt, suffix);
		} else {
			switch(bits) {
			case  8: fmt = SCNx8 ; break;
			case 16: fmt = SCNx16; break;
			case 32: fmt = SCNx32; break;
			default: fmt = SCNx64; break;
			}
			sprintf(format, " %s%%%i%s%s", prefix, n_digits, fmt, suffix);
		}
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
	size_t col = 0, count = 0;
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
		if(ashex) switch(bits) {
		case 8:
			if(calculate_column) column = 8;
			if(enum_width) print("%.*zu: ", enum_width, count);
			if(side_view) {
				int (*printable)(int) = extended_side_view ? isccntrl : isprint;
				for(uint8_t *b = buffer; !gSignal;) {
					size_t n = fread(b, sizeof(*b), COUNT, in);
					size_t i = 0;
					for(; !gSignal && (i < n);) {
						print(format, b[i++]);
						count++;
						if(++col == column) {
							outchar(' ');
							outchar('|');
							for(size_t j = i - col; j < i; j++) {
								if(printable(b[j])) outchar(b[j]);
								else                outchar('.');
							}
							outchar('|');
							outchar('\n');
							col = 0;
							if(enum_width) print("%.*zu: ", enum_width, count);
							continue;
						}
						outchar(' ');
					}
					if(n < COUNT) {
						if(col > 0) {
							for(size_t j = column - col; j-- > 0;) {
								outchar(' ');
								outchar(' ');
								outchar(' ');
							}
							outchar('|');
							for(size_t j = i - col; j < i; j++) {
								if(isprint(b[j])) outchar(b[j]);
								else              outchar('.');
							}
							for(size_t j = column - col; j-- > 0;) {
								outchar(' ');
							}
							outchar('|');
							outchar('\n');
							col = count = 0;
						}
						break;
					}
				}
			} else if(line_view) {
				for(uint8_t *b = buffer; !gSignal;) {
					size_t n = fread(b, sizeof(*b), COUNT, in);
					size_t i = 0;
					for(; !gSignal && (i < n);) {
						bool lf = b[i] == eol;
						print(format, b[i++]);
						count++;
						if(lf) {
							outchar('\n');
							col = 0;
							if(enum_width) print("%.*zu: ", enum_width, count);
						} else {
							outchar(' ');
							col++;
						}
					}
					if(n < COUNT) {
						if(col > 0) {
							outchar('\n');
							col = count = 0;
						}
						break;
					}
				}
			} else if(alternate_line_view) {
				for(uint8_t *b = buffer; !gSignal;) {
					size_t n = fread(b, sizeof(*b), COUNT, in);
					size_t i = 0;
					for(; !gSignal && (i < n);) {
						bool lf = b[i] == eol;
						if(lf) {
							if(count > 0) {
								outchar('\n');
								col = 0;
								if(enum_width) print("%.*zu: ", enum_width, count);
							}
						} else if(col > 0) {
							outchar(' ');
						}
						print(format, b[i++]);
						count++;
						col++;
					}
					if(n < COUNT) {
						if(col > 0) {
							outchar('\n');
							col = count = 0;
						}
						break;
					}
				}
			} else {
				for(uint8_t *b = buffer; !gSignal;) {
					size_t n = fread(b, sizeof(*b), COUNT, in);
					size_t i = 0;
					for(; !gSignal && (i < n);) {
						print(format, b[i++]);
						count++;
						if(++col == column) {
							outchar('\n');
							col = 0;
							if(enum_width) print("%.*zu: ", enum_width, count);
							continue;
						}
						if(spaced) outchar(' ');
					}
					if(n < COUNT) {
						if(col > 0) {
							if(enum_width) {
								outchar('\n');
								col = count = 0;
							}
						}
						break;
					}
				}
			}
			break;
		case 16:
			if(calculate_column) column = 8;
			if(enum_width) print("%.*zu: ", enum_width, count);
			for(uint16_t *b = buffer; !gSignal;) {
				size_t n = fread(b, sizeof(*b), COUNT, in);
				for(size_t i = 0; !gSignal && (i < n); i++) {
					print(format, b[i]);
					count++;
					if(++col == column) {
						outchar('\n');
						col = 0;
						if(enum_width) print("%.*zu: ", enum_width, count);
						continue;
					}
					if(spaced) outchar(' ');
				}
				if(n < COUNT) {
					if(enum_width) {
						outchar('\n');
						col = count = 0;
					}
					break;
				}
			}
			break;
		case 32:
			if(calculate_column) column = 4;
			if(enum_width) print("%.*zu: ", enum_width, count);
			for(uint32_t *b = buffer; !gSignal;) {
				size_t n = fread(b, sizeof(*b), COUNT, in);
				for(size_t i = 0; !gSignal && (i < n); i++) {
					print(format, b[i]);
					count++;
					if(++col == column) {
						outchar('\n');
						col = 0;
						if(enum_width) print("%.*zu: ", enum_width, count);
						continue;
					}
					if(spaced) outchar(' ');
				}
				if(n < COUNT) {
					if(enum_width) {
						outchar('\n');
						col = count = 0;
					}
					break;
				}
			}
			break;
		case 64:
			if(calculate_column) column = 4;
			if(enum_width) print("%.*zu: ", enum_width, count);
			for(uint64_t *b = buffer; !gSignal;) {
				size_t n = fread(b, sizeof(*b), COUNT, in);
				for(size_t i = 0; !gSignal && (i < n); i++) {
					print(format, b[i]);
					count++;
					if(++col == column) {
						outchar('\n');
						col = 0;
						if(enum_width) print("%.*zu: ", enum_width, count);
						continue;
					}
					if(spaced) outchar(' ');
				}
				if(n < COUNT) {
					if(enum_width) {
						outchar('\n');
						col = count = 0;
					}
					break;
				}
			}
			break;
		} else switch(bits) {
		case 8:
			for(uint8_t *b = buffer; !gSignal;) {
				size_t n = 0;
				for(; !gSignal && (n < COUNT); n++) {
					if(scan(format, &b[n]) != 1) break;
				}
				if((n > 0) && (fwrite(b, sizeof(*b), n, out) != n)) break;
				if(n < COUNT) break;
			}
			break;
		case 16:
			for(uint16_t *b = buffer; !gSignal;) {
				size_t n = 0;
				for(; !gSignal && (n < COUNT); n++) {
					if(scan(format, &b[n]) != 1) break;
				}
				if((n > 0) && (fwrite(b, sizeof(*b), n, out) != n)) break;
				if(n < COUNT) break;
			}
			break;
		case 32:
			for(uint32_t *b = buffer; !gSignal;) {
				size_t n = 0;
				for(; !gSignal && (n < COUNT); n++) {
					if(scan(format, b[n]) != 1) break;
				}
				if((n > 0) && (fwrite(b, sizeof(*b), n, out) != n)) break;
				if(n < COUNT) break;
			}
			break;
		case 64:
			for(uint64_t *b = buffer; !gSignal;) {
				size_t n = 0;
				for(; !gSignal && (n < COUNT); n++) {
					if(scan(format, &b[n]) != 1) break;
				}
				if((n > 0) && (fwrite(b, sizeof(*b), n, out) != n)) break;
				if(n < COUNT) break;
			}
		}
	} while(!gSignal && (argi < argc))
		;
	if(col > 0) {
		outchar('\n');
	}
	return 0;
}
