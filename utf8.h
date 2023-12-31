#ifndef HOL_UTF8_H_INCLUDED
#define HOL_UTF8_H_INCLUDED
/*
MIT License

Copyright (c) 2022 Tristan Styles

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

#include <stddef.h>
#include <uchar.h>

//------------------------------------------------------------------------------

extern size_t
utf8off(
	char const  *cs,
	char const **endp,
	size_t       index
);

extern size_t
utf8len(
	char const  *cs,
	char const **endp,
	size_t       maxlen
);

extern char32_t
utf8chr(
	char const  *cs,
	char const **endp
);

extern size_t
utf8enlen(
	char32_t cc
);

extern size_t
utf8encode(
	char    *s,
	char   **endp,
	char32_t cc
);

//------------------------------------------------------------------------------

#endif//ndef HOL_UTF8_H_INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_UTF8_H__IMPLEMENTATION
#undef HOL_UTF8_H__IMPLEMENTATION

#include <stdint.h>

//------------------------------------------------------------------------------

#define UTF8LEN(C)  ((UINT64_C(0x4322000011111111) >> (((C) >> 4) << 2)) & 0xF)

//------------------------------------------------------------------------------

static inline char32_t
utf8char32(
	char const *cs
) {
	return (char32_t)*(uint8_t *)cs;
}

size_t
utf8off(
	char const  *cs,
	char const **endp,
	size_t       index
) {
	size_t off = 0;

	for(size_t i = 0; i++ < index; ) {
		char32_t const c = utf8char32(cs);
		if(!c) goto end;

		size_t const n = UTF8LEN(c);
		off += n;
		cs += n;
	}
end:
	if(endp) *endp = cs;
	return off;
}

size_t
utf8len(
	char const  *cs,
	char const **endp,
	size_t       maxlen
) {
	size_t len = 0;

	for(; len < maxlen; ++len) {
		char32_t  c = utf8char32(cs);
		int const n = UTF8LEN(c);
		switch(n) {
		case 1:
			if(unlikely(!c)) goto end;
			++cs;
			continue;
		case 2:
			c = utf8char32(++cs);
			if(unlikely((c & 0xC0) != 0x80)) goto err;
			++cs;
			continue;
		case 3:
			c = utf8char32(++cs);
			if(unlikely((c & 0xC0) != 0x80)) goto err;
			c = utf8char32(++cs);
			if(unlikely((c & 0xC0) != 0x80)) goto err;
			++cs;
			continue;
		case 4:
			if(unlikely(c > 0xF7))           goto err;
			c = utf8char32(++cs);
			if(unlikely((c & 0xC0) != 0x80)) goto err;
			c = utf8char32(++cs);
			if(unlikely((c & 0xC0) != 0x80)) goto err;
			c = utf8char32(++cs);
			if(unlikely((c & 0xC0) != 0x80)) goto err;
			++cs;
			continue;
		case 0:
		default:
			for(c = utf8char32(++cs); (c & 0xC0) == 0x80; c = utf8char32(++cs))
				;
			goto err;
		}
	}
end:
	if(endp) *endp = cs;
	return len;
err:
	if(endp) *endp = cs;
	return ~(size_t)0;
}

char32_t
utf8chr(
	char const  *cs,
	char const **endp
) {
	char32_t  c = utf8char32(cs);
	int const n = UTF8LEN(c);
	switch(n) {
		char32_t cc;
	case 1:
		if(endp) *endp = cs + !!c;
		return c;
	case 2:
		cc = (c & 0x1F) << 6;
		c = utf8char32(++cs);
		if(unlikely((c & 0xC0) != 0x80)) goto err;
		cc |= (c & 0x3F);
		if(endp) *endp = cs + 1;
		return cc;
	case 3:
		cc = (c & 0x0F) << 12;
		c = utf8char32(++cs);
		if(unlikely((c & 0xC0) != 0x80)) goto err;
		cc |= (c & 0x3F) << 6;
		c = utf8char32(++cs);
		if(unlikely((c & 0xC0) != 0x80)) goto err;
		cc |= (c & 0x3F);
		if(endp) *endp = cs + 1;
		return cc;
	case 4:
		if(unlikely(c > 0xF7))           goto err;
		cc = (c & 0x07) << 18;
		c = utf8char32(++cs);
		if(unlikely((c & 0xC0) != 0x80)) goto err;
		cc |= (c & 0x3F) << 12;
		c = utf8char32(++cs);
		if(unlikely((c & 0xC0) != 0x80)) goto err;
		cc |= (c & 0x3F) << 6;
		c = utf8char32(++cs);
		if(unlikely((c & 0xC0) != 0x80)) goto err;
		cc |= (c & 0x3F);
		if(endp) *endp = cs + 1;
		return cc;
	case 0:
	default:
		for(c = utf8char32(++cs); (c & 0xC0) == 0x80; c = utf8char32(++cs))
			;
	err:
		if(endp) *endp = cs;
		return ~(char32_t)0;
	}
}

size_t
utf8enlen(
	char32_t cc
) {
	return (size_t)((cc < 0x110000) + (cc > 0x000080) + (cc > 0x000800) + (cc > 0x0001000));
}

size_t
utf8encode(
	char    *s,
	char   **endp,
	char32_t cc
) {
	size_t const n = utf8enlen(cc);
	switch(n) {
	case 1:
		*s++ = (char)(cc);
		break;
	case 2:
		*s++ = (char)(0xC0 | ((cc >>  6) & 0x1F));
		*s++ = (char)(0x80 | ( cc        & 0x3F));
		break;
	case 3:
		*s++ = (char)(0xE0 | ((cc >> 12) & 0x0F));
		*s++ = (char)(0x80 | ((cc >>  6) & 0x3F));
		*s++ = (char)(0x80 | ( cc        & 0x3F));
		break;
	case 4:
		*s++ = (char)(0xF0 | ((cc >> 18) & 0x07));
		*s++ = (char)(0x80 | ((cc >> 12) & 0x3F));
		*s++ = (char)(0x80 | ((cc >>  6) & 0x3F));
		*s++ = (char)(0x80 | ( cc        & 0x3F));
		break;
	case 0:
	default:
		break;
	}
	if(endp) *endp = s;
	return n;
}

//------------------------------------------------------------------------------

#endif//def HOL_UTF8_H__IMPLEMENTATION
