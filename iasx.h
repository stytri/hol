#ifndef HOL_IASX_H__INCLUDED
#define HOL_IASX_H__INCLUDED  1
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
#include <hol/gen64.h>

//------------------------------------------------------------------------------

// IASX: Iterated Add Shift Xor
//       a counter-based random number generator

static inline uint64_t iasx64key(uint64_t x) {
	return gen64(x, 2, 2, 2);
}

static inline bool isax64badkey(uint64_t u) {
	return hasrepeathexdigits64(u, 3, 1);
}

// No anomalies in PractRand (full 32TB run)
// Passes BigCrush (+ bytes and bits reversed)
#define IASX64KEY  UINT64_C(2718281828459045235)  // e * 10**18

static inline uint64_t iasx64(uint64_t u, uint64_t k) {
	for(unsigned i = 1; i < 64; i += 9) {
		unsigned j = ((i << 1) ^ i) & 63;  // 3, 30, 53, 36, 47, 50, 25
		u += k;
		u ^= (u << (64 - j));
		u ^= (u >> j);
	}
	return u;
}

static inline uint64_t iasx64rev(uint64_t u, uint64_t k) {
	for(unsigned i = 55; i < 64; i -= 9) {
		unsigned j = ((i << 1) ^ i) & 63;  // 25, 50, 47, 36, 53, 30, 3
		for(unsigned n = j; n < (CHAR_BIT * sizeof(u)); n += n) {
			u ^= u >> n;
		}
		for(unsigned n = 64 - j; n < (CHAR_BIT * sizeof(u)); n += n) {
			u ^= u << n;
		}
		u -= k;
	}
	return u;
}

#define IASX_VECTOR(IASX__Vn) \
static inline void iasx64v##IASX__Vn(uint64_t u[IASX__Vn], uint64_t const k[IASX__Vn]) { \
	for(unsigned i = 1; i < 64; i += 9) { \
		unsigned const j = ((i << 1) ^ i) & 63; \
		unsigned const l = 64 - j; \
		for(unsigned v = 0; v < IASX__Vn; v++) { \
			u[v] += k[v]; \
			u[v] ^= u[v] << l; \
			u[v] ^= u[v] >> j; \
		} \
	} \
	return; \
} \
static inline void iasx64v##IASX__Vn##k1(uint64_t u[IASX__Vn], uint64_t k) { \
	for(unsigned i = 1; i < 64; i += 9) { \
		unsigned const j = ((i << 1) ^ i) & 63; \
		unsigned const l = 64 - j; \
		for(unsigned v = 0; v < IASX__Vn; v++) { \
			u[v] += k; \
			u[v] ^= u[v] << l; \
			u[v] ^= u[v] >> j; \
		} \
	} \
	return; \
} \
static inline void iasx64v##IASX__Vn##c1(uint64_t u[IASX__Vn], uint64_t const k[IASX__Vn], uint64_t c) { \
	unsigned i = 1; \
	unsigned const j = ((i << 1) ^ i) & 63; \
	unsigned const l = 64 - j; \
	for(unsigned v = 0; v < IASX__Vn; v++) { \
		u[v]  = c + k[v]; \
		u[v] ^= u[v] << l; \
		u[v] ^= u[v] >> j; \
	} \
	for(i += 9; i < 64; i += 9) { \
		unsigned const j = ((i << 1) ^ i) & 63; \
		unsigned const l = 64 - j; \
		for(unsigned v = 0; v < IASX__Vn; v++) { \
			u[v] += k[v]; \
			u[v] ^= u[v] << l; \
			u[v] ^= u[v] >> j; \
		} \
	} \
	return; \
}

IASX_VECTOR(2)
IASX_VECTOR(4)
IASX_VECTOR(8)

//------------------------------------------------------------------------------

#endif//ndef HOL_IASX_H__INCLUDED