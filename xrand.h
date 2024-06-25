#ifndef HOL_XRAND_H__INCLUDED
#define HOL_XRAND_H__INCLUDED 1
/*
MIT License

Copyright (c) 2022 Tristan Styles

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

#include <stdint.h>

//------------------------------------------------------------------------------

#ifndef HOL_XRAND_H__SELECT
#	if SIZE_MAX > UINT32_MAX
#		define HOL_XRAND_H__SELECT  (64|32|16|8)
#	elif SIZE_MAX > UINT16_MAX
#		define HOL_XRAND_H__SELECT  (32|16|8)
#	else
#		define HOL_XRAND_H__SELECT  (16|8)
#	endif
#endif

//------------------------------------------------------------------------------

#define NLIS(NLIS__Nbits) \
\
extern uint##NLIS__Nbits##_t nlis##NLIS__Nbits(uint##NLIS__Nbits##_t x, uint##NLIS__Nbits##_t k, int n); \
extern uint##NLIS__Nbits##_t ulis##NLIS__Nbits(uint##NLIS__Nbits##_t x, uint##NLIS__Nbits##_t k, int n);

#if (HOL_XRAND_H__SELECT & 64) != 0
NLIS(64)
#endif
#if (HOL_XRAND_H__SELECT & 32) != 0
NLIS(32)
#endif
#if (HOL_XRAND_H__SELECT & 16) != 0
NLIS(16)
#endif
#if (HOL_XRAND_H__SELECT &  8) != 0
NLIS(8)
#endif

#undef NLIS

//------------------------------------------------------------------------------

extern void      xrand_seed(size_t m, void const *k, size_t n, void *v);
extern void      xrand_init(char const *cs, size_t n, void *v);
extern uintmax_t xrandseed (void);

//------------------------------------------------------------------------------

#define XRAND(XRAND__Nbits) \
\
struct xrand##XRAND__Nbits { \
	uint##XRAND__Nbits##_t a[5]; \
}; \
typedef  struct xrand##XRAND__Nbits  XRAND##XRAND__Nbits; \
\
extern void                   xseed##XRAND__Nbits(XRAND##XRAND__Nbits *r, uint##XRAND__Nbits##_t s); \
extern void                   xkeys##XRAND__Nbits(XRAND##XRAND__Nbits *r, char const *cs); \
extern void                   xinit##XRAND__Nbits(XRAND##XRAND__Nbits *r, char const *cs); \
extern void                   xfill##XRAND__Nbits(XRAND##XRAND__Nbits *r, uint##XRAND__Nbits##_t *b, size_t n); \
extern uint##XRAND__Nbits##_t xrand##XRAND__Nbits(XRAND##XRAND__Nbits *r);

#if (HOL_XRAND_H__SELECT & 64) != 0
XRAND(64)
#endif
#if (HOL_XRAND_H__SELECT & 32) != 0
XRAND(32)
#endif
#if (HOL_XRAND_H__SELECT & 16) != 0
XRAND(16)
#endif
#if (HOL_XRAND_H__SELECT &  8) != 0
XRAND(8)
#endif

#undef XRAND

//------------------------------------------------------------------------------

#endif//ndef HOL_XRAND_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XRAND_H__IMPLEMENTATION
#undef HOL_XRAND_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/xtdlib.h>
#include <hol/xtring.h>
#include <hol/xtime.h>

//------------------------------------------------------------------------------

#define NLIS(NLIS__Nbits) \
\
static inline uint##NLIS__Nbits##_t \
nlis##NLIS__Nbits##__g( \
	uint##NLIS__Nbits##_t x \
) { \
	return x ^ (x >> 1); \
} \
\
static inline uint##NLIS__Nbits##_t \
nlis##NLIS__Nbits##__round( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k \
) { \
	int i = 1; \
	do { \
		x   = nlis##NLIS__Nbits##__g(x); \
		x   = rotl##NLIS__Nbits(x, i); \
		i <<= 1; \
	} while(i < NLIS__Nbits) \
		; \
	x = nlis##NLIS__Nbits##__g(x); \
	do { \
		i >>= 1; \
		x   = rotl##NLIS__Nbits(x, i); \
		x   = nlis##NLIS__Nbits##__g(x); \
	} while(i != 1) \
		; \
	return x + k; \
} \
\
uint##NLIS__Nbits##_t \
nlis##NLIS__Nbits( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k, \
	int                   n \
) { \
	x -= k; \
	while(n-- > 0) { \
		x = nlis##NLIS__Nbits##__round(x, k); \
	} \
	return x; \
} \
\
static inline uint##NLIS__Nbits##_t \
ulis##NLIS__Nbits##__g( \
	uint##NLIS__Nbits##_t x \
) { \
	for(int n = (NLIS__Nbits / 2); n != 0; n >>= 1) { \
		x ^= (x >> n); \
	} \
	return x; \
} \
\
static inline uint##NLIS__Nbits##_t \
ulis##NLIS__Nbits##__round( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k \
) { \
	x -= k; \
	int i = 1; \
	do { \
		x   = ulis##NLIS__Nbits##__g(x); \
		x   = rotr##NLIS__Nbits(x, i); \
		i <<= 1; \
	} while(i < NLIS__Nbits) \
		; \
	x = ulis##NLIS__Nbits##__g(x); \
	do { \
		i >>= 1; \
		x   = rotr##NLIS__Nbits(x, i); \
		x   = ulis##NLIS__Nbits##__g(x); \
	} while(i != 1) \
		; \
	return x; \
} \
\
uint##NLIS__Nbits##_t \
ulis##NLIS__Nbits( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k, \
	int                   n \
) { \
	while(n-- > 0) { \
		x = ulis##NLIS__Nbits##__round(x, k); \
	} \
	return x + k; \
}

#if (HOL_XRAND_H__SELECT & 64) != 0
NLIS(64)
#endif
#if (HOL_XRAND_H__SELECT & 32) != 0
NLIS(32)
#endif
#if (HOL_XRAND_H__SELECT & 16) != 0
NLIS(16)
#endif
NLIS(8)

#undef NLIS

//------------------------------------------------------------------------------

static inline
uint8_t
xrand_sbox(
	uint8_t b
) {
	return nlis8(b, 33, 2);
}

static inline
uint8_t
xrand_hash(
	uint8_t h,
	uint8_t b
) {
	h += b;
	h  = rotr(h, 3);
	h ^= xrand_sbox(b);
	return h;
}

//------------------------------------------------------------------------------

void
xrand_seed(
	size_t      m,
	void const *k,
	size_t      n,
	void       *v
) {
	size_t i = 0;
	size_t j = 0;
	size_t r = m * n;
	do {
		((uint8_t *)v)[j] = xrand_hash(((uint8_t const *)k)[i], r--);
		j = (j + 1) % n;
		i = (i + 1) % m;
	} while(j)
		;
	while(r--) {
		((uint8_t *)v)[j] = xrand_hash(((uint8_t const *)v)[j], ((uint8_t const *)k)[i]);
		j = (j + 1) % n;
		i = (i + 1) % m;
	}
	return;
}

void
xrand_init(
	char const *cs,
	size_t      n,
	void       *v
) {
	if(*cs == '[') {
		strtomem(cs+1, NULL, -1, n, v);
	} else if(isdigit(*cs)) {
		uint64_t s = strtoull(cs, NULL, 0);
		xrand_seed(sizeof(s), &s, n, v);
	} else {
		cs += (*cs == '#');
		xrand_seed(strlen(cs), cs, n, v);
	}
}

uintmax_t
xrandseed(
	void
) {
	static uintmax_t n = 0;
	for(;;) {
		time_t    t = time();
		uintmax_t f = (uintmax_t)t * (n += UINTMAX_C(0xBAD5EED));
		uintmax_t s = f ^ (f << (BITS(f) / 2));
		if(likely(s)) return s;
	};
}

//------------------------------------------------------------------------------

#define XRAND__BODY(XRAND__BODY__Nbits,XRAND__BODY__Begin,XRAND__BODY__End) \
\
	int                          const xr = XRAND__BODY__Nbits / 4;            \
	int                          const xl = XRAND__BODY__Nbits - xr;           \
	int                          const yl = (XRAND__BODY__Nbits / 2) - 1;      \
	int                          const yr = XRAND__BODY__Nbits - yl;           \
	int                          const zl = 1;                                 \
	int                          const zr = XRAND__BODY__Nbits - zl;           \
	int                          const gl = (XRAND__BODY__Nbits - 3) / 2;      \
	int                          const gr = (XRAND__BODY__Nbits - 5) / 3;      \
	uint##XRAND__BODY__Nbits##_t const c  = xrand##XRAND__BODY__Nbits##__c();  \
	XRAND__BODY__Begin {                                                       \
	uint##XRAND__BODY__Nbits##_t const w  =  r->a[4] += c;                     \
	uint##XRAND__BODY__Nbits##_t const g  = rotl(w, gl) + rotr(w, gr) + c;     \
	uint##XRAND__BODY__Nbits##_t const x  = (r->a[3] << xl) | (r->a[2] >> xr); \
	uint##XRAND__BODY__Nbits##_t const y  = (r->a[2] << yl) | (r->a[1] >> yr); \
	uint##XRAND__BODY__Nbits##_t const z  = (r->a[1] << zl) | (r->a[3] >> zr); \
	uint##XRAND__BODY__Nbits##_t const v  =  r->a[0] ^ g;                      \
	uint##XRAND__BODY__Nbits##_t const u  = x + y + z;                         \
	uint##XRAND__BODY__Nbits##_t const t  = ~(v ^ u);                          \
	uint##XRAND__BODY__Nbits##_t const s  = t + w;                             \
	r->a[0]  = r->a[1];                                                        \
	r->a[1] ^= r->a[2];                                                        \
	r->a[2]  = r->a[3];                                                        \
	r->a[3] ^= t;                                                              \
	XRAND__BODY__End;                                                          \
	}

#define XRAND(XRAND__Nbits) \
\
void \
xseed##XRAND__Nbits( \
	XRAND##XRAND__Nbits   *r, \
	uint##XRAND__Nbits##_t s  \
) { \
	r->a[0] = r->a[1] = r->a[2] = r->a[3] = r->a[4] = s; \
} \
\
void \
xkeys##XRAND__Nbits( \
	XRAND##XRAND__Nbits *r, \
	char const          *cs \
) { \
	xrand_seed(strlen(cs), cs, sizeof(r->a), r->a); \
} \
\
void \
xinit##XRAND__Nbits( \
	XRAND##XRAND__Nbits *r, \
	char const          *cs \
) { \
	xrand_init(cs, sizeof(r->a), r->a); \
} \
\
static inline \
uint##XRAND__Nbits##_t \
xrand##XRAND__Nbits##__c( \
	void \
) { \
	return IPOW(UINT##XRAND__Nbits##_C(11), (XRAND__Nbits*2)/7); \
} \
\
void \
xfill##XRAND__Nbits( \
	XRAND##XRAND__Nbits    *r, \
	uint##XRAND__Nbits##_t *b, \
	size_t                  n \
) { \
	XRAND__BODY(XRAND__Nbits, while(n-- > 0), *b++ = s) \
} \
\
uint##XRAND__Nbits##_t \
xrand##XRAND__Nbits( \
	XRAND##XRAND__Nbits *r \
) { \
	XRAND__BODY(XRAND__Nbits, , return s) \
}

#if (HOL_XRAND_H__SELECT & 64) != 0
XRAND(64)
#endif
#if (HOL_XRAND_H__SELECT & 32) != 0
XRAND(32)
#endif
#if (HOL_XRAND_H__SELECT & 16) != 0
XRAND(16)
#endif
#if (HOL_XRAND_H__SELECT &  8) != 0
XRAND(8)
#endif

#undef XRAND

//------------------------------------------------------------------------------

#endif//def HOL_XRAND_H__IMPLEMENTATION
