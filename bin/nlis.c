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
#include <inttypes.h>
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

static inline size_t
ingest(
	uint64_t *w,
	size_t    n
) {
	return fread(w, sizeof(*w), n, in);
}

//------------------------------------------------------------------------------

static FILE       *out;
static char const *out_name   = "";
static bool        out_ispipe = false;

static inline size_t
egest(
	uint64_t *w,
	size_t    n
) {
	return fwrite(w, sizeof(*w), n, out);
}

//------------------------------------------------------------------------------

#define NKEYS  (4)
#define NPERMS (4*3*2)

static int
permute(
	uint64_t permute_high,
	uint64_t permute_low,
	size_t   index[NPERMS]
) {
	permute_low   ^= (permute_high << 16);
	permute_high >>= 48;
	uugenperm(permute_high, permute_low, NPERMS, index);
	return 0;
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
		{  0, "usage: %s [OPTION]... [ROUNDS:][SALT/]KEY[-PURGE] [FILE]", NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-o, --output FILE",       "output to FILE" },
		{  3, "-b, --buffer SIZE",       "set buffer SIZE" },
		{ 10, "-u, --undo",              "undo" },
		{ 19, "-c, --counter COUNT",     "generate COUNT numbers" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
		{  0, "", NULL },
		{  0, "ROUNDS is an integer value greater than 1", NULL },
		{  0, "SALT   is a 64-bit integer value", NULL },
		{  0, "KEY    is up-to 4 . separated 64-bit integer values,", NULL },
		{  0, "       or text optionally prefixed with #", NULL },
		{  0, "PURGE  number of purge cycles", NULL },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	in  = stdin;
	out = stdout;

	bool   undo              = false;
	bool   ignore_interrupts = false;
	size_t bufsize           = BUFSIZE;
	size_t counter           = 0;

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
			case 10:
				undo = true;
				break;
			case 19:
				counter = streval(argv[argi], NULL, 0);
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
		exit();
	}

	char           *rsk    = argv[argi++];
	size_t    const buflen = bufsize / sizeof(uint64_t);
	uint64_t *const buffer = calloc(buflen, sizeof(uint64_t));
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

	int             rounds = 11;
	uint64_t        salt   = 0;
	size_t          size   = 0;
	uint64_t        key[NKEYS];
	size_t          perm[NPERMS][NKEYS];
	size_t          index[NPERMS];
	char     const *non_integer;
	uint64_t        k = strtoull(non_integer = rsk, &rsk, 0);
	if(*rsk == ':') {
		rounds = (k > INT_MAX) ? INT_MAX : (int)k;
		if(rounds < 1) rounds = 11;
		k      = strtoull(non_integer = rsk+1, &rsk, 0);
	}
	if(*rsk == '/') {
		salt = k;
		k    = strtoull(non_integer = rsk+1, &rsk, 0);
	}
	if(rsk == non_integer) {
		xrand_init(rsk, sizeof(key), key);
		for(size_t i = 0; i < NKEYS; i++) {
			key[i] ^= genperm16(key[i] ^ salt);
		}
	} else {
		for(size_t i = 0; i < NKEYS; i++) {
			key[i] = k = genperm16(k ^ salt) ^ k;
			if(*rsk == '.') {
				k = strtoull(rsk+1, &rsk, 0);
			}
		}
	}
	for(size_t i = 0; i < NPERMS; i++) {
		genperm(i, NKEYS, perm[i]);
	}

	uint64_t x = nlis64(salt, key[0], rounds), w = nlis64(salt, key[1], rounds);
	uint64_t u = rotr(w|1, msbit(w|1)-1);
	size_t   y = 0;
#	define KEY(...)  ( \
		(y == 0) ? permute(x, w, index) : 0, \
		k = key[perm[index[y / NKEYS]][y % NKEYS]], \
		y = (y + 1) % (NPERMS * NKEYS), \
		k \
	)
	if(*rsk == '-') {
		for(size_t i = streval(rsk + 1, NULL, 0); i-- > 0; ) {
			x ^= KEY();
			w += u;
		}
	}
	uint64_t k0 = KEY();
	uint64_t k1 = KEY();
	uint64_t c = 0, e = 0;
	size_t   n = 0;
#	define EGEST(...)  do { \
		k0 = KEY(); \
		k1 = KEY(); \
		e = nlis64((__VA_ARGS__) + (w += u), k0, rounds); \
		buffer[n++] = nlis64(e ^ x, k1, rounds); \
		if(n == buflen) { \
			egest(buffer, n); \
			size += bufsize; \
			n = 0; \
		} \
		x += e; \
	} while(0)
#	define INGEST(...)  ( \
		k0 = KEY(), \
		k1 = KEY(), \
		e = ulis64(buffer[i], k1, rounds) ^ x, \
		ulis64(e, k0, rounds) - (w += u) \
	)
	if(counter > 0) {
		for(size_t i = 0; !gSignal && (i < counter); i++) {
			EGEST(i);
		}
		if(!gSignal && (n > 0)) {
			egest(buffer, n);
		}
	} else if(undo) {
		for(bool done = false;
			!gSignal && !done && ((n = ingest(buffer, buflen)) > 0);
		) {
			size_t m = 0;
			for(size_t i = 0; !gSignal && !done && (i < n); i++) {
				char  *t = (char *)&buffer[i];
				size_t z = 8;
				c = INGEST(buffer[i]);
				for(char *s = &t[8]; s > t; c >>= 8) {
					int ch = c & 0xFF;
					*--s = ch;
				}
				x += e;
				m += z;
			}
			fwrite(buffer, 1, m, out);
		}
	} else {
		size_t i = 0;
		for(int ch; !gSignal && ((ch = getchar()) != EOF); ) {
			c = (c << 8) | (ch & 0xFFu);
			if(i++ == 7) {
				EGEST(c);
				c = 0;
				i = 0;
			}
		}
		if(!gSignal && (i > 0)) {
			EGEST(c << ((8 - i) * 8));
		}
		size_t const SIZE = size + (bufsize - (size % bufsize));
		if(!gSignal && (size < SIZE)) {
			for(uint64_t j = 1; !gSignal && (size < SIZE); j++) {
				EGEST(j);
			}
		}
		if(!gSignal && (n > 0)) {
			egest(buffer, n);
		}
	}
#undef INGEST
#undef EGEST
#undef KEY
	return 0;
}
