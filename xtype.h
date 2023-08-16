#ifndef HOL_XTYPE_H__INCLUDED
#define HOL_XTYPE_H__INCLUDED 1
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

#include <stdio.h>
#include <ctype.h>

//------------------------------------------------------------------------------

static inline int iseol(int c) {
	return !c
		|| (c == '\n')
		|| (c == '\r')
		|| (c == '\f')
		|| (c == '\v')
		|| (c == EOF)
		;
}
static inline int isident (int c) { return isalnum(c) || (c == '_'); }
static inline int isbdigit(int c) { return (c == '0') || (c == '1'); }
static inline int isodigit(int c) { return (c >= '0') && (c <= '8'); }

static inline int todigit (int c) { return c - '0'; }
static inline int toxdigit(int c) { return isdigit(c) ? c - '0' : 10 + toupper(c) - 'A'; }

extern int str_is(char const *cs, int (*is_ctype)(int));

//------------------------------------------------------------------------------

#endif//ndef HOL_XTYPE_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XTYPE_H__IMPLEMENTATION

//------------------------------------------------------------------------------

int
str_is(
	char const *cs,
	int       (*is_ctype)(int)
) {
	int c = 0;
	if(cs) do {
		c = *cs++;
	} while(c && is_ctype(c))
			;
	return !c;
}

//------------------------------------------------------------------------------

#undef HOL_XTYPE_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#endif//def HOL_XTYPE_H__IMPLEMENTATION
