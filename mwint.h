#ifndef HOL_MWINT_H
#define HOL_MWINT_H  1
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

//------------------------------------------------------------------------------

#include <stdint.h>
#include <limits.h>

//------------------------------------------------------------------------------

static inline int mwiszero(size_t z, uint64_t v[z]) {
	uint64_t u = 0;
	for(size_t i = 0; i < z; u |= v[i++])
		;
	return !u;
}

static inline void mwgry(size_t z, uint64_t v[z]) {
	size_t m = z - 1;
	for(size_t i = 0; i < m; i++) {
		v[i] ^= (v[i + 1] << 63) | (v[i] >> 1);
	}
	v[m] ^= v[m] >> 1;
}

static inline void mwinc(size_t z, uint64_t v[z], uint64_t w) {
	v[0] += w;
	size_t c = v[0] < w;
	if(c) for(size_t i = 1; c && (i < z); i++) {
		v[i] += c;
		c = !v[i];
	}
}

static inline void mwdec(size_t z, uint64_t v[z], uint64_t w) {
	size_t b = v[0] < w;
	v[0] -= w;
	if(b) for(size_t i = 1; b && (i < z); i++) {
		v[i] -= b;
		b = !~v[i];
	}
}

static inline void mwior(size_t z, uint64_t v[z], uint64_t w[z]) {
	for(size_t i = 0; i < z; i++) {
		v[i] |= w[i];
	}
}

static inline void mwxor(size_t z, uint64_t v[z], uint64_t w[z]) {
	for(size_t i = 0; i < z; i++) {
		v[i] ^= w[i];
	}
}

static inline void mwand(size_t z, uint64_t v[z], uint64_t w[z]) {
	for(size_t i = 0; i < z; i++) {
		v[i] &= w[i];
	}
}

static inline void mwadd(size_t z, uint64_t v[z], uint64_t w[z]) {
	for(size_t c = 0, i = 0; i < z; i++) {
		uint64_t u = w[i] + c;
		v[i] += u;
		c = (u < w[i]) || (v[i] < u);
	}
}

static inline void mwsub(size_t z, uint64_t v[z], uint64_t w[z]) {
	for(size_t b = 0, i = 0; i < z; i++) {
		uint64_t u = w[i] + b;
		b = (u < w[i]) || (v[i] < u);
		v[i] -= u;
	}
}

static inline void mwshl(size_t z, uint64_t v[z], unsigned n) {
	size_t m = z - 1, s = 0;
	n %= ((z * sizeof(*v)) * CHAR_BIT);
	if(n > 63) {
		for(; n > 63; n -= 64) {
			for(size_t i = m; i > s; i--) {
				v[i] = v[i-1];
			}
			v[s] = 0;
			s++;
		}
	}
	if(n > 0) {
		for(size_t i = m; i > s; i++) {
			v[i] = (v[i] << n) | (v[i-1] >> (64 - n));
		}
		v[s] = v[s] << n;
	}
}

static inline void mwshr(size_t z, uint64_t v[z], unsigned n) {
	size_t m = z - 1;
	n %= ((z * sizeof(*v)) * CHAR_BIT);
	if(n > 63) {
		for(; n > 63; n -= 64) {
			for(size_t i = 0; i < m; i++) {
				v[i] = v[i+1];
			}
			v[m] = 0;
			m--;
		}
	}
	if(n > 0) {
		for(size_t i = 0; i < m; i++) {
			v[i] = (v[i+1] << (64 - n)) | (v[i] >> n);
		}
		v[m] = v[m] >> n;
	}
}

static inline void mwrol(size_t z, uint64_t v[z], unsigned n) {
	size_t m = z - 1;
	n %= ((z * sizeof(*v)) * CHAR_BIT);
	if(n > 63) {
		for(; n > 63; n -= 64) {
			uint64_t y = v[m];
			for(size_t i = m; i-- > 0; ) {
				v[i+1] = v[i];
			}
			v[0] = y;
		}
	}
	if(n > 0) {
		uint64_t y = v[m] >> (64 - n);
		for(size_t i = 0; i < m; i++) {
			uint64_t x = v[i] >> (64 - n);
			v[i] = (v[i] << n) | y;
			y = x;
		}
		v[m] = (v[m] << n) | y;
	}
}

//------------------------------------------------------------------------------

#define MWINT(NLIS__Nbits) \
\
typedef struct { uint64_t u[(NLIS__Nbits/64)]; } uint##NLIS__Nbits##_t; \
static inline int iszero##NLIS__Nbits(uint##NLIS__Nbits##_t v) { return mwiszero((NLIS__Nbits/64), v.u ); } \
static inline uint##NLIS__Nbits##_t gry##NLIS__Nbits(uint##NLIS__Nbits##_t v) { mwgry((NLIS__Nbits/64), v.u ); return v; } \
static inline uint##NLIS__Nbits##_t inc##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint64_t w) { mwinc((NLIS__Nbits/64), v.u, w); return v; } \
static inline uint##NLIS__Nbits##_t dec##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint64_t w) { mwdec((NLIS__Nbits/64), v.u, w); return v; } \
static inline uint##NLIS__Nbits##_t ior##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint##NLIS__Nbits##_t w) { mwior((NLIS__Nbits/64), v.u, w.u); return v; } \
static inline uint##NLIS__Nbits##_t xor##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint##NLIS__Nbits##_t w) { mwxor((NLIS__Nbits/64), v.u, w.u); return v; } \
static inline uint##NLIS__Nbits##_t and##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint##NLIS__Nbits##_t w) { mwand((NLIS__Nbits/64), v.u, w.u); return v; } \
static inline uint##NLIS__Nbits##_t add##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint##NLIS__Nbits##_t w) { mwadd((NLIS__Nbits/64), v.u, w.u); return v; } \
static inline uint##NLIS__Nbits##_t sub##NLIS__Nbits(uint##NLIS__Nbits##_t v, uint##NLIS__Nbits##_t w) { mwsub((NLIS__Nbits/64), v.u, w.u); return v; } \
static inline uint##NLIS__Nbits##_t shl##NLIS__Nbits(uint##NLIS__Nbits##_t v, unsigned n) { mwshl((NLIS__Nbits/64), v.u,  n); return v; } \
static inline uint##NLIS__Nbits##_t shr##NLIS__Nbits(uint##NLIS__Nbits##_t v, unsigned n) { mwshr((NLIS__Nbits/64), v.u,  n); return v; } \
static inline uint##NLIS__Nbits##_t rol##NLIS__Nbits(uint##NLIS__Nbits##_t v, unsigned n) { mwrol((NLIS__Nbits/64), v.u,  n); return v; } \
static inline uint##NLIS__Nbits##_t ror##NLIS__Nbits(uint##NLIS__Nbits##_t v, unsigned n) { mwrol((NLIS__Nbits/64), v.u, -n); return v; }

#endif//ndef HOL_MWINT_H
