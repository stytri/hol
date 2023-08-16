#ifndef HOL_MKARRAY_H__INCLUDED
#define HOL_MKARRAY_H__INCLUDED 1
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

#	define MKARRAY__1     MKARRAY__0  MKARRAY__0
#	define MKARRAY__2     MKARRAY__1  MKARRAY__1
#	define MKARRAY__3     MKARRAY__2  MKARRAY__2
#	define MKARRAY__4     MKARRAY__3  MKARRAY__3
#	define MKARRAY__5     MKARRAY__4  MKARRAY__4
#	define MKARRAY__6     MKARRAY__5  MKARRAY__5
#	define MKARRAY__7     MKARRAY__6  MKARRAY__6
#	define MKARRAY__8     MKARRAY__7  MKARRAY__7
#	define MKARRAY__9     MKARRAY__8  MKARRAY__8
#	define MKARRAY__10    MKARRAY__9  MKARRAY__9
#	define MKARRAY__11    MKARRAY__10 MKARRAY__10
#	define MKARRAY__12    MKARRAY__11 MKARRAY__11
#	define MKARRAY__13    MKARRAY__12 MKARRAY__12
#	define MKARRAY__14    MKARRAY__13 MKARRAY__13
#	define MKARRAY__15    MKARRAY__14 MKARRAY__14
#	define MKARRAY__16    MKARRAY__15 MKARRAY__15
#	define MKARRAY__17    MKARRAY__16 MKARRAY__16
#	define MKARRAY__18    MKARRAY__17 MKARRAY__17
#	define MKARRAY__19    MKARRAY__18 MKARRAY__18
#	define MKARRAY__20    MKARRAY__19 MKARRAY__19
#	define MKARRAY__21    MKARRAY__20 MKARRAY__20
#	define MKARRAY__22    MKARRAY__21 MKARRAY__21
#	define MKARRAY__23    MKARRAY__22 MKARRAY__22
#	define MKARRAY__24    MKARRAY__23 MKARRAY__23
#	define MKARRAY__25    MKARRAY__24 MKARRAY__24
#	define MKARRAY__26    MKARRAY__25 MKARRAY__25
#	define MKARRAY__27    MKARRAY__26 MKARRAY__26
#	define MKARRAY__28    MKARRAY__27 MKARRAY__27
#	define MKARRAY__29    MKARRAY__28 MKARRAY__28
#	define MKARRAY__30    MKARRAY__29 MKARRAY__29
#	define MKARRAY__31    MKARRAY__30 MKARRAY__30

//------------------------------------------------------------------------------

#endif//ndef HOL_MKARRAY_H__INCLUDED

//------------------------------------------------------------------------------

#ifndef MKARRAY_SIZE
#	define UNDEF_MKARRAY_SIZE  1
#	define MKARRAY_SIZE        0
#endif
#ifndef MKARRAY_DATA
#	define UNDEF_MKARRAY_DATA  1
#	define MKARRAY_DATA        0,
#endif

#define MKARRAY__0  MKARRAY_DATA

#if (MKARRAY_SIZE) & (1UL << 31)
	MKARRAY__31
#endif
#if (MKARRAY_SIZE) & (1UL << 30)
	MKARRAY__30
#endif
#if (MKARRAY_SIZE) & (1UL << 29)
	MKARRAY__29
#endif
#if (MKARRAY_SIZE) & (1UL << 28)
	MKARRAY__28
#endif
#if (MKARRAY_SIZE) & (1UL << 27)
	MKARRAY__27
#endif
#if (MKARRAY_SIZE) & (1UL << 26)
	MKARRAY__26
#endif
#if (MKARRAY_SIZE) & (1UL << 25)
	MKARRAY__25
#endif
#if (MKARRAY_SIZE) & (1UL << 24)
	MKARRAY__24
#endif
#if (MKARRAY_SIZE) & (1UL << 23)
	MKARRAY__23
#endif
#if (MKARRAY_SIZE) & (1UL << 22)
	MKARRAY__22
#endif
#if (MKARRAY_SIZE) & (1UL << 21)
	MKARRAY__21
#endif
#if (MKARRAY_SIZE) & (1UL << 20)
	MKARRAY__20
#endif
#if (MKARRAY_SIZE) & (1UL << 19)
	MKARRAY__19
#endif
#if (MKARRAY_SIZE) & (1UL << 18)
	MKARRAY__18
#endif
#if (MKARRAY_SIZE) & (1UL << 17)
	MKARRAY__17
#endif
#if (MKARRAY_SIZE) & (1UL << 16)
	MKARRAY__16
#endif
#if (MKARRAY_SIZE) & (1UL << 15)
	MKARRAY__15
#endif
#if (MKARRAY_SIZE) & (1UL << 14)
	MKARRAY__14
#endif
#if (MKARRAY_SIZE) & (1UL << 13)
	MKARRAY__13
#endif
#if (MKARRAY_SIZE) & (1UL << 12)
	MKARRAY__12
#endif
#if (MKARRAY_SIZE) & (1UL << 11)
	MKARRAY__11
#endif
#if (MKARRAY_SIZE) & (1UL << 10)
	MKARRAY__10
#endif
#if (MKARRAY_SIZE) & (1UL << 9)
	MKARRAY__9
#endif
#if (MKARRAY_SIZE) & (1UL << 8)
	MKARRAY__8
#endif
#if (MKARRAY_SIZE) & (1UL << 7)
	MKARRAY__7
#endif
#if (MKARRAY_SIZE) & (1UL << 6)
	MKARRAY__6
#endif
#if (MKARRAY_SIZE) & (1UL << 5)
	MKARRAY__5
#endif
#if (MKARRAY_SIZE) & (1UL << 4)
	MKARRAY__4
#endif
#if (MKARRAY_SIZE) & (1UL << 3)
	MKARRAY__3
#endif
#if (MKARRAY_SIZE) & (1UL << 2)
	MKARRAY__2
#endif
#if (MKARRAY_SIZE) & (1UL << 1)
	MKARRAY__1
#endif
#if (MKARRAY_SIZE) & (1UL << 0)
	MKARRAY__0
#endif

#undef MKARRAY__0
#if UNDEF_MKARRAY_SIZE
#	undef MKARRAY_DATA
#endif
#if UNDEF_MKARRAY_SIZE
#	undef MKARRAY_SIZE
#endif

//------------------------------------------------------------------------------
