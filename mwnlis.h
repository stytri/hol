#ifndef HOL_MWNLIS_H
#define HOL_MWNLIS_H  1
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

#include <hol/mwint.h>

//------------------------------------------------------------------------------

#define MWNLIS(NLIS__Nbits) \
\
MWINT(NLIS__Nbits) \
\
static inline uint##NLIS__Nbits##_t \
nlis##NLIS__Nbits##__round( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k \
) { \
	unsigned i = 1; \
	do { \
		x = gry##NLIS__Nbits(x); \
		x = rol##NLIS__Nbits(x, i); \
		i <<= 1; \
	} while(i < NLIS__Nbits) \
		; \
	x = gry##NLIS__Nbits(x); \
	do { \
		i >>= 1; \
		x = rol##NLIS__Nbits(x, i); \
		x = gry##NLIS__Nbits(x); \
	} while(i != 1) \
		; \
	return add##NLIS__Nbits(x, k); \
} \
\
uint##NLIS__Nbits##_t \
nlis##NLIS__Nbits( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k, \
	unsigned              n \
) { \
	x = sub##NLIS__Nbits(x, k); \
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
	for(unsigned n = (NLIS__Nbits / 2); n != 0; n >>= 1) { \
		x = xor##NLIS__Nbits(x, shr##NLIS__Nbits(x, n)); \
	} \
	return x; \
} \
\
static inline uint##NLIS__Nbits##_t \
ulis##NLIS__Nbits##__round( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k \
) { \
	x = sub##NLIS__Nbits(x, k); \
	unsigned i = 1; \
	do { \
		x = ulis##NLIS__Nbits##__g(x); \
		x = ror##NLIS__Nbits(x, i); \
		i <<= 1; \
	} while(i < NLIS__Nbits) \
		; \
	x = ulis##NLIS__Nbits##__g(x); \
	do { \
		i >>= 1; \
		x = ror##NLIS__Nbits(x, i); \
		x = ulis##NLIS__Nbits##__g(x); \
	} while(i != 1) \
		; \
	return x; \
} \
\
uint##NLIS__Nbits##_t \
ulis##NLIS__Nbits( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k, \
	unsigned              n \
) { \
	while(n-- > 0) { \
		x = ulis##NLIS__Nbits##__round(x, k); \
	} \
	return add##NLIS__Nbits(x, k); \
}

//------------------------------------------------------------------------------

#endif//ndef HOL_MWNLIS_H
