#ifndef HOL_LEBE_H__INCLUDED
#define HOL_LEBE_H__INCLUDED 1
/*
MIT License

Copyright (c) 2024 Tristan Styles

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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

static inline uint8_t swapbytes8(uint8_t u) {
	return u;
}

static inline uint16_t swapbytes16(uint16_t u) {
	return ((uint16_t)swapbytes8((uint8_t)u) << 8) | swapbytes8((uint8_t)(u >> 8));
}

static inline uint32_t swapbytes32(uint32_t u) {
	return ((uint32_t)swapbytes16((uint16_t)u) << 16) | swapbytes16((uint16_t)(u >> 16));
}

static inline uint64_t swapbytes64(uint64_t u) {
	return ((uint64_t)swapbytes32((uint32_t)u) << 32) | swapbytes32((uint32_t)(u >> 32));
}

#define swapbytes(swapbytes__x)  _Generic((swapbytes__x), \
	uint8_t : swapbytes8 ((uint8_t )(swapbytes__x)), \
	uint16_t: swapbytes16((uint16_t)(swapbytes__x)), \
	uint32_t: swapbytes32((uint32_t)(swapbytes__x)), \
	uint64_t: swapbytes64((uint64_t)(swapbytes__x)), \
	int8_t  : swapbytes8 ((uint8_t )(swapbytes__x)), \
	int16_t : swapbytes16((uint16_t)(swapbytes__x)), \
	int32_t : swapbytes32((uint32_t)(swapbytes__x)), \
	int64_t : swapbytes64((uint64_t)(swapbytes__x))  \
)

static inline size_t swapbytesz(size_t u) {
	return swapbytes(u);
}

//------------------------------------------------------------------------------

#ifndef LEBE_BIG_ENDIAN_BYTE_ORDER
#	if defined(__BIG__ENDIAN__) && !defined(__LITTLE__ENDIAN__)
#		define LEBE_BIG_ENDIAN_BYTE_ORDER  (1)
#	elif defined(__GNUC__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#		define LEBE_BIG_ENDIAN_BYTE_ORDER  (1)
#	elif defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__)
#		define LEBE_BIG_ENDIAN_BYTE_ORDER  (1)
#	elif defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#		define LEBE_BIG_ENDIAN_BYTE_ORDER  (1)
#	elif defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#		define LEBE_BIG_ENDIAN_BYTE_ORDER  (1)
#	else
#		define LEBE_BIG_ENDIAN_BYTE_ORDER  (0)
#	endif
#endif
#if LEBE_BIG_ENDIAN_BYTE_ORDER
#	define beswapbytes(beswapbytes__x)  (beswapbytes__x)
#	define leswapbytes(leswapbytes__x)  swapbytes(leswapbytes__x)
#else
#	define beswapbytes(beswapbytes__x)  swapbytes(beswapbytes__x)
#	define leswapbytes(leswapbytes__x)  (leswapbytes__x)
#endif

//------------------------------------------------------------------------------

static inline uint8_t getu8le(void const *p) {
	return leswapbytes(*(uint8_t const *)p);
}
static inline void ldu8le(void const *p, void *q) {
	*(uint8_t *)q = getu8le(p);
}

static inline uint16_t getu16le(void const *p) {
	return leswapbytes(*(uint16_t const *)p);
}
static inline void ldu16le(void const *p, void *q) {
	*(uint16_t *)q = getu16le(p);
}

static inline uint32_t getu32le(void const *p) {
	return leswapbytes(*(uint32_t const *)p);
}
static inline void ldu32le(void const *p, void *q) {
	*(uint32_t *)q = getu32le(p);
}

static inline uint64_t getu64le(void const *p) {
	return leswapbytes(*(uint64_t const *)p);
}
static inline void ldu64le(void const *p, void *q) {
	*(uint64_t *)q = getu64le(p);
}

static inline size_t getzle(void const *p) {
	return leswapbytes(*(uint64_t const *)p);
}
static inline void ldzle(void const *p, void *q) {
	*(size_t *)q = getzle(p);
}

static inline void setu8le(uint8_t u, void *p) {
	*(uint8_t *)p = leswapbytes(u);
}
static inline void stu8le(void const *q, void *p) {
	setu8le(*(uint8_t *)q, p);
}

static inline void setu16le(uint16_t u, void *p) {
	*(uint16_t *)p = leswapbytes(u);
}
static inline void stu16le(void const *q, void *p) {
	setu16le(*(uint16_t *)q, p);
}

static inline void setu32le(uint32_t u, void *p) {
	*(uint32_t *)p = leswapbytes(u);
}
static inline void stu32le(void const *q, void *p) {
	setu32le(*(uint32_t *)q, p);
}

static inline void setu64le(uint64_t u, void *p) {
	*(uint64_t *)p = leswapbytes(u);
}
static inline void stu64le(void const *q, void *p) {
	setu64le(*(uint64_t *)q, p);
}

static inline void setzle(size_t u, void *p) {
	*(uint64_t *)p = leswapbytes(u);
}
static inline void stuzle(void const *q, void *p) {
	setzle(*(size_t *)q, p);
}

//------------------------------------------------------------------------------

static inline uint8_t getu8be(void const *p) {
	return beswapbytes(*(uint8_t const *)p);
}
static inline void ldu8be(void const *p, void *q) {
	*(uint8_t *)q = getu8be(p);
}

static inline uint16_t getu16be(void const *p) {
	return beswapbytes(*(uint16_t const *)p);
}
static inline void ldu16be(void const *p, void *q) {
	*(uint16_t *)q = getu16be(p);
}

static inline uint32_t getu32be(void const *p) {
	return beswapbytes(*(uint32_t const *)p);
}
static inline void ldu32be(void const *p, void *q) {
	*(uint32_t *)q = getu32be(p);
}

static inline uint64_t getu64be(void const *p) {
	return beswapbytes(*(uint64_t const *)p);
}
static inline void ldu64be(void const *p, void *q) {
	*(uint64_t *)q = getu64be(p);
}

static inline size_t getzbe(void const *p) {
	return beswapbytes(*(uint64_t const *)p);
}
static inline void ldzbe(void const *p, void *q) {
	*(size_t *)q = getzbe(p);
}

static inline void setu8be(uint8_t u, void *p) {
	*(uint8_t *)p = beswapbytes(u);
}
static inline void stu8be(void const *q, void *p) {
	setu8be(*(uint8_t *)q, p);
}

static inline void setu16be(uint16_t u, void *p) {
	*(uint16_t *)p = beswapbytes(u);
}
static inline void stu16be(void const *q, void *p) {
	setu16be(*(uint16_t *)q, p);
}

static inline void setu32be(uint32_t u, void *p) {
	*(uint32_t *)p = beswapbytes(u);
}
static inline void stu32be(void const *q, void *p) {
	setu32be(*(uint32_t *)q, p);
}

static inline void setu64be(uint64_t u, void *p) {
	*(uint64_t *)p = beswapbytes(u);
}
static inline void stu64be(void const *q, void *p) {
	setu64be(*(uint64_t *)q, p);
}

static inline void setzbe(size_t u, void *p) {
	*(uint64_t *)p = beswapbytes(u);
}
static inline void stzbe(void const *q, void *p) {
	setzbe(*(size_t *)q, p);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif//ndef HOL_LEBE_H__INCLUDED
