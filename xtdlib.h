#ifndef HOL_XTDLIB_H__INCLUDED
#define HOL_XTDLIB_H__INCLUDED 1
/*
MIT License

Copyright (c) 2021 Tristan Styles

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
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

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

//------------------------------------------------------------------------------

#define Ki  <<10
#define Mi  <<20
#define Gi  <<30
#define Ti  <<40
#define Pi  <<50
#define Ei  <<60

//------------------------------------------------------------------------------

#if !defined(likely) && defined(__GNUC__)
#	define likely(x)    __builtin_expect(!!(x),1)
#	define unlikely(x)  __builtin_expect(!!(x),0)
#elif !defined(likely)
#	define likely(x)    (x)
#	define unlikely(x)  (x)
#endif

//------------------------------------------------------------------------------

#define exit(...)    (exit)(__VA_ARGS__+0)
#define fail(...)    (exit)(EXIT_FAILURE)

//------------------------------------------------------------------------------

#ifndef CONCAT
#define CONCAT(CONCAT__x,CONCAT__y)  CONCAT__x##CONCAT__y
#endif

#define BITS(BITS__x)      ((int)(sizeof(BITS__x) * CHAR_BIT))

#define TYPE_MASK(TYPE_MASK__type)  (~0ull >> (BITS(~0ull) - BITS(TYPE_MASK__type)))

#define SIZE_BIT           (BITS(size_t))
#define SIZE_MIN           (sizeof(size_t))
#if SIZE_MAX > ULONG_MAX
#	define SIZE_C(SIZE_C__x)  (SIZE_C__x##ull)
#elif SIZE_MAX > UINT_MAX
#	define SIZE_C(SIZE_C__x)  (SIZE_C__x##ul)
#else
#	define SIZE_C(SIZE_C__x)  (SIZE_C__x##u)
#endif

#define BITMASK32(BITMASK__x)                ((BITMASK__x) | ((BITMASK__x) >> 8 >> 8 >> 8 >> 8))
#define BITMASK16(BITMASK__x)      BITMASK32((BITMASK__x) | ((BITMASK__x) >> 8 >> 8))
#define BITMASK8(BITMASK__x)       BITMASK16((BITMASK__x) | ((BITMASK__x) >> 8))
#define BITMASK4(BITMASK__x)       BITMASK8((BITMASK__x)  | ((BITMASK__x) >> 4))
#define BITMASK2(BITMASK__x)       BITMASK4((BITMASK__x)  | ((BITMASK__x) >> 2))
#define BITMASK(BITMASK__x)        BITMASK2((BITMASK__x)  | ((BITMASK__x) >> 1))

#define ISPO2(ISPO2__x)            (((ISPO2__x) & ((ISPO2__x) - 1)) == 0)
#define PO2(PO2__x)                (ISPO2(PO2__x) ? ((PO2__x) : (BITMASK(PO2__x) + 1)))

#define MASK64(MASK64__x)                ((MASK64__x) | ((MASK64__x) << 8 << 8 << 8 << 8))
#define MASK32(MASK32__x)          MASK64((MASK32__x) | ((MASK32__x) << 8 << 8))
#define MASK16(MASK16__x)          MASK32((MASK16__x) | ((MASK16__x) << 8))

#define POPCOUNT__MASK8            MASK32(0x00FFull)
#define POPCOUNT__MASK4            MASK32(0x0F0Full)
#define POPCOUNT__MASK2            MASK32(0x3333ull)
#define POPCOUNT__MASK1            MASK32(0x5555ull)

#define POPCOUNTXX(POPCOUNTXX__n,POPCOUNTXX__x)  ( \
	((POPCOUNTXX__x) & CONCAT(POPCOUNT__MASK,POPCOUNTXX__n)) \
	+ (((POPCOUNTXX__x) >> POPCOUNTXX__n) & CONCAT(POPCOUNT__MASK,POPCOUNTXX__n)) \
)

#define POPCOUNT32(POPCOUNT32__x)          ((((POPCOUNT32__x) + ((POPCOUNT32__x) >> 8 >> 8 >> 8 >> 8)) & 0xFF))
#define POPCOUNT16(POPCOUNT16__x)  POPCOUNT32((POPCOUNT16__x) + ((POPCOUNT16__x) >> 8 >> 8))
#define POPCOUNT8(POPCOUNT8__x)    POPCOUNT16(POPCOUNTXX(8,POPCOUNT8__x))
#define POPCOUNT4(POPCOUNT4__x)    POPCOUNT8(POPCOUNTXX(4,POPCOUNT4__x))
#define POPCOUNT2(POPCOUNT2__x)    POPCOUNT4(POPCOUNTXX(2,POPCOUNT2__x))
#define POPCOUNT(POPCOUNT__x)      POPCOUNT2(POPCOUNTXX(1,POPCOUNT__x))

#define BITCOUNT(BITCOUNT__x)      POPCOUNT(BITMASK(BITCOUNT__x))
#define MSBIT(MSBIT__x)            BITCOUNT(MSBIT__x)

#define IPOW__1(IPOW__1__n,IPOW__1__p,IPOW__1__b) ( \
	((IPOW__1__p) >= (IPOW__1__b)) ? (IPOW__1__n) : 1 \
)
#define IPOW__16(IPOW__16__n,IPOW__16__p,IPOW__16__b) ( \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 16) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 15) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 14) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 13) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 12) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 11) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) + 10) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  9) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  8) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  7) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  6) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  5) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  4) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  3) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  2) * \
	IPOW__1(IPOW__16__n, IPOW__16__p, (IPOW__16__b) +  1)   \
)
#define IPOW(IPOW__n,IPOW__p) ( \
	IPOW__16(IPOW__n, IPOW__p, 48) * \
	IPOW__16(IPOW__n, IPOW__p, 32) * \
	IPOW__16(IPOW__n, IPOW__p, 16) * \
	IPOW__16(IPOW__n, IPOW__p,  0)   \
)

//------------------------------------------------------------------------------

static inline bool
is_little_endian(
	void
) {
#if defined(__GNUC__)
	return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#else
	return *(char*)((int[]){1});
#endif
}

//------------------------------------------------------------------------------

#define XTDLIB__BIT1(XTDLIB__BIT1__fn,XTDLIB__BIT1__x)  \
_Generic((XTDLIB__BIT1__x), \
	uint8_t : XTDLIB__BIT1__fn##8, \
	uint16_t: XTDLIB__BIT1__fn##16, \
	uint32_t: XTDLIB__BIT1__fn##32, \
	uint64_t: XTDLIB__BIT1__fn##64 \
)(XTDLIB__BIT1__x)

#define XTDLIB__BIT2(XTDLIB__BIT2__fn,XTDLIB__BIT2__x,...)  \
_Generic((XTDLIB__BIT2__x), \
	uint8_t : XTDLIB__BIT2__fn##8, \
	uint16_t: XTDLIB__BIT2__fn##16, \
	uint32_t: XTDLIB__BIT2__fn##32, \
	uint64_t: XTDLIB__BIT2__fn##64 \
)(XTDLIB__BIT2__x,__VA_ARGS__)

#define XTDLIB__INCWRAP(XTDLIB__INCWRAP__bits)  \
static inline uint##XTDLIB__INCWRAP__bits##_t \
incwrap##XTDLIB__INCWRAP__bits( \
	uint##XTDLIB__INCWRAP__bits##_t x, \
	uint##XTDLIB__INCWRAP__bits##_t i, \
	uint##XTDLIB__INCWRAP__bits##_t m \
) { \
	return (x > (m - i)) ? ( \
		(x + 1) % i \
	) : ( \
		x + i \
	); \
}
XTDLIB__INCWRAP(8)
XTDLIB__INCWRAP(16)
XTDLIB__INCWRAP(32)
XTDLIB__INCWRAP(64)
#undef XTDLIB__INCWRAP
#define incwrap(incwrap__x,incwrap__i,incwrap__m)  XTDLIB__BIT2(incwrap,incwrap__x,incwrap__i,incwrap__m)

#if defined(__GNUC__)
#	define XTDLIB__POPCOUNT(XTDLIB__POPCOUNT__bits)  \
	static inline int \
	popcount##XTDLIB__POPCOUNT__bits( \
		uint##XTDLIB__POPCOUNT__bits##_t x \
	) { \
		if(sizeof(x) > sizeof(unsigned long)) { \
			return __builtin_popcountll((unsigned long long)x); \
		} \
		if(sizeof(x) > sizeof(unsigned)) { \
			return __builtin_popcountl((unsigned long)x); \
		} \
		return __builtin_popcount((unsigned)x); \
	}
#else
	// https://en.wikipedia.org/wiki/Hamming_weight
#	define XTDLIB__POPCOUNT(XTDLIB__POPCOUNT__bits)  \
	static inline int \
	popcount##XTDLIB__POPCOUNT__bits( \
		uint##XTDLIB__POPCOUNT__bits##_t x \
	) { \
		uint##XTDLIB__POPCOUNT__bits##_t const m1 = TYPE_MASK(x) & 0x5555555555555555ull; \
		uint##XTDLIB__POPCOUNT__bits##_t const m2 = TYPE_MASK(x) & 0x3333333333333333ull; \
		uint##XTDLIB__POPCOUNT__bits##_t const m4 = TYPE_MASK(x) & 0x0F0F0F0F0F0F0F0Full; \
		x -= (x >> 1) & m1; \
		x  = (x & m2) + ((x >> 2) & m2); \
		x  = (x + (x >> 4)) & m4; \
		if(BITS(x) >  8) x += x >> 8; \
		if(BITS(x) > 16) x += x >> 16; \
		if(BITS(x) > 32) x += x >> 32; \
		return x & 0x7f; \
	}
#endif
XTDLIB__POPCOUNT(8)
XTDLIB__POPCOUNT(16)
XTDLIB__POPCOUNT(32)
XTDLIB__POPCOUNT(64)
#undef XTDLIB__POPCOUNT
#define popcount(popcount__x)  XTDLIB__BIT1(popcount,popcount__x)

#if defined(__GNUC__)
#	define XTDLIB__LZCOUNT(XTDLIB__LZCOUNT__bits)  \
	static inline int \
	lzcount##XTDLIB__LZCOUNT__bits( \
		uint##XTDLIB__LZCOUNT__bits##_t x \
	) { \
		if(x != 0) { \
			if(sizeof(x) > sizeof(unsigned long)) { \
				return __builtin_clzll((unsigned long long)x); \
			} \
			if(sizeof(x) > sizeof(unsigned)) { \
				return __builtin_clzl((unsigned long)x); \
			} \
			return __builtin_clz((unsigned)x) - (BITS(unsigned) - BITS(x)); \
		} \
		return BITS(x); \
	}
#else
#	define XTDLIB__LZCOUNT__CLZ(XTDLIB__LZCOUNT__CLZ__n) do { \
		int const \
		m   = (x > (SIZE_C(1) << (1 << (XTDLIB__LZCOUNT__CLZ__n)))) << (XTDLIB__LZCOUNT__CLZ__n); \
		n  -= m; \
		x >>= m; \
	} while(0)
#	define XTDLIB__LZCOUNT(XTDLIB__LZCOUNT__name,XTDLIB__LZCOUNT__bits)  \
	static inline int \
	lzcount##XTDLIB__LZCOUNT__bits( \
		uint##XTDLIB__LZCOUNT__bits##t x \
	) { \
		int n = SIZE_BIT; \
		if(BITS(x) > 32) XTDLIB__LZCOUNT__CLZ(5); \
		if(BITS(x) > 16) XTDLIB__LZCOUNT__CLZ(4); \
		if(BITS(x) >  8) XTDLIB__LZCOUNT__CLZ(3); \
		XTDLIB__LZCOUNT__CLZ(2); \
		XTDLIB__LZCOUNT__CLZ(1); \
		XTDLIB__LZCOUNT__CLZ(0); \
		n -= x; \
		return n; \
	}
#	undef XTDLIB__LZCOUNT__CLZ
#endif
XTDLIB__LZCOUNT(8)
XTDLIB__LZCOUNT(16)
XTDLIB__LZCOUNT(32)
XTDLIB__LZCOUNT(64)
#undef XTDLIB__LZCOUNT
#define lzcount(lzcount__x)  XTDLIB__BIT1(lzcount,lzcount__x)

#if defined(__GNUC__)
#	define XTDLIB__TZCOUNT(XTDLIB__TZCOUNT__bits)  \
	static inline int \
	tzcount##XTDLIB__TZCOUNT__bits( \
		uint##XTDLIB__TZCOUNT__bits##_t x \
	) { \
		if(x != 0) { \
			if(sizeof(x) > sizeof(unsigned long)) { \
				return __builtin_ctzll((unsigned long long)x); \
			} \
			if(sizeof(x) > sizeof(unsigned)) { \
				return __builtin_ctzl((unsigned long)x); \
			} \
			int const n = __builtin_ctz((unsigned)x); \
			return (n > BITS(x)) ? BITS(x) : n; \
		} \
		return BITS(x); \
	}
#else
#	define XTDLIB__TZCOUNT__CTZ(XTDLIB__TZCOUNT__CTZ__n) do { \
		int const \
		m   = (!(x & ((SIZE_C(1) << (1 << (XTDLIB__TZCOUNT__CTZ__n))) - 1))) << (XTDLIB__TZCOUNT__CTZ__n); \
		n  += m; \
		x >>= m; \
	} while(0)
#	define XTDLIB__TZCOUNT(XTDLIB__TZCOUNT__bits)  \
	static inline int \
	tzcount##XTDLIB__TZCOUNT__bits( \
		uint##XTDLIB__TZCOUNT__bits##_t x \
	) { \
		int n = SIZE_BIT; \
		if(x) { \
			n = !(x & 1); \
			if(n) { \
				if(BITS(x) > 32) XTDLIB__TZCOUNT__CTZ(5); \
				if(BITS(x) > 16) XTDLIB__TZCOUNT__CTZ(4); \
				if(BITS(x) >  8) XTDLIB__TZCOUNT__CTZ(3); \
				XTDLIB__TZCOUNT__CTZ(2); \
				XTDLIB__TZCOUNT__CTZ(1); \
				n -= x & 1; \
			} \
		} \
		return n; \
	}
#	undef XTDLIB__TZCOUNT__CTZ
#endif
XTDLIB__TZCOUNT(8)
XTDLIB__TZCOUNT(16)
XTDLIB__TZCOUNT(32)
XTDLIB__TZCOUNT(64)
#undef XTDLIB__TZCOUNT
#define tzcount(tzcount__x)  XTDLIB__BIT1(tzcount,tzcount__x)

#define XTDLIB__MSBIT(XTDLIB__MSBIT__bits)  \
static inline int \
msbit##XTDLIB__MSBIT__bits( \
	uint##XTDLIB__MSBIT__bits##_t x \
) { \
	return BITS(x) - lzcount(x); \
}
XTDLIB__MSBIT(8)
XTDLIB__MSBIT(16)
XTDLIB__MSBIT(32)
XTDLIB__MSBIT(64)
#undef XTDLIB__MSBIT
#define msbit(msbit__x)  XTDLIB__BIT1(msbit,msbit__x)

#define XTDLIB__ROTL(XTDLIB__ROTL__bits)  \
static inline uint##XTDLIB__ROTL__bits##_t \
rotl##XTDLIB__ROTL__bits( \
	uint##XTDLIB__ROTL__bits##_t x, \
	int                          n  \
) { \
	return (x << n) | (x >> (BITS(x) - n)); \
}
XTDLIB__ROTL(8)
XTDLIB__ROTL(16)
XTDLIB__ROTL(32)
XTDLIB__ROTL(64)
#undef XTDLIB__ROTL
#define rotl(rotl__x,rotl__n)  XTDLIB__BIT2(rotl,rotl__x,rotl__n)

#define XTDLIB__ROTR(XTDLIB__ROTR__bits)  \
static inline uint##XTDLIB__ROTR__bits##_t \
rotr##XTDLIB__ROTR__bits( \
	uint##XTDLIB__ROTR__bits##_t x, \
	int                          n  \
) { \
	return (x << (BITS(x) - n)) | (x >> n); \
}
XTDLIB__ROTR(8)
XTDLIB__ROTR(16)
XTDLIB__ROTR(32)
XTDLIB__ROTR(64)
#undef XTDLIB__ROTR
#define rotr(rotr__x,rotr__n)  XTDLIB__BIT2(rotr,rotr__x,rotr__n)

#if defined(__GNUC__)
#define XTDLIB__BITMASK(XTDLIB__BITMASK__bits) \
static inline uint##XTDLIB__BITMASK__bits##_t \
bitmask##XTDLIB__BITMASK__bits( \
	uint##XTDLIB__BITMASK__bits##_t x \
) { \
	return (x > 0) ? ( \
		~(uint##XTDLIB__BITMASK__bits##_t)0 >> lzcount(x) \
	):( \
		(uint##XTDLIB__BITMASK__bits##_t)0 \
	); \
}
#else
#define XTDLIB__BITMASK(XTDLIB__BITMASK__bits) \
static inline uint##XTDLIB__BITMASK__bits##_t \
bitmask##XTDLIB__BITMASK__bits( \
	uint##XTDLIB__BITMASK__bits##_t x \
) { \
	x |= x >> 1; \
	x |= x >> 2; \
	x |= x >> 4; \
	if(BITS(x) >  8) x |= x >> 8; \
	if(BITS(x) > 16) x |= x >> 16; \
	if(BITS(x) > 32) x |= x >> 32; \
	return x; \
}
#endif
XTDLIB__BITMASK(8)
XTDLIB__BITMASK(16)
XTDLIB__BITMASK(32)
XTDLIB__BITMASK(64)
#undef XTDLIB__BITMASK
#define bitmask(bitmask__x)  XTDLIB__BIT1(bitmask,bitmask__x)

#define XTDLIB__BITCOUNT(XTDLIB__BITCOUNT__bits) \
static inline uint##XTDLIB__BITCOUNT__bits##_t \
bitcount##XTDLIB__BITCOUNT__bits( \
	uint##XTDLIB__BITCOUNT__bits##_t x \
) { \
	return popcount(bitmask(x)); \
}
XTDLIB__BITCOUNT(8)
XTDLIB__BITCOUNT(16)
XTDLIB__BITCOUNT(32)
XTDLIB__BITCOUNT(64)
#undef XTDLIB__BITMASK
#define bitcount(bitcount__x)  XTDLIB__BIT1(bitcount,bitcount__x)

//------------------------------------------------------------------------------

static inline bool
is_power_of_2(
	size_t n
) {
	return (n & (n - 1)) == 0;
}

static inline bool
at_capacity(
	size_t n
) {
	return is_power_of_2(n);
}

static inline bool
not_at_capacity(
	size_t n
) {
	return !is_power_of_2(n);
}

static inline size_t
capacity_of(
	size_t n
) {
	if(not_at_capacity(n)) {
		size_t const m = bitmask(n);
		n = m + !!~m;
	}
	return n;
}

static inline size_t
roundup(
	size_t const x,
	size_t const r
) {
	if(is_power_of_2(r)) {
		return (x + (r - 1)) & ~(r - 1);
	}

	return ((x + (r - 1)) / r) * r;
}

static inline size_t
floor_midpoint(
	size_t x,
	size_t y
) {
	return (x | y) - ((x ^ y) >> 1);
}

static inline size_t
ceil_midpoint(
	size_t x,
	size_t y
) {
	return (x & y) + ((x ^ y) >> 1);
}

//------------------------------------------------------------------------------

static inline void *
tag_pointer(
	void const *p
) {
	return (void *)((uintptr_t)p | (uintptr_t)1);
}

static inline void *
untag_pointer(
	void const *p
) {
	return (void *)((uintptr_t)p & ~(uintptr_t)1);
}

static inline bool
is_tagged_pointer(
	void const *p
) {
	return ((uintptr_t)p & (uintptr_t)1) != 0;
}

static inline bool
is_untagged_pointer(
	void const *p
) {
	return ((uintptr_t)p & (uintptr_t)1) == 0;
}

//------------------------------------------------------------------------------

extern int cmplz(long l, size_t z);

extern int ndigits(unsigned long long n);

//------------------------------------------------------------------------------

extern uint64_t factorial(unsigned n);

extern void     genperm  (uint64_t p, size_t n, size_t i[]);
extern uint8_t  genperm4 (uint8_t p);
extern uint32_t genperm8 (uint32_t p);
extern uint64_t genperm16(uint64_t p);

//------------------------------------------------------------------------------

#endif//ndef HOL_XTDLIB_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XTDLIB_H__IMPLEMENTATION
#undef HOL_XTDLIB_H__IMPLEMENTATION

//------------------------------------------------------------------------------

int
cmplz(
	long   l,
	size_t z
) {
	if((l < 0) || (LONG_MAX < z)) {
		return -1;
	}

	long x = (long)z;
	return (l < x) ? (
		-1
	):(
		(l > x) ? (
			1
		):(
			0
		)
	);
}

int
ndigits(
	unsigned long long n
) {
	int d = 1;

	for(; n > 999999999ul; n /= 1000000000ul, d += 9);
	for(; n > 999u; n /= 1000u, d += 3);
	for(; n > 9u; n /= 10u, d += 1);

	return d;
}

//------------------------------------------------------------------------------

uint64_t
factorial(
	unsigned n
) {
	if(n <= 20) {
		uint64_t f = n;
		for(; n-- > 2; f *= n);
		return f;
	}
	return ~UINT64_C(0);
}

void
genperm(
	uint64_t p,
	size_t   n,
	size_t   i[]
) {
	size_t const z = n-1;
	size_t       y = z;
	if(y > 0) {
		genperm(p / n, z, i);
		size_t const x = z - (p % n);
		for(; y > x; y--) {
			i[y] = i[y-1];
		}
	}
	i[y] = z;
}

#define XTDLIB__GENPERM(XTDLIB__GENPERM__type,XTDLIB__GENPERM__bits,XTDLIB__GENPERM__states) \
\
static inline XTDLIB__GENPERM__type \
genperm##XTDLIB__GENPERM__states##__actual( \
	XTDLIB__GENPERM__type p, \
	size_t                n \
) { \
	XTDLIB__GENPERM__type i = 0; \
	size_t const z = n-1; \
	if(z > 0) { \
		i = genperm##XTDLIB__GENPERM__states##__actual(p / n, z); \
		size_t                const x = z - (p % n); \
		size_t                const b = x * XTDLIB__GENPERM__bits; \
		XTDLIB__GENPERM__type const m = ~(XTDLIB__GENPERM__type)0u << b; \
		i = ((i & m) << XTDLIB__GENPERM__bits) | (z << b) | (i & ~m); \
	} \
	return i; \
} \
\
XTDLIB__GENPERM__type \
genperm##XTDLIB__GENPERM__states( \
	XTDLIB__GENPERM__type p \
) { \
	return genperm##XTDLIB__GENPERM__states##__actual(p, XTDLIB__GENPERM__states); \
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-negative-value"
XTDLIB__GENPERM(uint8_t,2,4)
XTDLIB__GENPERM(uint32_t,3,8)
XTDLIB__GENPERM(uint64_t,4,16)
#pragma GCC diagnostic pop

#undef XTDLIB__GENPERM

//------------------------------------------------------------------------------

#endif//def HOL_XTDLIB_H__IMPLEMENTATION
