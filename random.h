#ifndef HOL_RANDOM_H__INCLUDED
#define HOL_RANDOM_H__INCLUDED  1
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

#include <hol/holib.h>

//------------------------------------------------------------------------------

extern void fillrandom(uint64_t (*gen)(void *), void *ctx, void *buf, size_t buflen);
extern uint64_t fallback_genrandom(void *ctx); // XRAND64 *ctx
extern ssize_t fallback_getrandom(void *buf, size_t buflen, unsigned int flags);
#ifdef _WIN32
extern ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);
#endif

//------------------------------------------------------------------------------

#endif//ndef HOL_RANDOM_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_RANDOM_H__IMPLEMENTATION
#undef HOL_RANDOM_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/holib.h>

//------------------------------------------------------------------------------

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
void fillrandom(uint64_t (*gen)(void *), void *ctx, void *buf, size_t buflen) {
	uint8_t *b = buf;
	for(size_t x = 0; x < buflen; ) {
		size_t   n = buflen - x;
		uint64_t r = gen(ctx);
		switch(n) {
		default: b[x++] = (r >>  0) & 0xff; nobreak;
		case  7: b[x++] = (r >>  8) & 0xff; nobreak;
		case  6: b[x++] = (r >> 16) & 0xff; nobreak;
		case  5: b[x++] = (r >> 24) & 0xff; nobreak;
		case  4: b[x++] = (r >> 32) & 0xff; nobreak;
		case  3: b[x++] = (r >> 40) & 0xff; nobreak;
		case  2: b[x++] = (r >> 48) & 0xff; nobreak;
		case  1: b[x++] = (r >> 56) & 0xff; break;
		}
	}
}

uint64_t fallback_genrandom(void *ctx) {
	return xrand64(ctx);
}

ssize_t fallback_getrandom(void *buf, size_t buflen, unsigned int flags) {
	XRAND64 xr;
	XRAND64 xt;
	struct timespec t;
	timespec_get(&t, TIME_UTC);
	xseed64(&xt, ((uint64_t)t.tv_sec * 1000000000ULL) + t.tv_nsec);
	uint64_t s[5];
	for(size_t i = 0; i < 5; i++) {
		for(size_t j = 0; j < 64; j++) {
			(void)xrand64(&xr);
			(void)xrand64(&xt);
		}
		s[i] = xrand64(&xr) + xrand64(&xt);
	}
	for(size_t i = 0; i < 5; i++) {
		xr.a[i] = s[i];
	}
	fillrandom(fallback_genrandom, &xr, buf, buflen);
	return buflen;
	(void)flags;
}
#pragma GCC diagnostic pop

#ifdef _WIN32
#include <windef.h>
#include <wtypesbase.h>
#include <wincrypt.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
ssize_t getrandom(void *buf, size_t buflen, unsigned int flags) {
	ssize_t z = -1;
	HCRYPTPROV hcp;
	if(CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, 0)
		|| ((GetLastError() == NTE_BAD_KEYSET)
			&& CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)
		)
	) {
		if(CryptGenRandom(hcp, buflen, buf)) {
			z = buflen;
		}
		CryptReleaseContext(hcp, 0);
	}
	if(z < 0) {
		errno = EINVAL;
	}
	return z;
	(void)flags;
}
#pragma GCC diagnostic pop
#endif

//------------------------------------------------------------------------------

#endif//ndef HOL_RANDOM_H__IMPLEMENTATION

