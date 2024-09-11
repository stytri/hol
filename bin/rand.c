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
#include <stdarg.h>
#include <errno.h>
#include <math.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//==============================================================================

#define WIP__BODY(WIP__BODY__Nbits,WIP__BODY__Begin,WIP__BODY__End) \
\
	int                        const xr = WIP__BODY__Nbits / 4;              \
	int                        const xl = WIP__BODY__Nbits - xr;             \
	int                        const yl = (WIP__BODY__Nbits / 2) - 1;        \
	int                        const yr = WIP__BODY__Nbits - yl;             \
	int                        const zl = 1;                                 \
	int                        const zr = WIP__BODY__Nbits - zl;             \
	int                        const gl = (WIP__BODY__Nbits - 3) / 2;        \
	int                        const gr = (WIP__BODY__Nbits - 5) / 3;        \
	uint##WIP__BODY__Nbits##_t const c  = wip##WIP__BODY__Nbits##__c();      \
	WIP__BODY__Begin {                                                       \
	uint##WIP__BODY__Nbits##_t const w  =  r->a[4] += c;                     \
	uint##WIP__BODY__Nbits##_t const g  = rotl(w, gl) + rotr(w, gr) + c;     \
	uint##WIP__BODY__Nbits##_t const x  = (r->a[3] << xl) | (r->a[2] >> xr); \
	uint##WIP__BODY__Nbits##_t const y  = (r->a[2] << yl) | (r->a[1] >> yr); \
	uint##WIP__BODY__Nbits##_t const z  = (r->a[1] << zl) | (r->a[3] >> zr); \
	uint##WIP__BODY__Nbits##_t const v  =  r->a[0] ^ g;                      \
	uint##WIP__BODY__Nbits##_t const u  = x + y + z;                         \
	uint##WIP__BODY__Nbits##_t const t  = ~(v ^ u);                          \
	uint##WIP__BODY__Nbits##_t const s  = t + w;                             \
	r->a[0]  = r->a[1];                                                      \
	r->a[1] ^= r->a[2];                                                      \
	r->a[2]  = r->a[3];                                                      \
	r->a[3] ^= t;                                                            \
	WIP__BODY__End;                                                          \
	}

#define WIP(WIP__Nbits) \
\
struct wip##WIP__Nbits { \
	uint##WIP__Nbits##_t a[5]; \
}; \
typedef struct wip##WIP__Nbits WIP##WIP__Nbits; \
\
static void \
wseed##WIP__Nbits( \
	WIP##WIP__Nbits     *r, \
	uint##WIP__Nbits##_t s  \
) { \
	r->a[0] = r->a[1] = r->a[2] = r->a[3] = r->a[4] = s; \
} \
\
static void \
winit##WIP__Nbits( \
	WIP##WIP__Nbits *r, \
	char const      *cs \
) { \
	xrand_init(cs, sizeof(r->a), r->a); \
} \
\
static inline \
uint##WIP__Nbits##_t \
wip##WIP__Nbits##__c( \
	void \
) { \
	return IPOW(UINT##WIP__Nbits##_C(11), (WIP__Nbits*2)/7); \
} \
\
static void \
wfill##WIP__Nbits( \
	WIP##WIP__Nbits      *r, \
	uint##WIP__Nbits##_t *b, \
	size_t                n \
) { \
	WIP__BODY(WIP__Nbits, while(n-- > 0), *b++ = s) \
} \
\
static uint##WIP__Nbits##_t \
wip##WIP__Nbits( \
	WIP##WIP__Nbits *r \
) { \
	WIP__BODY(WIP__Nbits, , return s) \
}

WIP(64)
WIP(32)
WIP(16)
WIP(8)

#undef WIP

//------------------------------------------------------------------------------

#define WIP_WRAPPERS(WIP_WRAPPERS__Nbits) \
\
static uint64_t \
wip##WIP_WRAPPERS__Nbits##_rand( \
	void *p \
) { \
	return wip##WIP_WRAPPERS__Nbits(p); \
} \
\
static void \
wip##WIP_WRAPPERS__Nbits##_fill( \
	void  *p, \
	void  *b, \
	size_t n \
) { \
	return wfill##WIP_WRAPPERS__Nbits(p, b, n); \
} \
\
static void \
wip##WIP_WRAPPERS__Nbits##_seed( \
	void    *p, \
	uint64_t s \
) { \
	wseed##WIP_WRAPPERS__Nbits(p, s); \
} \
\
static void \
wip##WIP_WRAPPERS__Nbits##_init( \
	void       *p, \
	char const *s \
) { \
	winit##WIP_WRAPPERS__Nbits(p, s); \
}

WIP_WRAPPERS(8)
WIP_WRAPPERS(16)
WIP_WRAPPERS(32)
WIP_WRAPPERS(64)

#undef WIP_WRAPPERS

//==============================================================================

#define XRAND_WRAPPERS(XRAND_WRAPPERS__Nbits) \
\
static uint64_t \
xrand##XRAND_WRAPPERS__Nbits##_rand( \
	void *p \
) { \
	return xrand##XRAND_WRAPPERS__Nbits(p); \
} \
\
static void \
xrand##XRAND_WRAPPERS__Nbits##_fill( \
	void  *p, \
	void  *b, \
	size_t n \
) { \
	return xfill##XRAND_WRAPPERS__Nbits(p, b, n); \
} \
\
static void \
xrand##XRAND_WRAPPERS__Nbits##_seed( \
	void    *p, \
	uint64_t s \
) { \
	xseed##XRAND_WRAPPERS__Nbits(p, s); \
} \
\
static void \
xrand##XRAND_WRAPPERS__Nbits##_init( \
	void       *p, \
	char const *s \
) { \
	xinit##XRAND_WRAPPERS__Nbits(p, s); \
}

XRAND_WRAPPERS(8)
XRAND_WRAPPERS(16)
XRAND_WRAPPERS(32)
XRAND_WRAPPERS(64)

#undef XRAND_WRAPPERS

//==============================================================================

#define NLIS_WRAPPERS(NLIS__Bits,NLIS__Rounds) \
\
struct nlis##NLIS__Bits { \
	uint##NLIS__Bits##_t a[2]; \
}; \
\
static inline uint64_t \
nlis##NLIS__Bits##_rand( \
	struct nlis##NLIS__Bits *r \
) { \
	int                  n = NLIS__Rounds; \
	uint##NLIS__Bits##_t k = IPOW(UINT##NLIS__Bits##_C(33),NLIS__Bits/5); \
	uint##NLIS__Bits##_t x = r->a[0]++ ^ r->a[1]; \
	uint##NLIS__Bits##_t s = nlis##NLIS__Bits(x, k, n); \
	if(likely(r->a[0])) return s; \
	r->a[1] += IPOW(UINT##NLIS__Bits##_C(211), NLIS__Bits/8); \
	return s; \
} \
\
static void \
nlis##NLIS__Bits##_fill( \
	void  *r, \
	void  *b, \
	size_t n \
) { \
	for(uint##NLIS__Bits##_t *u = b; n-- > 0; ) { \
		*u++ = (uint##NLIS__Bits##_t)nlis##NLIS__Bits##_rand(r); \
	} \
} \
\
static void \
nlis##NLIS__Bits##_seed( \
	struct nlis##NLIS__Bits *r, \
	uint64_t                 s \
) { \
	r->a[0] = 0; \
	r->a[1] = s; \
} \
\
static void \
nlis##NLIS__Bits##_init( \
	struct nlis##NLIS__Bits *r, \
	char const              *s \
) { \
	xrand_init(s, sizeof(*r), r); \
}

NLIS_WRAPPERS(8,4)
NLIS_WRAPPERS(16,6)
NLIS_WRAPPERS(32,7)
NLIS_WRAPPERS(64,11)

#undef NLIS_WRAPPERS

//==============================================================================

#define ULIS_WRAPPERS(ULIS__Bits) \
\
struct ulis##ULIS__Bits { \
	uint##ULIS__Bits##_t a[2]; \
}; \
\
static inline uint64_t \
ulis##ULIS__Bits##_rand( \
	struct ulis##ULIS__Bits *r \
) { \
	int                  n = MSBIT(ULIS__Bits); \
	uint##ULIS__Bits##_t k = IPOW(UINT##ULIS__Bits##_C(33),ULIS__Bits/5); \
	uint##ULIS__Bits##_t x = r->a[0]++ ^ r->a[1]; \
	uint##ULIS__Bits##_t s = ulis##ULIS__Bits(x, k, n); \
	if(likely(r->a[0])) return s; \
	r->a[1] += IPOW(UINT##ULIS__Bits##_C(211), ULIS__Bits/8); \
	return s; \
} \
\
static void \
ulis##ULIS__Bits##_fill( \
	void  *r, \
	void  *b, \
	size_t n \
) { \
	for(uint##ULIS__Bits##_t *u = b; n-- > 0; ) { \
		*u++ = (uint##ULIS__Bits##_t)ulis##ULIS__Bits##_rand(r); \
	} \
} \
\
static void \
ulis##ULIS__Bits##_seed( \
	struct ulis##ULIS__Bits *r, \
	uint64_t                 s \
) { \
	r->a[0] = 0; \
	r->a[1] = s; \
} \
\
static void \
ulis##ULIS__Bits##_init( \
	struct ulis##ULIS__Bits *r, \
	char const              *s \
) { \
	xrand_init(s, sizeof(*r), r); \
}

ULIS_WRAPPERS(8)
ULIS_WRAPPERS(16)
ULIS_WRAPPERS(32)
ULIS_WRAPPERS(64)

#undef ULIS_WRAPPERS

//==============================================================================

// The Classic BAD PRNG

#define RANDU  struct randu
struct randu {
	uint32_t a;
};

static inline uint64_t
randu_rand(
	RANDU *r
) {
	uint32_t const t = ((r->a * 65539lu) % (UINT32_C(1) << 31)) & UINT32_C(0x7FFFFFFF);
	r->a = t;
	return t;
}

static void
randu_fill(
	void  *r,
	void  *b,
	size_t n
) {
	for(uint32_t *u = b; n-- > 0; ) {
		*u++ = (uint32_t)randu_rand(r);
	}
}

static void
randu_seed(
	RANDU   *r,
	uint64_t s
) {
	r->a = s;
}

static void
randu_init(
	RANDU      *r,
	char const *s
) {
	xrand_init(s, sizeof(*r), r);
}

//==============================================================================

// The C Library PRNG

struct lib {
};

static inline uint64_t
lib_rand(
	void *r
) {
	(void)r;
	return (uint64_t)rand() & BITMASK(RAND_MAX);
}

static void
lib_fill(
	void  *r,
	void  *b,
	size_t n
) {
	size_t const m = MSBIT(RAND_MAX);
	if(m > 32) {
		for(uint64_t *u = b; n-- > 0; ) {
			*u++ = (uint64_t)lib_rand(r);
		}
	} else if(m > 16) {
		for(uint32_t *u = b; n-- > 0; ) {
			*u++ = (uint32_t)lib_rand(r);
		}
	} else if(m > 8) {
		for(uint16_t *u = b; n-- > 0; ) {
			*u++ = (uint16_t)lib_rand(r);
		}
	} else {
		for(uint8_t *u = b; n-- > 0; ) {
			*u++ = (uint8_t)lib_rand(r);
		}
	}
}

static void
lib_seed(
	void    *r,
	uint64_t s
) {
	(void)r;
	srand((unsigned)s);
}

static void
lib_init(
	struct lib *r,
	char const *s
) {
	uint64_t x;
	xrand_init(s, sizeof(x), &x);
	lib_seed(r, x);
}

//==============================================================================

#define PRNG  struct prng
struct prng {
	char const *name;
	size_t      size;
	void       *state[2];
	void      (*init)(void *p, char const *s);
	void      (*seed)(void *p, uint64_t s);
	uint64_t  (*next)(void *p);
	void      (*fill)(void *p, void *b, size_t n);
	size_t      bits;
};

#define PRNG_WRAP(PRNG_WRAP__type) \
static void \
PRNG_WRAP__type##__init_wrapper( \
	void       *p, \
	char const *s \
) { \
	PRNG_WRAP__type##_init(p, s); \
} \
static void \
PRNG_WRAP__type##__seed_wrapper( \
	void    *p, \
	uint64_t s \
) { \
	PRNG_WRAP__type##_seed(p, s); \
} \
static uint64_t \
PRNG_WRAP__type##__rand_wrapper( \
	void *p \
) { \
	return PRNG_WRAP__type##_rand(p); \
} \
static void \
PRNG_WRAP__type##__fill_wrapper( \
	void   *p, \
	void   *b, \
	size_t  n \
) { \
	return PRNG_WRAP__type##_fill(p, b, n); \
}

PRNG_WRAP(wip8)
PRNG_WRAP(wip16)
PRNG_WRAP(wip32)
PRNG_WRAP(wip64)

PRNG_WRAP(xrand8)
PRNG_WRAP(xrand16)
PRNG_WRAP(xrand32)
PRNG_WRAP(xrand64)

PRNG_WRAP(nlis8)
PRNG_WRAP(nlis16)
PRNG_WRAP(nlis32)
PRNG_WRAP(nlis64)

PRNG_WRAP(ulis8)
PRNG_WRAP(ulis16)
PRNG_WRAP(ulis32)
PRNG_WRAP(ulis64)

PRNG_WRAP(randu)

PRNG_WRAP(lib)

#undef PRNG_WRAP

PRNG table[] = {
#define PRNG_ENTRY(PRNG_ENTRY__name,PRNG_ENTRY__type,PRNG_ENTRY__bits)  \
	{ \
		PRNG_ENTRY__name, \
		sizeof(struct PRNG_ENTRY__type), \
		{ NULL, NULL },\
		PRNG_ENTRY__type##__init_wrapper, \
		PRNG_ENTRY__type##__seed_wrapper, \
		PRNG_ENTRY__type##__rand_wrapper, \
		PRNG_ENTRY__type##__fill_wrapper, \
		PRNG_ENTRY__bits \
	}

	PRNG_ENTRY("xrand"       , xrand64   , 64),

	PRNG_ENTRY("wip8"        , wip8      ,  8),
	PRNG_ENTRY("wip16"       , wip16     , 16),
	PRNG_ENTRY("wip32"       , wip32     , 32),
	PRNG_ENTRY("wip64"       , wip64     , 64),

	PRNG_ENTRY("xrand8"      , xrand8    ,  8),
	PRNG_ENTRY("xrand16"     , xrand16   , 16),
	PRNG_ENTRY("xrand32"     , xrand32   , 32),
	PRNG_ENTRY("xrand64"     , xrand64   , 64),

	PRNG_ENTRY("nlis8"       , nlis8     ,  8),
	PRNG_ENTRY("nlis16"      , nlis16    , 16),
	PRNG_ENTRY("nlis32"      , nlis32    , 32),
	PRNG_ENTRY("nlis64"      , nlis64    , 64),

	PRNG_ENTRY("ulis8"       , ulis8     ,  8),
	PRNG_ENTRY("ulis16"      , ulis16    , 16),
	PRNG_ENTRY("ulis32"      , ulis32    , 32),
	PRNG_ENTRY("ulis64"      , ulis64    , 64),

	PRNG_ENTRY("randu"       , randu     , 31),

	PRNG_ENTRY("lib"         , lib       , MSBIT(RAND_MAX)),

	{ NULL }
#undef PRNG_ENTRY
};

//------------------------------------------------------------------------------

static void
report_state(
	PRNG *prng,
	int   B,
	int   k
) {
	size_t z = prng->size;
	fputs("state: [", stderr);
	switch(B) {
	case 8:
		for(uint8_t *s = prng->state[k]; z > 0; z -= sizeof(*s), s++) {
			fprintf(stderr, " 0x%.2"PRIX8, *s);
		}
		fputs(" ]\n", stderr);
		break;
	case 16:
		for(uint16_t *s = prng->state[k]; z > 0; z -= sizeof(*s), s++) {
			fprintf(stderr, " 0x%.4"PRIX16, *s);
		}
		fputs(" ]\n", stderr);
		break;
	case 32:
		for(uint32_t *s = prng->state[k]; z > 0; z -= sizeof(*s), s++) {
			fprintf(stderr, " 0x%.8"PRIX32, *s);
		}
		fputs(" ]\n", stderr);
		break;
	case 64:
		for(uint64_t *s = prng->state[k]; z > 0; z -= sizeof(*s), s++) {
			fprintf(stderr, " 0x%.16"PRIX64, *s);
		}
		fputs(" ]\n", stderr);
		break;
	}
}

//------------------------------------------------------------------------------

static volatile sig_atomic_t gSignal = 0;

static void
signal_handler(
	int signal
) {
    gSignal = signal;
}

static int
output(
	void  *b,
	size_t z,
	size_t n
)  {
	while(!gSignal && (n > 0)) {
		size_t m = fwrite(b, z, n, stdout);
		if(ferror(stdout)) return errno;
		b  = (char *)b + m;
		n -= m;
	}
	return 0;
}

//------------------------------------------------------------------------------

static size_t
get_size_and_count(
	char   *args,
	size_t *count
) {
	size_t n = strtozs(args, &args, 0), z = 1;
	if(*args) {
		z = strtozs(args+1, NULL, 0);
	} else {
		while(((n & 1) == 0) && (z < (1 Mi))) {
			n >>= 1;
			z <<= 1;
		}
	}
	*count = n;
	return z;
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
	int   argc,
	char *argv[]
) {
#endif
	static struct optget options[] = {
		{  0, "usage: %s [options] [PRNG] COUNT[xSIZE] [FILE]", NULL },
		{  0, "options:",          NULL },
		{  1, "-h, --help",        "display help" },
		{ 90, "-l, --list",        "list available PRNGs" },

		{  2, "-z, --zero",        "set prng state to all bits clear" },
		{  3, "-f, --fill",        "set prng state to all bits set" },
		{  4, "-s, --seed SEED",   "set prng state using SEED value" },
		{  5, "-p, --purge COUNT", "purge the first COUNT values" },
		{  7, "-2, --interleave",  "interleave two generators" },
		{  8, "-1, --single",      "single generator" },

		{ 10, "-g, --pbm",         "output .pbm file header" },

		{ 80, "-T, --utc-time",    "timed execution" },
#ifdef TIMESTAMP_REALTIME
		{ 81, "    --realtime",    NULL },
#endif
#ifdef TIMESTAMP_MONOTIME
		{ 82, "    --monotime",    NULL },
#endif
#ifdef TIMESTAMP_CPU_TIME
		{ 83, "    --cpu-time",    NULL },
#endif
		{ 98, "-E, --info-to-stderr",    "output information to stderr" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },

	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	if(argc == 1) {
		optuse(n_options, options, argv[0], stdout);
		return 0;
	}

	bool   ignore_interrupts = false;
	bool   noexecute         = false;
	size_t interleaved       = 0;
	bool   zeroed            = false;
	bool   filled            = false;
	char  *seed              = NULL;
	size_t purge             = 0;
	bool   pbm               = false;
	int    timed             = NO_TIMESTAMP;
	bool   report_info       = false;

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
				noexecute = true;
				break;
			case 2:
				zeroed = true;
				filled = false;
				seed   = NULL;
				break;
			case 3:
				zeroed = false;
				filled = true;
				seed   = NULL;
				break;
			case 4:
				zeroed = false;
				filled = false;
				seed   = argv[argi];
				break;
			case 5: purge = streval(argv[argi], NULL, 0); break;
			case 7: interleaved = 1; break;
			case 8: interleaved = 0; break;
			case 10: pbm = true; break;
			case 80: timed = TIMESTAMP_UTC_TIME; break;
#ifdef TIMESTAMP_REALTIME
			case 81: timed = TIMESTAMP_REALTIME; break;
#endif
#ifdef TIMESTAMP_MONOTIME
			case 82: timed = TIMESTAMP_MONOTIME; break;
#endif
#ifdef TIMESTAMP_CPU_TIME
			case 83: timed = TIMESTAMP_CPU_TIME; break;
#endif
			case 90: {
					int w = 0;
					for(size_t i = 0; table[i].name != NULL; i++) {
						int n = strlen(table[i].name);
						if(w < n) w = n;
					}
					for(size_t i = 0; table[i].name != NULL; i++) {
						printf("%-*s: %3zu byte state: %4zu bit value\n", w, table[i].name, table[i].size, table[i].bits);
					}
				}
				noexecute = true;
				break;
			case 98: report_info = true; break;
			case 99: ignore_interrupts = true; break;
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				fail();
			}
			argi += params;
		} while(argp)
			;
	}

	if(noexecute) {
		return 0;
	}

	if(ignore_interrupts) {
		signal(SIGINT, SIG_IGN);
	} else {
		signal(SIGINT, signal_handler);
	}

	PRNG *prng = &table[0];
	if(argi < argc) {
		for(size_t i = 0; table[i].name != NULL; i++) {
			if(strcmp(table[i].name, argv[argi]) == 0) {
				prng = &table[i];
				argi++;
				break;
			}
		}
	}
	prng->state[0] = prng->size ? malloc(prng->size) : NULL;
	if(prng->size && !prng->state[0]) {
		perror();
		abort();
	}
	if(interleaved) {
		prng->state[1] = prng->size ? malloc(prng->size) : NULL;
		if(prng->size && !prng->state[1]) {
			perror();
			abort();
		}
	}

	char           *argy = (argi < argc) ? argv[argi++] : "";
	size_t   const  B    = (prng->bits > 8) ? ((prng->bits > 16) ? ((prng->bits > 32) ? 64 : 32) : 16) : 8;
	size_t          Z    = 1;
	size_t   const  Y    = get_size_and_count(argy, &Z);
	size_t   const  X    = pbm ? (((Z + (B - 1)) / B) * B) : Z;
	size_t   const  N    = pbm ? ((X / B) * Y) : Y;
	char     const *file = NULL;
#ifdef _WIN32
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	if(argi < argc) {
		file = argv[argi++];
		freopen(file, "wb", stdout);
	}
#ifdef _WIN32
	setbuf(stdout, NULL);
#endif

	if(report_info) {
		fprintf(stderr,"bits : %zu\n", B);
		fprintf(stderr,"count: %zu / %zu\n", X, Z);
		fprintf(stderr,"size : %zu / %zu\n", Y, N);
	}

	if(zeroed) {
		if(report_info) fprintf(stderr,"seed : zeroed\n");
		memset(prng->state[0], 0, prng->size);
	} else if(filled) {
		if(report_info) fprintf(stderr,"seed : filled\n");
		memset(prng->state[0], ~0, prng->size);
	} else if(seed) {
		if(report_info) fprintf(stderr,"seed : %s\n", seed);
		if(*seed == '@') {
			FILE *in = fopen(seed+1, "rb");
			if(!in || (fread(prng->state[0], prng->size, 1, in) != 1)) {
				perror(seed+1);
				fail();
			}
			fclose(in);
		} else {
			char     *t;
			uintmax_t s = strtoumax(seed, &t, 0);
			if(*t) prng->init(prng->state[0], seed);
			else prng->seed(prng->state[0], s);
		}
	} else {
		uint64_t s = xrandseed();
		if(report_info) fprintf(stderr,"seed : 0x%"PRIx64"\n", s);
		prng->seed(prng->state[0], s);
	}
	if(report_info) report_state(prng, B, 0);

	if(interleaved) {
		uint8_t       *b = prng->state[1];
		uint8_t const *s = prng->state[0];
		uint8_t const *e = s + prng->size;
		*b = ~*s ^ 1;
		for(b++, s++; s != e; b++, s++) {
			*b = ~*s;
		}
		if(report_info) report_state(prng, B, 1);
	}

	uint64_t (*prng_next)(void *)                 = prng->next;
	void     (*prng_fill)(void *, void *, size_t) = prng->fill;

	size_t count = 0;
	struct timespec t1, t2;
	timestamp(timed, &t1);

#	define MKARRAY_SIZE  (1 Ki)
	if(interleaved) {
		for(; !gSignal && (purge >= (1 Ki)); purge -= 1 Ki, count += 2 Ki) {
#			define MKARRAY_DATA  prng_next(prng->state[0]);
			#include <hol/mkarray.h>
#			undef MKARRAY_DATA
#			define MKARRAY_DATA  prng_next(prng->state[1]);
			#include <hol/mkarray.h>
#			undef MKARRAY_DATA
		}
	} else {
		for(; !gSignal && (purge >= (1 Ki)); purge -= 1 Ki, count += 1 Ki) {
#			define MKARRAY_DATA  prng_next(prng->state[0]);
			#include <hol/mkarray.h>
#			undef MKARRAY_DATA
		}
	}
#	undef MKARRAY_SIZE

	purge <<= interleaved;
	for(size_t k = 0; !gSignal && purge--; k ^= interleaved) {
		prng_next(prng->state[k]);
		count++;
	}

	if(report_info) fflush(stderr);

	int stdout_errno = 0;

	if(N > 0) {
		if(pbm) {
			if(fprintf(stdout, "P4\n%zu %zu\n", X, Y) < 0) {
				stdout_errno = errno;
			}
		}
		if(!stdout_errno) switch(B) {
		case 8:
			{
				uint8_t *buf = malloc(sizeof(*buf) * Y);
				for(size_t x = 0, k = 0; !gSignal && (x < X); x++) {
					if(interleaved) {
						for(size_t y = 0; y < Y; y++, k ^= interleaved) {
							buf[y] = prng_next(prng->state[k]);
						}
					} else {
						prng_fill(prng->state[0], buf, Y);
					}
					count += Y;
					if((stdout_errno = output(buf, sizeof(*buf), Y))) break;
				}
				free(buf);
			}
			break;
		case 16:
			{
				uint16_t *buf = malloc(sizeof(*buf) * Y);
				for(size_t x = 0, k = 0; !gSignal && (x < X); x++) {
					if(interleaved) {
						for(size_t y = 0; y < Y; y++, k ^= interleaved) {
							buf[y] = prng_next(prng->state[k]);
						}
					} else {
						prng_fill(prng->state[0], buf, Y);
					}
					count += Y;
					if((stdout_errno = output(buf, sizeof(*buf), Y))) break;
				}
				free(buf);
			}
			break;
		case 32:
			{
				uint32_t *buf = malloc(sizeof(*buf) * Y);
				for(size_t x = 0, k = 0; !gSignal && (x < X); x++) {
					if(interleaved) {
						for(size_t y = 0; y < Y; y++, k ^= interleaved) {
							buf[y] = prng_next(prng->state[k]);
						}
					} else {
						prng_fill(prng->state[0], buf, Y);
					}
					count += Y;
					if((stdout_errno = output(buf, sizeof(*buf), Y))) break;
				}
				free(buf);
			}
			break;
		case 64:
			{
				uint64_t *buf = malloc(sizeof(*buf) * Y);
				for(size_t x = 0, k = 0; !gSignal && (x < X); x++) {
					if(interleaved) {
						for(size_t y = 0; y < Y; y++, k ^= interleaved) {
							buf[y] = prng_next(prng->state[k]);
						}
					} else {
						prng_fill(prng->state[0], buf, Y);
					}
					count += Y;
					if((stdout_errno = output(buf, sizeof(*buf), Y))) break;
				}
				free(buf);
			}
			break;
		}
	}

	if(report_info || timed) fflush(stdout);

	if(stdout_errno) {
		if(1
#ifdef EPIPE
			&& (stdout_errno != EPIPE)
#endif
		) {
			errno = stdout_errno;
			perror(file);
		}
	} else {
		if(report_info && gSignal) {
			fprintf(stderr,"signal: %i\n", (int)gSignal);
		}
		if(timed) {
			timestamp(timed, &t2);
			time_interval(&t1, &t2, &t2);
			time_per(&t2, count, &t1);
			fprintf(stderr, "%zu words in ", count);
			fprint_time_interval(stderr, t2.tv_sec, t2.tv_nsec);
			fputs(" = ", stderr);
			fprint_time_interval(stderr, t1.tv_sec, t1.tv_nsec);
			fputs(" per word\n", stderr);
		}
	}

	free(prng->state[0]);
	if(interleaved) free(prng->state[1]);

	if(report_info) fputs("done\n", stderr);

	return 0;
}
