#ifndef HOL_HASH_H__INCLUDED
#define HOL_HASH_H__INCLUDED 1
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

#include <stdint.h>

//------------------------------------------------------------------------------

extern uint64_t memhash(void const *key, size_t len);

//------------------------------------------------------------------------------

#endif//ndef HOL_HASH_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_HASH_H__IMPLEMENTATION
#undef HOL_HASH_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#define HASH2(HASH2__x)  ((uint64_t)(HASH2__x) * UINT64_C(0x780C7372621BD74D))
#define HASH1(HASH1__x)  ((uint64_t)(HASH1__x) * UINT64_C(0x4D28CB56C33FA539))

uint64_t
memhash(
	void   const *key,
	size_t        len
) {
	uint64_t h = HASH1(len);

	uint8_t const *b = key;
	uint8_t        x[4];

	for(uint8_t const *end = b + (len & ~3); b != end; ) {
		x[0] = *b++;
		x[1] = *b++;
		x[2] = *b++;
		x[3] = *b++;
		h    = HASH2(HASH2(HASH2(HASH2(h) + HASH1(x[0])) + HASH1(x[1])) + HASH1(x[2])) + HASH1(x[3]);
	}

	if(len & 2) {
		x[0] = *b++;
		x[1] = *b++;
		h    = HASH2(HASH2(h) + HASH1(x[0])) + HASH1(x[1]);
	}

	if(len & 1) {
		x[0] = *b;
		h    = HASH2(h) + HASH1(x[0]);
	}

	return h;
}

//------------------------------------------------------------------------------

#endif//def HOL_HASH_H__IMPLEMENTATION
