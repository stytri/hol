#ifndef HOL_XTRING_H__INCLUDED
#define HOL_XTRING_H__INCLUDED 1
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

//------------------------------------------------------------------------------

static inline bool
streq(
	char const *cs,
	char const *ct
) {
	return strcmp(cs, ct) == 0;
}

static inline bool
streqn(
	char const *cs,
	char const *ct,
	size_t      n
) {
	return strncmp(cs, ct, n) == 0;
}

static inline bool
memeq(
	char const *cs,
	char const *ct,
	size_t      n
) {
	return memcmp(cs, ct, n) == 0;
}

//------------------------------------------------------------------------------

extern uintmax_t          strtoumaxs(char const *cs, char **endp, int base);
extern unsigned long long strtoulls (char const *cs, char **endp, int base);
extern unsigned long      strtouls  (char const *cs, char **endp, int base);
extern unsigned int       strtous   (char const *cs, char **endp, int base);
extern intmax_t           strtoimaxs(char const *cs, char **endp, int base);
extern long long          strtolls  (char const *cs, char **endp, int base);
extern long               strtols   (char const *cs, char **endp, int base);
extern int                strtois   (char const *cs, char **endp, int base);

static inline size_t
strtozs(
	char const *cs,
	char      **endp,
	int         base
) {
	return (sizeof(size_t) > sizeof(unsigned long)) ? (
		strtoulls(cs, endp, base)
	):(
		strtouls(cs, endp, base)
	);
}

static inline size_t
strtoz(
	char const *cs,
	char      **endp,
	int         base
) {
	return (sizeof(size_t) > sizeof(unsigned long)) ? (
		strtoull(cs, endp, base)
	):(
		strtoul(cs, endp, base)
	);
}

extern void strtomem(char const *cs, char **endp, int fill, size_t n, void *v);

extern uintmax_t streval(char const *cs, char **endp, int base);

//------------------------------------------------------------------------------

extern void *memfill(void *t, size_t n, void const *s, size_t m);

//------------------------------------------------------------------------------

extern void *mcopy   (void const *t, size_t n);
extern void *mconcat (void const *t, size_t n, void const *s, size_t m);
extern void *mappend (void       *t, size_t n, void const *s, size_t m);
extern void *mprepend(void       *t, size_t n, void const *s, size_t m);
extern void *minsert (void       *t, size_t n, void const *s, size_t m, size_t x);
extern int   mcompare(void const *t, size_t n, void const *s, size_t m);
extern void *mprintf (size_t *np, char const *f, ...);

//------------------------------------------------------------------------------

#endif//ndef HOL_XTRING_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XTRING_H__IMPLEMENTATION
#undef HOL_XTRING_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/nobreak.h>
#include <stdarg.h>

//------------------------------------------------------------------------------

#define HOL_XTRING_H__STRTO_HELPER_FUNCTIONS
#include <hol/xtring.h>

uintmax_t
strtoumaxs(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  uintmax_t
#	include <hol/xtring.h>
}

unsigned long long
strtoulls(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  unsigned long long
#	include <hol/xtring.h>
}

unsigned long
strtouls(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  unsigned long
#	include <hol/xtring.h>
}

unsigned int
strtous(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  unsigned int
#	include <hol/xtring.h>
}

intmax_t
strtoimaxs(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  intmax_t
#	include <hol/xtring.h>
}

long long
strtolls(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  long long
#	include <hol/xtring.h>
}

long
strtols(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  long
#	include <hol/xtring.h>
}

int
strtois(
	char const *cs,
	char      **endp,
	int         base
) {
#	define HOL_XTRING_H__STRTO_TYPE  int
#	include <hol/xtring.h>
}

//------------------------------------------------------------------------------

void
strtomem(
	char const *cs,
	char      **endp,
	int         fill,
	size_t      n,
	void       *v
) {
	char const *ct = cs;
	int         q  = 1;
	char       *t;
	for(uint8_t *b = v, *e = b + n; b != e; q = 0) {
		int ec = *ct, cc;
		while(ec && !isgraph(ec)) {
			ec = *++ct;
		}
		if((ec == '\'') || (ec == '"')) {
			for(q = 0, cc = *++ct; (b != e) && cc && (q || (cc != ec)); cc = *++ct) {
				if(q == 0) {
					if(cc != '\\') {
						*b++ = cc;
					} else {
						q = 1;
					}
				} else if(q > 0) {
					switch(cc) {
					default : *b++ = cc;   break;
					case 'a': *b++ = '\a'; break;
					case 'b': *b++ = '\b'; break;
					case 'f': *b++ = '\f'; break;
					case 'n': *b++ = '\n'; break;
					case 'r': *b++ = '\r'; break;
					case 't': *b++ = '\t'; break;
					case 'v': *b++ = '\v'; break;
					case '0': *b++ = '\0'; break;
					case '^':  q   = -1;   break;
					}
					q = (q > 0) ? 0 : q;
				} else {
					*b++ = cc & 0x1f;
					q = 0;
				}
			}
			if(*ct) ct++;
			continue;
		}
		while(ec && !isdigit(ec)) {
			ec = *++ct;
		}
		if(ec) {
			uintmax_t u = strtoumax(ct, &t, 0);
			if(ct != t) {
				ct = t;
				int j = BITS(u) - 8;
				for(; (j > 0) && (((u >> j) & 0xffu) == 0); j -= 8)
					;
				for(; (b != e) && (j > 8); j -= 8) {
					*b++ = (u >> j) & 0xffu;
				}
				if(b != e) {
					*b++ = u & 0xffu;
				}
				continue;
			}
		}
		if(endp) *endp = (char *)ct;
		size_t r = e - b;
		if(r > 0) {
			if(fill < 0) memfill(v, n, v, n - r);
			else memset(b, fill, r);
		}
		break;
	}
	return;
}

//------------------------------------------------------------------------------

static uintmax_t
streval__subeval(
	char const         *cs,
	char              **endp,
	int                 base
) {
	for(; isspace(*cs); cs++);
	char     *s = (char *)cs;
	uintmax_t v = (*cs == '(') ? (
		streval__subeval(s + 1, &s, base)
	):(
		strtoumaxs(s, &s, base)
	);
	for(int c; (c = *s); ) {
		if(isspace(c)) {
			s++;
			continue;
		}
		if(c == ')') {
			s++;
			break;
		}
		size_t    n = strcspn(cs = s, "(0123456789)");
		uintmax_t u = (s[n] == '(') ? (
			streval__subeval(s + n + 1, &s, base)
		):(
			strtoumaxs(s + n, &s, base)
		);
		while(n > 1) {
			if(cs[n - 1] == '-') {
				u = -u;
				n--;
				continue;
			}
			if(cs[n - 1] == '~') {
				u = ~u;
				n--;
				continue;
			}
			break;
		}
		switch(c) {
		default:
			*endp = (char *)cs;
			return v;
#		define streval__OP1(OP1__leme,OP1__op)  \
		case OP1__leme: \
			v = v OP1__op u; \
			break
#		define streval__OP2(OP2__leme,OP2__op)  \
		case OP2__leme: \
			if(n > 1) { \
				if(cs[1] == OP2__leme) { \
					v = v CONCAT(OP2__op,OP2__op) u; \
					break; \
				} \
			} \
			break
		streval__OP1('^', ^);
		streval__OP1('|', |);
		streval__OP1('&', &);
		streval__OP1('+', +);
		streval__OP1('-', -);
		streval__OP1('*', *);
		streval__OP1('/', /);
		streval__OP1('%', %);
		streval__OP2('<', <);
		streval__OP2('>', >);
#		undef streval__OP2
#		undef streval__OP1
		}
	}
	*endp = s;
	return v;
}

uintmax_t
streval(
	char const *cs,
	char      **endp,
	int         base
) {
	char     *s = (char *)cs;
	uintmax_t v = streval__subeval(cs, &s, base);
	if(endp) *endp = s;
	return v;
}


//------------------------------------------------------------------------------

void *
memfill(
	void       *t,
	size_t      n,
	void const *cs,
	size_t      m
) {
	char *s = t;
	if(m > n) m = n;
	if(t != cs) {
		memcpy(s, cs, m);
	}
	for(s += m, n -= m; m < n; s += m, n -= m, m += m) {
		memcpy(s, t, m);
	}
	if(n > 0) {
		memcpy(s, t, n);
	}
	return t;
}

//------------------------------------------------------------------------------

void *
mcopy(
	void   const *t,
	size_t const  n
) {
	void *p = malloc(n);
	if(likely(p)) {
		memcpy(p, t, n);
	}
	return p;
}

void *
mconcat(
	void   const *t,
	size_t const  n,
	void   const *s,
	size_t const  m
) {
	void *p = malloc(n + m);
	if(likely(p)) {
		memcpy(p, t, n);
		memcpy((char *)p + n, s, m);
	}
	return p;
}

void *
mappend(
	void         *t,
	size_t const  n,
	void   const *s,
	size_t const  m
) {
	void *p = realloc(t, n + m);
	if(likely(p)) {
		memcpy((char *)p + n, s, m);
	}
	return p;
}

void *
mprepend(
	void         *t,
	size_t const  n,
	void   const *s,
	size_t const  m
) {
	void *p = realloc(t, n + m);
	if(likely(p)) {
		memmove((char *)p + m, p, n);
		memcpy(p, s, m);
	}
	return p;
}

void *
minsert(
	void         *t,
	size_t const  n,
	void   const *s,
	size_t const  m,
	size_t        x
) {
	void *p = realloc(t, n + m);
	if(likely(p)) {
		memmove((char *)p + x + m, (char *)p + x, n - x);
		memcpy((char *)p + x, s, m);
	}
	return p;
}

int
mcompare(
	void   const *t,
	size_t const  n,
	void   const *s,
	size_t const  m
) {
	int    const r = (n < m) ? -1 : ((n > m) ? 1 : 0);
	size_t const u = (n < m) ? n : m;
	int    const q = memcmp(t, s, u);
	return (q != 0) ? q : r;
}

void *
mprintf(
	size_t       *np,
	char   const *f,
	...
) {
	va_list va;
	va_start(va, f);
	size_t n = vsnprintf(NULL, 0, f, va);
	va_end(va);
	void *p = malloc(n+1);
	if(likely(p)) {
		va_start(va, f);
		n = vsnprintf(p, n+1, f, va);
		va_end(va);
		*np = n;
	}
	return p;
}

//------------------------------------------------------------------------------

#endif//def HOL_XTRING_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#ifdef HOL_XTRING_H__STRTO_HELPER_FUNCTIONS
#undef HOL_XTRING_H__STRTO_HELPER_FUNCTIONS
#	ifndef HOL_XTRING_H__STRTO_HELPER_FUNCTIONS__IMPLEMENTATION
#	define HOL_XTRING_H__STRTO_HELPER_FUNCTIONS__IMPLEMENTATION  1

//------------------------------------------------------------------------------

#include <ctype.h>

//------------------------------------------------------------------------------

static inline int
strto__digit(
	int c,
	int base
) {
	return isdigit(c) ? (
		((c = (c - '0')) < base) ? c : -1
	):(
		isalpha(c) ? (
			((c = (10 + (toupper(c) - 'A'))) < base) ? c : -1
		):(
			-1
		)
	);
}

static inline char const *
strto__pre(
	char const *cs,
	int        *base,
	int        *cmpl,
	int        *neg
) {
	for(; isspace(*cs); cs++)
		;
	if(*cs == '~') {
		*cmpl = 1;
		++cs;
	}
	if(*cs == '-') {
		*neg = 1;
		++cs;
	} else if(*cs == '+') {
		++cs;
	}
	if(!*base) {
		if(*cs == '0') {
			for(; *cs == '0'; cs++)
				;
			switch(*cs) {
			default :           *base = 10; cs--; break;
			case '0': case '1':
			case '2': case '3':
			case '4': case '5':
			case '6': case '7': *base =  8;       break;
			case '8': case '9': *base = 10;       break;
			case 'B': case 'b': *base =  2; cs++; break;
			case 'O': case 'o': *base =  8; cs++; break;
			case 'D': case 'd': *base = 10; cs++; break;
			case 'X': case 'x': *base = 16; cs++; break;
			}
		} else {
			*base = 10;
		}
	}
	return cs;
}

static inline char const *
strto__post(
	char const *cs,
	int        *scale,
	int        *n
) {
	switch(*cs) {
	case 'Y': case 'y': *n += 3; nobreak;
	case 'Z': case 'z': *n += 3; nobreak;
	case 'E': case 'e': *n += 3; nobreak;
	case 'P': case 'p': *n += 3; nobreak;
	case 'T': case 't': *n += 3; nobreak;
	case 'G': case 'g': *n += 3; nobreak;
	case 'M': case 'm': *n += 3; nobreak;
	case 'K': case 'k': *n += 3;
		cs++;
		if((*cs == 'I') || (*cs == 'i')) {
			*scale = 1024;
			cs++;
		}
		break;
	default:
		break;
	}
	return cs;
}

//------------------------------------------------------------------------------

#	endif//ndef HOL_XTRING_H__STRTO_HELPER_FUNCTIONS__IMPLEMENTATION
#endif//def HOL_XTRING_H__STRTO_HELPER_FUNCTIONS

//------------------------------------------------------------------------------

#ifdef HOL_XTRING_H__STRTO_TYPE

//------------------------------------------------------------------------------

#	ifndef HOL_XTRING_H__STRTO__SEPARATOR
#		define HOL_XTRING_H__STRTO__SEPARATOR  '\''
#	endif

	HOL_XTRING_H__STRTO_TYPE x    = 0;
	int                      cmpl = 0;
	int                      neg  = 0;
	cs                            = strto__pre(cs, &base, &cmpl, &neg);
	if(neg) {
		for(int c; (c = *cs) != '\0'; cs++) {
			if(c != HOL_XTRING_H__STRTO__SEPARATOR) {
				int const d = strto__digit(c, base);
				if(d < 0) break;

				x = (x * (HOL_XTRING_H__STRTO_TYPE)base) - (HOL_XTRING_H__STRTO_TYPE)d;
			}
		}
	} else {
		for(int c; (c = *cs) != '\0'; cs++) {
			if(c != HOL_XTRING_H__STRTO__SEPARATOR) {
				int const d = strto__digit(c, base);
				if(d < 0) break;

				x = (x * (HOL_XTRING_H__STRTO_TYPE)base) + (HOL_XTRING_H__STRTO_TYPE)d;
			}
		}
	}
	int scale = 1000, n = 0;
	cs        = strto__post(cs, &scale, &n);
	switch(n) {
	case 24: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case 21: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case 18: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case 15: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case 12: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case  9: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case  6: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	case  3: x *= (HOL_XTRING_H__STRTO_TYPE)scale; nobreak;
	default: break;
	}
	if(*cs == '!') {
		cs++;
		for(HOL_XTRING_H__STRTO_TYPE f = x; f-- > 1; ) {
			x *= f;
		}
	}
	if(endp) *endp = (char *)cs;
	if(cmpl) x = ~x;
	return x;

//------------------------------------------------------------------------------

#undef HOL_XTRING_H__STRTO_TYPE
#endif//def HOL_XTRING_H__STRTO_TYPE
