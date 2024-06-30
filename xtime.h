#ifndef HOL_XTIME_H__INCLUDED
#define HOL_XTIME_H__INCLUDED 1
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

#include <time.h>

//------------------------------------------------------------------------------

#define time(...)  (time)(__VA_ARGS__+0)

//------------------------------------------------------------------------------

#define NO_TIMESTAMP        (0)
#define TIMESTAMP_UTC_TIME  (1)
#ifdef CLOCK_REALTIME
#define TIMESTAMP_REALTIME  (2)
#endif
#ifdef CLOCK_MONOTONIC
#define TIMESTAMP_MONOTIME  (3)
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
#define TIMESTAMP_CPU_TIME  (4)
#endif
#ifdef _WIN32
#define TIMESTAMP_PROCTIME  (5)
#endif

extern void timestamp    (int timer, struct timespec *tp);
extern void time_interval(struct timespec const *t1, struct timespec const *t2, struct timespec *tp);
extern void time_per     (struct timespec const *ti, size_t n, struct timespec *tp);

//------------------------------------------------------------------------------

#endif//ndef HOL_XTIME_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XTIME_H__IMPLEMENTATION
#undef HOL_XTIME_H__IMPLEMENTATION

#ifdef TIMESTAMP_PROCTIME
#include <windef.h>
#include <wtypesbase.h>
#endif

//------------------------------------------------------------------------------

void
timestamp(
	int              timer,
	struct timespec *tp
) {
	switch(timer) {
	case TIMESTAMP_UTC_TIME:
		timespec_get(tp, TIME_UTC);
		break;
#ifdef TIMESTAMP_REALTIME
	case TIMESTAMP_REALTIME:
		clock_gettime(CLOCK_REALTIME, tp);
		break;
#endif
#ifdef TIMESTAMP_MONOTIME
	case TIMESTAMP_MONOTIME:
		clock_gettime(CLOCK_MONOTONIC, tp);
		break;
#endif
#ifdef TIMESTAMP_CPU_TIME
	case TIMESTAMP_CPU_TIME:
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tp);
		break;
#endif
#ifdef TIMESTAMP_PROCTIME
	case TIMESTAMP_PROCTIME: {
			FILETIME createTime;
			FILETIME exitTime;
			FILETIME kernelTime;
			FILETIME userTime;
			if(GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime)) {
				uint64_t ut = ((uint64_t)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;
				uint64_t kt = ((uint64_t)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
				uint64_t ct = (ut + kt) * 100;
				tp->tv_sec  = ct / UINT64_C(1000000000);
				tp->tv_nsec = ct % UINT64_C(1000000000);
			}
		}
		break;
#endif
	default:
		break;
	}
}

void
time_interval(
	struct timespec const *t1,
	struct timespec const *t2,
	struct timespec       *tp
) {
	tp->tv_sec = t2->tv_sec - t1->tv_sec;
	if(t2->tv_nsec < t1->tv_nsec) {
		tp->tv_nsec = t1->tv_nsec - t2->tv_nsec;
		tp->tv_sec -= 1;
	} else {
		tp->tv_nsec = t2->tv_nsec - t1->tv_nsec;
	}
}

void
time_per(
	struct timespec const *ti,
	size_t                 n,
	struct timespec       *tp
) {
	if(n > 1) {
		unsigned long long b = 1000UL * 1000UL * 1000UL;
		unsigned long long g = (ti->tv_sec  % n) * b;
		unsigned long long r = (ti->tv_nsec + g) / n;
		tp->tv_sec  = (ti->tv_sec / n) + (r / b);
		tp->tv_nsec = r % b;
	}
}

//------------------------------------------------------------------------------

#endif//def HOL_XTIME_H__IMPLEMENTATION
