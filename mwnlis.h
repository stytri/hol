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
uint##NLIS__Nbits##_t \
nlis##NLIS__Nbits( \
	uint##NLIS__Nbits##_t x, \
	uint##NLIS__Nbits##_t k, \
	unsigned              n \
) { \
	mwsub(MWINT_WORDS(NLIS__Nbits), x.u, k.u); \
	while(n-- > 0) { \
		unsigned i = 1; \
		do { \
			mwgry(MWINT_WORDS(NLIS__Nbits), x.u); \
			mwrol(MWINT_WORDS(NLIS__Nbits), x.u, i); \
			i <<= 1; \
		} while(i < NLIS__Nbits) \
			; \
		mwgry(MWINT_WORDS(NLIS__Nbits), x.u); \
		do { \
			i >>= 1; \
			mwror(MWINT_WORDS(NLIS__Nbits), x.u, i); \
			mwgry(MWINT_WORDS(NLIS__Nbits), x.u); \
		} while(i != 1) \
			; \
		mwadd(MWINT_WORDS(NLIS__Nbits), x.u, k.u); \
	} \
	return x; \
} \
\
static inline uint##NLIS__Nbits##_t \
ulis##NLIS__Nbits##__g( \
	uint##NLIS__Nbits##_t x \
) { \
	for(unsigned n = (NLIS__Nbits / 2); n != 0; n >>= 1) { \
		mwxor(MWINT_WORDS(NLIS__Nbits), x.u, shr##NLIS__Nbits(x, n).u); \
	} \
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
		mwsub(MWINT_WORDS(NLIS__Nbits), x.u, k.u); \
		unsigned i = 1; \
		do { \
			x = ulis##NLIS__Nbits##__g(x); \
			mwrol(MWINT_WORDS(NLIS__Nbits), x.u, i); \
			i <<= 1; \
		} while(i < NLIS__Nbits) \
			; \
		x = ulis##NLIS__Nbits##__g(x); \
		do { \
			i >>= 1; \
			mwror(MWINT_WORDS(NLIS__Nbits), x.u, i); \
			x = ulis##NLIS__Nbits##__g(x); \
		} while(i != 1) \
			; \
	} \
	mwadd(MWINT_WORDS(NLIS__Nbits), x.u, k.u); \
	return x; \
}

//------------------------------------------------------------------------------

#endif//ndef HOL_MWNLIS_H
