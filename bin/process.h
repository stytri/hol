#ifndef HOLIB_TEST_PROCESS_H
#define HOLIB_TEST_PROCESS_H 1
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

#include <hol/holibc.h>
#include <signal.h>
#include <stdio.h>

//------------------------------------------------------------------------------

static volatile sig_atomic_t gSignal = 0;

static void
signal_handler(
	int signal
) {
	gSignal = signal;
}

//------------------------------------------------------------------------------

static size_t
sizeof_underlying_data_type(
	size_t bits
) {
	if(bits > 32) return sizeof(uint64_t);
	if(bits > 16) return sizeof(uint32_t);
	if(bits >  8) return sizeof(uint16_t);
	return sizeof(uint8_t);
}

//------------------------------------------------------------------------------

#ifdef EPIPE
#	define PROCESS__EPIPE  (EPIPE)
#else
#	define PROCESS__EPIPE  (0)
#endif

#ifndef ALWAYS_INLINE
#	if defined(__GNUC__)
#		define ALWAYS_INLINE  inline __attribute__((always_inline))
#	else
#		define ALWAYS_INLINE  inline
#	endif
#endif

#define PROCESS(PROCESS__stream,PROCESS__test,PROCESS__action,PROCESS__buffer,PROCESS__size,PROCESS__count,PROCESS__bits)  do \
{ \
	for(size_t PROCESS__i = 0; !gSignal; ) { \
		size_t PROCESS__n = fread(PROCESS__buffer, PROCESS__size, PROCESS__count, PROCESS__stream); \
		if(PROCESS__n > 0) { \
			bool PROCESS__no_continue = false; \
			if(PROCESS__size == sizeof(uint64_t)) { \
				uint64_t *PROCESS__b = PROCESS__buffer; \
				for(size_t PROCESS__j = 0; !gSignal && (PROCESS__j < PROCESS__n); PROCESS__j++) { \
					if(PROCESS__test(PROCESS__b[PROCESS__j], PROCESS__size, PROCESS__bits, PROCESS__i++)) { \
						PROCESS__action; \
						PROCESS__no_continue = true; \
						break; \
					} \
				} \
			} else if(PROCESS__size == sizeof(uint32_t)) { \
				uint32_t *PROCESS__b = PROCESS__buffer; \
				for(size_t PROCESS__j = 0; !gSignal && (PROCESS__j < PROCESS__n); PROCESS__j++) { \
					if(PROCESS__test(PROCESS__b[PROCESS__j], PROCESS__size, PROCESS__bits, PROCESS__i++)) { \
						PROCESS__action; \
						PROCESS__no_continue = true; \
						break; \
					} \
				} \
			} else if(PROCESS__size == sizeof(uint16_t)) { \
				uint16_t *PROCESS__b = PROCESS__buffer; \
				for(size_t PROCESS__j = 0; !gSignal && (PROCESS__j < PROCESS__n); PROCESS__j++) { \
					if(PROCESS__test(PROCESS__b[PROCESS__j], PROCESS__size, PROCESS__bits, PROCESS__i++)) { \
						PROCESS__action; \
						PROCESS__no_continue = true; \
						break; \
					} \
				} \
			} else if(PROCESS__size == sizeof(uint8_t)) { \
				uint8_t *PROCESS__b = PROCESS__buffer; \
				for(size_t PROCESS__j = 0; !gSignal && (PROCESS__j < PROCESS__n); PROCESS__j++) { \
					if(PROCESS__test(PROCESS__b[PROCESS__j], PROCESS__size, PROCESS__bits, PROCESS__i++)) { \
						PROCESS__action; \
						PROCESS__no_continue = true; \
						break; \
					} \
				} \
			} \
			if(PROCESS__no_continue) break; \
		} \
		if(ferror(PROCESS__stream)) { \
			if(!PROCESS__EPIPE \
				|| (errno != PROCESS__EPIPE) \
			) { \
				perror(); \
				fail(); \
			} \
			break; \
		} \
		if(feof(PROCESS__stream)) break; \
	} \
} while(0)

//------------------------------------------------------------------------------

#endif//ndef HOLIB_TEST_PROCESS_H
