#ifndef HOL_BASE64_H__INCLUDED
#define HOL_BASE64_H__INCLUDED  1
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

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

//------------------------------------------------------------------------------

static inline size_t
base64__enclen(
	size_t z,
	size_t n
) {
	z *= CHAR_BIT;
	n *= z;
	return (n + 5) / 6;
}
static inline size_t base64enclen8 (size_t n) { return base64__enclen(sizeof(uint8_t), n); }
static inline size_t base64enclen16(size_t n) { return base64__enclen(sizeof(uint16_t), n); }
static inline size_t base64enclen32(size_t n) { return base64__enclen(sizeof(uint32_t), n); }
static inline size_t base64enclen64(size_t n) { return base64__enclen(sizeof(uint64_t), n); }
static inline size_t base64enclen  (size_t n) { return base64__enclen(sizeof(char), n); }

static inline size_t
base64__declen(
	size_t z,
	size_t n
) {
	z *= CHAR_BIT;
	n *= 6;
	return (n + (z - 1)) / z;
}
static inline size_t base64declen8 (size_t n) { return base64__declen(sizeof(uint8_t), n); }
static inline size_t base64declen16(size_t n) { return base64__declen(sizeof(uint16_t), n); }
static inline size_t base64declen32(size_t n) { return base64__declen(sizeof(uint32_t), n); }
static inline size_t base64declen64(size_t n) { return base64__declen(sizeof(uint64_t), n); }
static inline size_t base64declen  (size_t n) { return base64__declen(sizeof(char), n); }

extern int
base64__enchar(
	int c
);
extern int
base64__dechar(
	int c
);
extern int
base64__encode(
	size_t      z,
	size_t      n,
	void const *k,
	char       *cs
);
static inline int base64encode8(size_t n, uint8_t const k[n], char *cs) {
	return base64__encode(sizeof(uint8_t), n, k, cs);
}
static inline int base64encode16(size_t n, uint16_t const k[n], char *cs) {
	return base64__encode(sizeof(uint16_t), n, k, cs);
}
static inline int base64encode32(size_t n, uint32_t const k[n], char *cs) {
	return base64__encode(sizeof(uint32_t), n, k, cs);
}
static inline int base64encode64(size_t n, uint64_t const k[n], char *cs) {
	return base64__encode(sizeof(uint64_t), n, k, cs);
}
static inline int base64encode(size_t n, char const k[n], char *cs) {
	return base64__encode(sizeof(char), n, k, cs);
}

extern int
base64__decode(
	size_t      z,
	size_t      n,
	void       *k,
	char const *cs,
	int         wrap
);
static inline int base64decode8(size_t n, uint8_t k[n], char const *cs, int wrap) {
	return base64__decode(sizeof(uint8_t), n, k, cs, wrap);
}
static inline int base64decode16(size_t n, uint16_t k[n], char const *cs, int wrap) {
	return base64__decode(sizeof(uint16_t), n, k, cs, wrap);
}
static inline int base64decode32(size_t n, uint32_t k[n], char const *cs, int wrap) {
	return base64__decode(sizeof(uint32_t), n, k, cs, wrap);
}
static inline int base64decode64(size_t n, uint64_t k[n], char const *cs, int wrap) {
	return base64__decode(sizeof(uint64_t), n, k, cs, wrap);
}
static inline int base64decode(size_t n, char k[n], char const *cs, int wrap) {
	return base64__decode(sizeof(char), n, k, cs, wrap);
}

//------------------------------------------------------------------------------

#endif//ndef HOL_BASE64_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_BASE64_H__IMPLEMENTATION

//------------------------------------------------------------------------------

static const char base64__chars[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'+', '/'
};

#if BASE64__ASCII_MAP || BASE64__UTF8_MAP
static int    const base64__minchar = 43;
static int    const base64__maxchar = 122;
static size_t const base64__mapsize = 80;
static int    const base64__map[80] = {
	62,     -1,     -1,     -1,     63,     52,     53,     54,
	55,     56,     57,     58,     59,     60,     61,     -1,
	-1,     -1,     -1,     -1,     -1,     -1,      0,      1,
	 2,      3,      4,      5,      6,      7,      8,      9,
	10,     11,     12,     13,     14,     15,     16,     17,
	18,     19,     20,     21,     22,     23,     24,     25,
	-1,     -1,     -1,     -1,     -1,     -1,     26,     27,
	28,     29,     30,     31,     32,     33,     34,     35,
	36,     37,     38,     39,     40,     41,     42,     43,
	44,     45,     46,     47,     48,     49,     50,     51,
};
#else
static int base64__minchar = INT_MAX;
static int base64__maxchar = INT_MIN;

static size_t base64__mapsize = 0;
static int   *base64__map     = NULL;

static int
base64__initialize(
	void
) {
	for(size_t i = 0; i < 64; i++) {
		if(base64__minchar > base64__chars[i]) base64__minchar = base64__chars[i];
		if(base64__maxchar < base64__chars[i]) base64__maxchar = base64__chars[i];
	}
	size_t n = (base64__maxchar - base64__minchar) + 1;
	base64__map = calloc(n, sizeof(*base64__map));
	if(base64__map) {
		base64__mapsize = n;
		for(size_t i = 0; i < n; i++) {
			base64__map[i] = -1;
		}
		for(size_t i = 0; i < 64; i++) {
			base64__map[base64__chars[i] - base64__minchar] = i;
		}
		return 0;
	}
	return -1;
}
#endif
int
base64__enchar(
	int c
) {
	return base64__chars[c];
}

int
base64__dechar(
	int c
) {
	if((base64__minchar <= c) && (c <= base64__maxchar)) {
		return base64__map[c - base64__minchar];
	}
	return -1;
}

static inline uint64_t base64__mask64(int n) { return ~(~UINT64_C(0) << n); }

int
base64__encode(
	size_t      z,
	size_t      n,
	void const *k,
	char       *cs
) {
	int r = 0;
#if !(BASE64__ASCII_MAP || BASE64__UTF8_MAP)
	if(base64__mapsize || ((r = base64__initialize()) == 0)) {
#endif
		int  const  zb = z * CHAR_BIT;
		int         b  = 0, c = 0, d = 0;
		uint64_t    u  = 0;
		for(size_t i = 0;; ) {
#		define BASE64__GET(BASE64__GET__u)  do switch(z) { \
			case sizeof(uint8_t ): (BASE64__GET__u) = ((uint8_t  *)k)[i++]; break; \
			case sizeof(uint16_t): (BASE64__GET__u) = ((uint16_t *)k)[i++]; break; \
			case sizeof(uint32_t): (BASE64__GET__u) = ((uint32_t *)k)[i++]; break; \
			case sizeof(uint64_t): (BASE64__GET__u) = ((uint64_t *)k)[i++]; break; \
			} while(0)
			if(b == 0) {
				if(i == n) break;
				BASE64__GET(u);
				b = zb;
			}
			if(d == 0) {
				if(b >= 6) {
					b -= 6;
					c  = (u >> b) & base64__mask64(6);
					*cs++ = base64__enchar(c);
					c = 0;
					continue;
				}
				c = u & base64__mask64(b);
				d = 6 - b;
				b = 0;
				continue;
			}
			if(b >= d) {
				b  -= d;
				c <<= d;
				c  |= (u >> b) & base64__mask64(d);
				d   = 0;
				*cs++ = base64__enchar(c);
				c = 0;
				continue;
			}
			c <<= b;
			c  |= u & base64__mask64(b);
			d  -= b;
			b   = 0;
			continue;
#		undef BASE64__GET
		}
		if(d) *cs++ = base64__enchar(c << d);
#if !(BASE64__ASCII_MAP || BASE64__UTF8_MAP)
	}
#endif
	return r;
}

int
base64__decode(
	size_t      z,
	size_t      n,
	void       *k,
	char const *cs,
	int         wrap
) {
	int r = 0;
#if !(BASE64__ASCII_MAP || BASE64__UTF8_MAP)
	if(base64__mapsize || ((r = base64__initialize()) == 0)) {
#endif
		char const *ct = cs;
		int  const  zb = z * CHAR_BIT;
		int         b  = zb, c, d;
		uint64_t    u  = 0;
		for(size_t i = 0; i < n; ) {
#		define BASE64__SET(BASE64__SET__u)  do switch(z) { \
			case sizeof(uint8_t ): ((uint8_t  *)k)[i++] = (uint8_t )(BASE64__SET__u); break; \
			case sizeof(uint16_t): ((uint16_t *)k)[i++] = (uint16_t)(BASE64__SET__u); break; \
			case sizeof(uint32_t): ((uint32_t *)k)[i++] = (uint32_t)(BASE64__SET__u); break; \
			case sizeof(uint64_t): ((uint64_t *)k)[i++] = (uint64_t)(BASE64__SET__u); break; \
			} while(0)
			if(!*cs) {
				if(!wrap) {
					if(b != zb) {
						BASE64__SET(u);
					}
					while(i < n) {
						BASE64__SET(0);
					}
					break;
				}
				cs = ct;
			}
			c = base64__dechar(*cs++);
			if(c < 0) continue;
			if(b >= 6) {
				u <<= 6;
				u  |= c;
				b  -= 6;
				if(b > 0) continue;
				BASE64__SET(u);
				u = 0;
				b = zb;
				continue;
			}
			d   = 6 - b;
			u <<= b;
			u  |= ((uint64_t)c >> d) & base64__mask64(b);
			BASE64__SET(u);
			u = (uint64_t)c & base64__mask64(d);
			b = zb - d;
#		undef BASE64__SET
		}
#if !(BASE64__ASCII_MAP || BASE64__UTF8_MAP)
	}
#endif
	return r;
}

//------------------------------------------------------------------------------

#undef HOL_BASE64_H__IMPLEMENTATION
#endif//def HOL_BASE64_H__IMPLEMENTATION
