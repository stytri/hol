#ifndef HOL_GEN64_H__INCLUDED
#define HOL_GEN64_H__INCLUDED  1
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

//------------------------------------------------------------------------------

extern uint64_t gen64(uint64_t k, unsigned xw, int xl, int xt) {

extern void gen64mw(size_t n, uint64_t u[n], uint64_t const k[n], size_t xw, int xl, int xt);

//------------------------------------------------------------------------------

#endif//ndef HOL_GEN64_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_GEN64_H__IMPLEMENTATION

//------------------------------------------------------------------------------

uint64_t gen64(
	uint64_t k,
	unsigned xw,
	int      xl,
	int      xt
) {
	if((xw > 8) || (xl < 0) || (xl > 2) || (xt < -2) || (xt > 2)) {
		return 0;
	}

	uint64_t r = 0;

	uint64_t excludedigits = 0;
	uint64_t digits;
	uint64_t e;
	uint64_t c;
	size_t   x = 0;
	size_t   i;
	size_t   n;
	unsigned d = 0;

	if(xl) {
		n = 0;
		digits = 0;
		for(c = 1; c < (16 - (xl > 1)); c++) {
			digits = (digits << 4) | c;
			n++;
		}
		i = (k % n) * 4;
		k /= n;
		c = (digits >> i) & 0xf;
		if(x < xw) x++;
		excludedigits = (excludedigits << 4) | c;
		r = c;
		d++;
	}
	for(unsigned t = 16 - (xt != 0); d < t; d++) {
		n = 0;
		digits = 0;
		for(c = 0; c < 16; c++) {
			e = excludedigits;
			for(i = 0; i < x; i++) {
				if(c == (e & 0xf)) goto skip1;
				e >>= 4;
			}
			digits = (digits << 4) | c;
			n++;
		skip1:;
		}
		i = (k % n) * 4;
		k /= n;
		c = (digits >> i) & 0xf;
		if(x < xw) x++;
		excludedigits = (excludedigits << 4) | c;
		r = (r << 4) | c;
	}
	if(xt) {
		n = 0;
		digits = 0;
		if(xt < 0) {
			for(c = 1; c < (16 - (xt < -1)); c += 2) {
				e = excludedigits;
				for(i = 0; i < x; i++) {
					if(c == (e & 0xf)) goto skip2;
					e >>= 4;
				}
				digits = (digits << 4) | c;
				n++;
			skip2:;
			}
		} else {
			for(c = 1; c < (16 - (xt > 1)); c++) {
				e = excludedigits;
				for(i = 0; i < x; i++) {
					if(c == (e & 0xf)) goto skip3;
					e >>= 4;
				}
				digits = (digits << 4) | c;
				n++;
			skip3:;
			}
		}
		i = (k % n) * 4;
		c = (digits >> i) & 0xf;
		r = (r << 4) | c;
	}

	return r;
}

void gen64mw(
	size_t         n,
	uint64_t       u[n],
	uint64_t const k[n],
	size_t         xw,
	int            xl,
	int            xt
) {
	if(n > 1) {
		size_t i = 0, m = n - 1;
		u[i] = gen64(k[i], xw, xl, 0);
		for(i++; i < m; i++){
			u[i] = gen64(k[i], xw, 0, 0);
		}
		u[i] = gen64(k[i], xw, 0, xt);
		return;
	}
	if(n > 0) {
		u[0] = gen64(k[0], xw, xl, xt);
	}
	return;
}

//------------------------------------------------------------------------------

#undef HOL_GEN64_H__IMPLEMENTATION
#endif//def HOL_GEN64_H__IMPLEMENTATION
