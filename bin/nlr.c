/*
MIT License

Copyright (c) 2024 Tristan Styles

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

//
// Build with https://github.com/stytri/m
//
// ::compile
// :+  $CC $CFLAGS $XFLAGS $SMALL-BINARY
// :+      -o $+^ $"* $"!
//
// ::clean
// :+  $RM *.i *.s *.o *.exe
//
// ::-
// :+  $CC $CFLAGS
// :+      -Og -g -DDEBUG_$: -o $+: $"!
// :&  $DBG -tui --args $+: -o "tmp:" $"*
// :&  $RM $+:
//
// ::CFLAGS
// :+      -Wall -Wextra $WINFLAGS $INCLUDE
//
// ::XFLAGS
// :+      -DNDEBUG=1 -O3
//
// ::SMALL-BINARY
// :+      -fmerge-all-constants -ffunction-sections -fdata-sections
// :+      -fno-unwind-tables -fno-asynchronous-unwind-tables
// :+      -Wl,--gc-sections -s
//
// ::windir?WINFLAGS
// :+      -D__USE_MINGW_ANSI_STDIO=1
//
// ::INCLUDE!INCLUDE
// :+      -I ../../../inc
//

//------------------------------------------------------------------------------

#define BASE64__UTF8_MAP (1)
#include <hol/holibc.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

MWNLIS(512)

#ifndef ROUNDS
enum { ROUNDS = 47 };
#endif

static inline void memzero(void *p, size_t n) {
#ifdef _WIN32
	SecureZeroMemory(p, n);
#else
    for(volatile unsigned char *volatile v = p, *e = v + n; v < e; *v++ = 0)
		;
#endif
}

static inline uint64_t uint64frombytes(uint8_t const b[8]) {
	return ((uint64_t)b[0] <<  0)
	|      ((uint64_t)b[1] <<  8)
	|      ((uint64_t)b[2] << 16)
	|      ((uint64_t)b[3] << 24)
	|      ((uint64_t)b[4] << 32)
	|      ((uint64_t)b[5] << 40)
	|      ((uint64_t)b[6] << 48)
	|      ((uint64_t)b[7] << 56)
	;
}

static inline void uint64tobytes(uint64_t u, uint8_t b[8]) {
	b[0] = (uint8_t)(u >>  0);
	b[1] = (uint8_t)(u >>  8);
	b[2] = (uint8_t)(u >> 16);
	b[3] = (uint8_t)(u >> 24);
	b[4] = (uint8_t)(u >> 32);
	b[5] = (uint8_t)(u >> 40);
	b[6] = (uint8_t)(u >> 48);
	b[7] = (uint8_t)(u >> 56);
}

static inline uint512_t uint512frombytes(uint8_t const b[64]) {
	uint512_t u = {{
		uint64frombytes(&b[ 0]),
		uint64frombytes(&b[ 8]),
		uint64frombytes(&b[16]),
		uint64frombytes(&b[24]),
		uint64frombytes(&b[32]),
		uint64frombytes(&b[40]),
		uint64frombytes(&b[48]),
		uint64frombytes(&b[56]),
	}};
	return u;
}

static inline void uint512tobytes(uint512_t u, uint8_t b[64]) {
	uint64tobytes(u.u[0], &b[ 0]);
	uint64tobytes(u.u[1], &b[ 8]);
	uint64tobytes(u.u[2], &b[16]);
	uint64tobytes(u.u[3], &b[24]);
	uint64tobytes(u.u[4], &b[32]);
	uint64tobytes(u.u[5], &b[40]);
	uint64tobytes(u.u[6], &b[48]);
	uint64tobytes(u.u[7], &b[56]);
}

struct genrand {
	uint512_t k, c;
	size_t    i;
};
static uint64_t genrand(void *ctx) {
	struct genrand *g = ctx;
	uint512_t       r = nlis512(g->c, g->k, 3*ROUNDS);
	g->c              = inc512(g->c, 1);
	size_t          i = g->i % 8;
	uint64_t        x = r.u[i];
	g->i             += x;
	return x;
}

static bool encode(size_t z, uint8_t b[z], uint512_t k, XFILE *in, XFILE *out) {
	bool           ok = false;
	struct genrand g;
	ssize_t        m = getrandom(&g, sizeof(g), 0);
	if(m != sizeof(g)) {
		if(m < 0) m = 0;
		fallback_getrandom((uint8_t*)&g + m, sizeof(g) - m, 0);
	}
	uint8_t v[8];
	uint8_t w[64];
	z = ((z - 56) / 56) * 56;
	for(bool eof = false; !eof;) {
		size_t n = xfread(b, sizeof(*b), z, in);
		if(n < z) {
			if(xferror(in)) {
#ifdef EPIPE
				if(errno != EPIPE)
#endif
				{
					goto done;
				}
			}
			size_t m = 56 - (n % 56);
			fillrandom(genrand, &g, &b[n], m);
			m += n;
			for(b[n++] |= 0x80; n < m; b[n++] &= 0x7F)
				;
			eof = true;
		}
		for(size_t m = 0; m < n; m += 56) {
			uint64_t r = genrand(&g);
			uint64tobytes(r, &v[0]);
			size_t  h = 0, i = 0, j = m;
			uint8_t t = v[h++];
			w[i++] = t;
			do {
				size_t x = t % 8, y = 0;
				for(; y < x; y++) {
					w[i++] = b[j++];
				}
				t = v[h++];
				w[i++] = t;
				for(; y < 8; y++) {
					w[i++] = b[j++];
				}
			} while(i < 64)
				;
			uint512_t u = nlis512(uint512frombytes(w), k, ROUNDS);
			uint512tobytes(u, w);
			if(xfwrite(w, 1, 64, out) != 64) {
				goto done;
			}
		}
	}
	ok = true;
done:
	memzero(&k, sizeof(k));
	memzero(&w, sizeof(w));
	memzero(&v, sizeof(v));
	memzero(&g, sizeof(g));
	return ok;
}

static bool decode(size_t z, uint8_t b[z], uint512_t k, XFILE *in, XFILE *out) {
	bool    ok = false;
	uint8_t w[64];
	uint8_t v[56];
	uint8_t d[56];
	z = (z / 64) * 64;
	for(bool buffered = false, eof = false; !eof;) {
		size_t n = xfread(b, sizeof(*b), z, in);
		if(n % 64) {
			errno = EILSEQ;
			goto done;
		}
		if(n < z) {
			if(xferror(in)) {
#ifdef EPIPE
				if(errno != EPIPE)
#endif
				{
					goto done;
				}
			}
			eof = true;
		}
		for(size_t m = 0; m < n; m += 64) {
			uint512_t u = ulis512(uint512frombytes(&b[m]), k, ROUNDS);
			uint512tobytes(u, &w[0]);
			size_t  i = 0, j = 0;
			uint8_t t = w[j++];
			do {
				size_t x = t % 8, y = 0;
				for(; y < x; y++) {
					v[i++] = w[j++];
				}
				t = w[j++];
				for(; y < 8; y++) {
					v[i++] = w[j++];
				}
			} while(i < 56)
				;
			if(!buffered) {
				buffered = true;
			} else {
				if(xfwrite(d, 1, 56, out) != 56) {
					goto done;
				}
			}
			memcpy(d, &v[0], 56);
		}
		if(eof && (n > 0)) {
			for(n = 55; (n > 0) && !(d[n] & 0x80); n--)
				;
			if(!(d[n] & 0x80)) {
				errno = EILSEQ;
				goto done;
			}
			if(xfwrite(d, 1, n, out) != n) {
				goto done;
			}
		}
	}
	ok = true;
done:
	memzero(&k, sizeof(k));
	memzero(&w, sizeof(w));
	memzero(&v, sizeof(v));
	memzero(&d, sizeof(d));
	return ok;
}

static uint512_t initialize(uint512_t k, void const *key, size_t z) {
	if(z > sizeof(k)) {
		z = sizeof(k);
	}
	uint8_t b[sizeof(k)];
	memcpy(b, key, z);
	while(z < sizeof(k)) {
		b[z++] = 0;
	}
	k = nlis512(uint512frombytes(b), k, ROUNDS);
	memzero(b, sizeof(k));
	return k;
}

//------------------------------------------------------------------------------

#ifdef __MINGW64__
int _dowildcard = -1;
#endif

int
main(
	int    argc,
	char **argv
) {
	static struct optget options[] = {
		{  0, "usage: %s [OPTION]... [FILE]...", NULL },
		{  0, "options:",          NULL },
		{  1, "-h, --help",        "display help" },
		{  2, "-o, --output FILE", "output to FILE" },
		{  5, "-k, --key-file",    "read KEY from text file" },
		{  6, "-b, --binary-key",  "read KEY from binary file" },
		{  8, "-e, --encode KEY",  NULL },
		{  9, "-d, --decode KEY",  NULL },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	XFILE *in  = xfopen("stdin:");
	XFILE *out = xfopen("stdout:");
	if(!in || !out) {
		perror();
		fail();
	}

	uint512_t k = nlis512((uint512_t){{1}}, (uint512_t){{0}}, ROUNDS);
	bool    (*coder)(size_t z, uint8_t b[z], uint512_t k, XFILE *in, XFILE *out) = 0;
	int       keytype = 0;
	size_t    keylen = 0;
	char     *key = NULL;

	int argi = 1;
	while((argi < argc) && (*argv[argi] == '-')) {
		char const *args = argv[argi++];
		char const *argp = NULL;
		do {
			int argn   = argc - argi;
			int params = 0;
			int opt    = optget(n_options - 2, options + 2, &argp, args, argn, &params);
			switch(opt) {
			case 1:
				optuse(n_options, options, argv[0], stdout);
				return 0;
			case 2:
				xfclose(out);
				out = xfopen(argv[argi], "wb");
				if(!out) {
					perror(argv[argi]);
					fail();
				}
				break;
			case 5:
				keytype = 1;
				break;
			case 6:
				keytype = 2;
				break;
			case 8:
				coder = encode;
				goto get_key;
			case 9:
				coder = decode;
			get_key:
				key = argv[argi];
				if(keytype) {
					XFILE *kf = xfopen(key, (keytype > 1) ? "rb" : "r");
					if(!kf) {
						perror(key);
						fail();
					}
					if(keytype > 1) {
						key = calloc(1, sizeof(k));
						if(!key) {
							perror();
							fail();
						}
						keylen = xfread(key, 1, sizeof(k), kf);
					} else {
						key = mfgets(xfstream(kf));
						if(key) {
							keylen = strlen(key);
						}
					}
					if(xferror(kf)) {
						perror(xfname(kf));
						fail();
					}
					xfclose(kf);
				} else {
					keylen = strlen(key);
				}
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
	if(!coder) {
		optuse(n_options, options, argv[0], stderr);
		fail();
	}

	size_t   const z = BUFSIZ * 64;
	uint8_t *const b = calloc(z, sizeof(*b));
	if(!b) {
		perror();
		memzero((void *)key, keylen);
		xfclose(out);
		fail();
	}
	if(keytype < 2) {
		size_t n = base64declen(keylen);
		char  *s = calloc(n, sizeof(*s));
		if(!s) {
			perror();
			memzero((void *)key, keylen);
			xfclose(out);
			fail();
		}
		base64decode(n, s, key, 0);
		memzero((void *)key, keylen);
		if(keytype) {
			free((void *)key);
		}
		key = s;
		keylen = n;
	}
	k = initialize(k, key, keylen);
	memzero((void *)key, keylen);
	if(keytype) {
		free((void *)key);
	}
	if(iszero512(k)) {
		perror();
		xfclose(out);
		fail();
	}

#ifdef _WIN32
	if(xfisstd(in)) _setmode(_fileno(xfstream(in)), _O_BINARY);
	if(xfisstd(out)) _setmode(_fileno(xfstream(out)), _O_BINARY);
#endif
	do {
		if(argi < argc) {
			char const *args = argv[argi++];
			in = xfclose(in);
			in = xfopen(args, "rb");
			if(!in) {
#ifdef EACCES
				if(errno == EACCES) continue;
#endif
#ifdef EPERM
				if(errno == EPERM) continue;
#endif
				perror(args);
				memzero(b, z * sizeof(*b));
				memzero(&k, sizeof(k));
				xfclose(out);
				fail();
			}
		}
		if(!coder(z, b, k, in, out)) {
			if(xferror(out)) perror(xfname(out));
			else             perror(xfname(in));
			memzero(b, z * sizeof(*b));
			memzero(&k, sizeof(k));
			xfclose(out);
			xfclose(in);
			fail();
		}
	} while(argi < argc)
		;
	memzero(b, z * sizeof(*b));
	memzero(&k, sizeof(k));
	xfclose(out);
	xfclose(in);
	return 0;
}
