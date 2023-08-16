#ifndef HOL_XTDINT_H__INCLUDED
#define HOL_XTDINT_H__INCLUDED 1
/*
MIT License

Copyright (c) 2023 Tristan Styles

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

static inline int
uucmp(
	uintmax_t ah,
	uintmax_t al,
	uintmax_t bh,
	uintmax_t bl
) {
	if(ah < bh) return -1;
	if(ah > bh) return +1;
	if(al < bl) return -1;
	if(al > bl) return +1;
	return 0;
}

extern uintmax_t uusxl(uintmax_t ah, uintmax_t al, int n, uintmax_t *vh, uintmax_t *vl);
extern uintmax_t uusxr(uintmax_t ah, uintmax_t al, int n, uintmax_t *vh, uintmax_t *vl);

extern void uusil(uintmax_t ah, uintmax_t al, int n, uintmax_t b, uintmax_t *vh, uintmax_t *vl);
extern void uusir(uintmax_t ah, uintmax_t al, int n, uintmax_t b, uintmax_t *vh, uintmax_t *vl);

extern void uushl(uintmax_t ah, uintmax_t al, int n, uintmax_t *vh, uintmax_t *vl);
extern void uushr(uintmax_t ah, uintmax_t al, int n, uintmax_t *vh, uintmax_t *vl);

extern void uurol(uintmax_t ah, uintmax_t al, int n, uintmax_t *vh, uintmax_t *vl);
static inline void
uuror(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	uurol(ah, al, -n, vh, vl);
}

static inline void
uuaddu(
	uintmax_t  ah,
	uintmax_t  al,
	uintmax_t  b,
	uintmax_t *vh,
	uintmax_t *vl
) {
	*vl = al + b;
	*vh = ah + (*vl < b);
}

static inline void
uusubu(
	uintmax_t  ah,
	uintmax_t  al,
	uintmax_t  b,
	uintmax_t *vh,
	uintmax_t *vl
) {
	*vl = al - b;
	*vh = ah - (al < b);
}

extern void      uumulu(uintmax_t ah, uintmax_t al, uintmax_t b, uintmax_t *vh, uintmax_t *vl);
extern void      uumadu(uintmax_t ah, uintmax_t al, uintmax_t b, uintmax_t  c,  uintmax_t *vh, uintmax_t *vl);
extern void      uumsbu(uintmax_t ah, uintmax_t al, uintmax_t b, uintmax_t  c,  uintmax_t *vh, uintmax_t *vl);
extern uintmax_t uudivu(uintmax_t ah, uintmax_t al, uintmax_t b, uintmax_t *vh, uintmax_t *vl);

extern void uufactorial(unsigned n, uintmax_t *vh, uintmax_t *vl);

extern void uugenperm(uintmax_t ph, uintmax_t pl, size_t n, size_t i[]);

extern void strtouus(char const *cs, char **endp, int base, uintmax_t *vh, uintmax_t *vl);

//------------------------------------------------------------------------------

#endif//ndef HOL_XTDINT_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XTDINT_H__IMPLEMENTATION
#undef HOL_XTDINT_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/holib.h>

//------------------------------------------------------------------------------

uintmax_t
uusxl(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % BITS(uintmax_t);
	if(n) {
		*vh = (ah << n) | (al >> (BITS(uintmax_t) - n));
		*vl = (al << n);
		return ah >> (BITS(uintmax_t) - n);
	}
	*vh = ah;
	*vl = al;
	return 0;
}

uintmax_t
uusxr(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % BITS(uintmax_t);
	if(n) {
		*vl = (al >> n) | (ah << (BITS(uintmax_t) - n));
		*vh = (ah >> n);
		return al & ((UINTMAX_C(1) << n) - 1);
	}
	*vh = ah;
	*vl = al;
	return 0;
}

void
uusil(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t  b,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % BITS(uintmax_t);
	if(n) {
		*vh = (ah << n) | (al >> (BITS(uintmax_t) - n));
		*vl = (al << n) | (b & ((UINTMAX_C(1) << n) - 1));
		return;
	}
	*vh = ah;
	*vl = al;
	return;
}

void
uusir(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t  b,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % BITS(uintmax_t);
	if(n) {
		*vl = (al >> n) | (ah << (BITS(uintmax_t) - n));
		*vh = (ah >> n) | (b << (BITS(uintmax_t) - n));
		return;
	}
	*vh = ah;
	*vl = al;
	return;
}

void
uushl(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % (2*BITS(uintmax_t));
	if(n) {
		if(n < BITS(uintmax_t)) {
			*vh = (ah << n) | (al >> (BITS(uintmax_t) - n));
			*vl = (al << n);
			return;
		}
		if(n > BITS(uintmax_t)) {
			*vh = al << (n - BITS(uintmax_t));
			*vl = 0;
			return;
		}
		*vh = al;
		*vl = 0;
		return;
	}
	*vh = ah;
	*vl = al;
	return;
}

void
uushr(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % (2*BITS(uintmax_t));
	if(n) {
		if(n < BITS(uintmax_t)) {
			*vl = (al >> n) | (ah << (BITS(uintmax_t) - n));
			*vh = (ah >> n);
			return;
		}
		if(n > BITS(uintmax_t)) {
			*vl = ah >> (n - BITS(uintmax_t));
			*vh = 0;
			return;
		}
		*vl = ah;
		*vh = 0;
		return;
	}
	*vh = ah;
	*vl = al;
	return;
}

void
uurol(
	uintmax_t  ah,
	uintmax_t  al,
	int        n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	n = (unsigned)n % (2*BITS(uintmax_t));
	if(n) {
		if(n < BITS(uintmax_t)) {
			*vh = (ah << n) | (al >> (BITS(uintmax_t) - n));
			*vl = (al << n) | (ah >> (BITS(uintmax_t) - n));
			return;
		}
		if(n > BITS(uintmax_t)) {
			n  -= BITS(uintmax_t);
			*vh = (al << n) | (ah >> (BITS(uintmax_t) - n));
			*vl = (ah << n) | (al >> (BITS(uintmax_t) - n));
			return;
		}
		*vh = al;
		*vl = ah;
		return;
	}
	*vh = ah;
	*vl = al;
	return;
}

void
uumulu(
	uintmax_t  ah,
	uintmax_t  al,
	uintmax_t  b,
	uintmax_t *vh,
	uintmax_t *vl
) {
	int n = msbit(b);
	int u = msbit(al);
	if((u + n) <= (BITS(uintmax_t)/2)) {
		*vl = al * b;
		*vh = ah * b;
		return;
	}
	if(is_power_of_2(b)) {
		if(b > 1) uushl(ah, al, (int)n - 1, vh, vl);
		else if(b == 0) *vh = *vl = 0;
		return;
	}

	uintmax_t const mask   = (UINTMAX_C(1) << (BITS(uintmax_t)/2)) - 1;
	uintmax_t const al_lsw = al & mask;
	uintmax_t const al_msw = al >> (BITS(uintmax_t)/2);
	uintmax_t const b_lsw  = b & mask;
	uintmax_t const b_msw  = b >> (BITS(uintmax_t)/2);

	uintmax_t const x      = al_lsw * b_lsw;
	uintmax_t const x_lsw  = x & mask;
	uintmax_t const x_msw  = x >> (BITS(uintmax_t)/2);

	uintmax_t const y      = (al_msw * b_lsw) + x_msw;
	uintmax_t const y_lsw  = y & mask;
	uintmax_t const y_msw  = y >> (BITS(uintmax_t)/2);

	uintmax_t const z      = (al_lsw * b_msw) + y_lsw;
	uintmax_t const z_lsw  = z << (BITS(uintmax_t)/2);
	uintmax_t const z_msw  = z >> (BITS(uintmax_t)/2);

	*vl = z_lsw | x_lsw;
	*vh = (ah * b) + (al_msw * b_msw) + y_msw + z_msw;
}

void
uumadu(
	uintmax_t  ah,
	uintmax_t  al,
	uintmax_t  b,
	uintmax_t  c,
	uintmax_t *vh,
	uintmax_t *vl
) {
	uumulu(ah, al, b, vh, vl);
	*vh += (*vl += c) < c;
}

void
uumsbu(
	uintmax_t  ah,
	uintmax_t  al,
	uintmax_t  b,
	uintmax_t  c,
	uintmax_t *vh,
	uintmax_t *vl
) {
	uumulu(ah, al, b, vh, vl);
	*vh -= *vl < c;
	*vl -= c;
}

uintmax_t
uudivu(
	uintmax_t  ah,
	uintmax_t  al,
	uintmax_t  b,
	uintmax_t *vh,
	uintmax_t *vl
) {
	if(is_power_of_2(b)) {
		if(b >  1) {
			int n = msbit(b);
			return uusxr(ah, al, (int)n - 1, vh, vl);
		}
		if(b == 0) *vh = *vl = ~UINTMAX_C(0);
		else       *vh = ah, *vl = al;
		return 0;
	}

	*vh = ah / b;
	ah %= b;
	if(ah) {
		int       u = lzcount(ah) - lzcount(b);
		int       n = !u + u;
		uintmax_t m = UINTMAX_C(1) << (BITS(uintmax_t) - n);
		uintmax_t l = b << (BITS(uintmax_t) - n);
		uintmax_t x = 0;
		for(b >>= n; b && m; m >>= 1) {
			if((b < ah) || ((b == ah) && (l <= al))) {
				ah -= b + (l > al);
				al -= l;
				x  |= m;
			}
			l = ((b & 1) << (BITS(uintmax_t)-1)) | (l >> 1);
			b >>= 1;
		}
		for(; ah && m; m >>= 1) {
			if(ah) {
				ah -= l > al;
				al -= l;
				x  |= m;
			}
			l >>= 1;
		}
		for(; m; m >>= 1) {
			if(l <= al) {
				al -= l;
				x  |= m;
			}
			l >>= 1;
		}
		*vl = x;
		return al;
	}
	*vl = al / b;
	return al % b;
}

void
uufactorial(
	unsigned   n,
	uintmax_t *vh,
	uintmax_t *vl
) {
	// d = (2*BITS(uintmax_t)) * log(2)
	unsigned const d = (((2*BITS(uintmax_t)) * 3) / 10)
	+                  (((2*BITS(uintmax_t)) * 1) / 1000)
	+                  (((2*BITS(uintmax_t)) * 3) / 100000)
	;
	if(n <= d) {
		uintmax_t ah = 0, al = n;
		while(n-- > 2) {
			uumulu(ah, al, n, &ah, &al);
		}
		*vh = ah;
		*vl = al;
		return;
	}
	*vh = *vl = ~UINTMAX_C(0);
}

void
uugenperm(
	uintmax_t ph,
	uintmax_t pl,
	size_t    n,
	size_t    i[]
) {
	size_t const z = n-1;
	size_t       y = z;
	if(y > 0) {
		size_t const r = uudivu(ph, pl, n, &ph, &pl);
		uugenperm(ph, pl, z, i);
		size_t const x = z - r;
		for(; y > x; y--) {
			i[y] = i[y-1];
		}
	}
	i[y] = z;
}

//------------------------------------------------------------------------------

#define HOL_XTRING_H__STRTO_HELPER_FUNCTIONS
#include <hol/xtring.h>

void
strtouus(
	char const *cs,
	char      **endp,
	int         base,
	uintmax_t  *vh,
	uintmax_t  *vl
) {

#	ifndef HOL_XTRING_H__STRTO__SEPARATOR
#		define HOL_XTRING_H__STRTO__SEPARATOR  '\''
#	endif

	uintmax_t xh   = 0, xl = 0;
	int       cmpl = 0;
	int       neg  = 0;
	cs             = strto__pre(cs, &base, &cmpl, &neg);
	if(neg) {
		for(int c; (c = *cs) != '\0'; cs++) {
			if(c != HOL_XTRING_H__STRTO__SEPARATOR) {
				int const d = strto__digit(c, base);
				if(d < 0) break;

				uumsbu(xh, xl, base, d, &xh, &xl);
			}
		}
	} else {
		for(int c; (c = *cs) != '\0'; cs++) {
			if(c != HOL_XTRING_H__STRTO__SEPARATOR) {
				int const d = strto__digit(c, base);
				if(d < 0) break;

				uumadu(xh, xl, base, d, &xh, &xl);
			}
		}
	}
	int scale = 1000, n = 0;
	cs        = strto__post(cs, &scale, &n);
	switch(n) {
	case 24: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case 21: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case 18: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case 15: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case 12: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case  9: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case  6: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	case  3: uumulu(xh, xl, scale, &xh, &xl); nobreak;
	default: break;
	}
	if(*cs == '!') {
		cs++;
		if(!xh) uufactorial(xl, &xh, &xl);
		else    xh = xl = ~UINTMAX_C(0);
	}
	if(endp) *endp = (char *)cs;
	if(cmpl) xh = ~xh, xl = ~xl;
	*vh = xh, *vl = xl;
	return;
}

//------------------------------------------------------------------------------

#endif//def HOL_XTDINT_H__IMPLEMENTATION
