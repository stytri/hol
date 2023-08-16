#ifndef HOLIB_TEST_WMEM_H
#define HOLIB_TEST_WMEM_H 1
/*
MIT License

Copyright (c) 2023 Tristan Styles

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

#include <stdlib.h>
#include <wchar.h>
#include <errno.h>

//------------------------------------------------------------------------------

static inline void
wfree(
	void *p
) {
	free(p);
}

static inline void *
wmalloc(
	size_t z
) {
	z += !z;
	if(z <= (SIZE_MAX / sizeof(wchar_t))) {
		size_t wz = sizeof(wchar_t) * z;
		return malloc(wz);
	}
	errno = ENOMEM;
	return NULL;
}

static inline void *
wcalloc(
	size_t n,
	size_t z
) {
	z += !z;
	if(z <= (SIZE_MAX / sizeof(wchar_t))) {
		if(n <= ((SIZE_MAX / sizeof(wchar_t)) / z)) {
			size_t wz = sizeof(wchar_t) * z;
			return calloc(n, wz);
		}
	}
	errno = ENOMEM;
	return NULL;
}

static inline void *
wrealloc(
	void  *p,
	size_t z
) {
	z += !z;
	if(z <= (SIZE_MAX / sizeof(wchar_t))) {
		size_t wz = sizeof(wchar_t) * z;
		return realloc(p, wz);
	}
	errno = ENOMEM;
	return NULL;
}

//------------------------------------------------------------------------------

#endif//ndef HOLIB_TEST_WMEM_H
