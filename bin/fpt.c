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

static uint64_t
getfilesize(
	char const *file
) {
	FILE *f = fopen(file, "rb");
	if(f) {
		if(fseek(f, 0, SEEK_END) == 0) {
			long l = ftell(f);
			if(l > 0) {
				fclose(f);
				return (uint64_t)l;
			}
		}
		fclose(f);
	}
	return 0;
}


//------------------------------------------------------------------------------

static FILE       *in;
static char const *in_name   = "";
static bool        in_ispipe = false;

static inline size_t
ingest(
	void   *b,
	size_t  n
) {
	return fread(b, 1, n, in);
}

//------------------------------------------------------------------------------

static FILE       *out;
static char const *out_name   = "";
static bool        out_ispipe = false;

static inline size_t
egest(
	void const *b,
	size_t      n
) {
	return fwrite(b, 1, n, out);
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
		"-np", "justfile", "justfile.out",
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
		{  0, "usage: %s [OPTION]... [FILE]",    NULL },
		{  0, "options:",                  NULL },
		{  1, "-h, --help",                "display help" },
		{  2, "-o, --output FILE",         "output to FILE" },
		{  3, "-b, --buffer SIZE",         "set buffer SIZE" },
		{ 10, "-n, --nul-terminate",       "nul terminate" },
		{ 11, "-N, --undo-nul-terminate",  "undo nul termination" },
		{ 12, "-z, --size [SIZE|@FILE]",   "size with SIZE or FILE size" },
		{ 13, "-Z, --undo-size",           "undo sized" },
		{ 14, "-a, --align COUNT",         "align to COUNT byte boundary with zero" },
		{ 15, "-A, --random-align COUNT",  "align to COUNT byte boundary with random values" },
		{ 16, "-p, --prefix COUNT",        "prefix COUNT zero bytes" },
		{ 17, "-P, --random-prefix COUNT", "prefix COUNT random bytes" },
		{ 18, "-s, --suffix COUNT",        "suffix COUNT zero bytes" },
		{ 19, "-S, --random-suffix COUNT", "suffix COUNT random bytes" },
		{ 20, "-k, --skip COUNT",          "skip initial COUNT bytes" },
		{ 99, "-I, --ignore-interrupts",   "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	bool     undo              = false;
	bool     nul_terminated    = false;
	bool     sized             = false;
	bool     random_prefix     = false;
	bool     random_suffix     = false;
	bool     random_alignto    = false;
	bool     ignore_interrupts = false;
	size_t   bufsize           = BUFSIZE;
	uint64_t size              = 0;
	size_t   prefix            = 0;
	size_t   suffix            = 0;
	size_t   skip              = 0;
	size_t   alignto           = false;

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
				if(bufsize == 0) bufsize = BUFSIZE;
				break;
			case 10:
				undo = false;
				nul_terminated = true;
				break;
			case 11:
				undo = nul_terminated = true;
				break;
			case 12:
				if(*argv[argi] == '@') {
					if(in != stdin) {
						in_ispipe ? pclose(in) : fclose(in);
					}
					in_name   = argv[argi] + 1;
					in_ispipe = false;
					size      = getfilesize(in_name);
					in        = fopen(in_name, "rb");
					if(!in) {
						perror(in_name);
						fail();
					}
				} else {
					size = streval(argv[argi], NULL, 0);
				}
				undo = false;
				sized = true;
				break;
			case 13:
				undo = sized = true;
				break;
			case 14:
				random_alignto = false;
				alignto = streval(argv[argi], NULL, 0);
				if(alignto == 0) alignto = bufsize;
				break;
			case 15:
				random_alignto = true;
				alignto = streval(argv[argi], NULL, 0);
				if(alignto == 0) alignto = bufsize;
				break;
			case 16:
				random_prefix = false;
				prefix = streval(argv[argi], NULL, 0);
				if(prefix == 0) prefix = bufsize;
				break;
			case 17:
				random_prefix = true;
				prefix = streval(argv[argi], NULL, 0);
				if(prefix == 0) prefix = bufsize;
				break;
			case 18:
				random_suffix = false;
				suffix = streval(argv[argi], NULL, 0);
				if(suffix == 0) suffix = bufsize;
				break;
			case 19:
				random_suffix = true;
				suffix = streval(argv[argi], NULL, 0);
				if(suffix == 0) suffix = bufsize;
				break;
			case 20:
				skip = streval(argv[argi], NULL, 0);
				if(skip == 0) skip = bufsize;
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

	uint8_t *const buffer = malloc(bufsize);
	if(!buffer) {
		perror();
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

	bool   done = false;
	size_t z    = 0, m = 0, n = 0, u = 0;

	XRAND8   r;
	uint64_t s = xrandseed();
	xrand_seed(sizeof(s), &s, sizeof(r), &r);

	if(skip) {
		for(; !gSignal && (skip > 0); skip -= n) {
			m = (skip > bufsize) ? bufsize : skip;
			n = ingest(buffer, m);
			if(!n) {
				perror();
				fail();
			}
		}
	}

	if(prefix) {
		memset(buffer, 0, bufsize);
		for(u = prefix; !gSignal && (u > 0); u -= n) {
			if(random_prefix) {
				xfill8(&r, buffer, bufsize);
			}
			m = (u > bufsize) ? bufsize : u;
			n = egest(buffer, m);
			if(!n) {
				perror();
				fail();
			}
		}
	}

	if(sized) {
		if(undo) {
			if(ingest(&size, sizeof(size)) != sizeof(size)) {
				perror();
				fail();
			}
		} else {
			if(egest(&size, sizeof(size)) != sizeof(size)) {
				perror();
				fail();
			}
		}
	}

	while(!gSignal && !done && ((n = ingest(buffer, bufsize)) > 0)) {
		m = 0;
		if(sized) {
			u = size - z;
			if((done = (n >= u))) {
				m = u;
			} else {
				m = n;
			}
		} else if(nul_terminated) {
			for(; !gSignal && !done && (m < n); m += !done) {
				done = !buffer[m];
			}
		} else {
			m = n;
		}
		if(!gSignal) {
			n = egest(buffer, m);
			if(!n) {
				perror();
				fail();
			}
			z += n;
		}
	}
	while(!gSignal && done && (n > 0)) {
		n = ingest(buffer, bufsize);
	}
	if(ferror(stdin)) {
		perror();
		fail();
	}

	nul_terminated = !undo && nul_terminated;

	if(!gSignal && (nul_terminated || alignto)) {
		if(sized) {
			z += sizeof(size);
		}
		if(alignto > 1) {
			m   = (z + nul_terminated) % alignto;
			alignto = (m > 0) ? (alignto - m) : nul_terminated;
		} else {
			alignto |= nul_terminated;
		}
		memset(buffer, 0, bufsize);
		for(; !gSignal && (alignto > 0); alignto -= n) {
			if(random_alignto) {
				xfill8(&r, buffer, bufsize);
				if(nul_terminated) {
					nul_terminated = false;
					buffer[0] = 0;
				}
			}
			m = (alignto > bufsize) ? bufsize : alignto;
			n = egest(buffer, m);
			if(!n) {
				perror();
				fail();
			}
		}
	}

	if(suffix) {
		memset(buffer, 0, bufsize);
		for(u = suffix; !gSignal && (u > 0); u -= n) {
			if(random_suffix) {
				xfill8(&r, buffer, bufsize);
			}
			m = (u > bufsize) ? bufsize : u;
			n = egest(buffer, m);
			if(!n) {
				perror();
				fail();
			}
		}
	}

	return 0;
}
