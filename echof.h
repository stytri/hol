#ifndef HOL_ECHOF_H__INCLUDED
#define HOL_ECHOF_H__INCLUDED 1
/*
MIT License

Copyright (c) 2022 Tristan Styles

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

#include <stdarg.h>

//------------------------------------------------------------------------------

extern int vechof(char const *fmt, va_list va);
extern int echof(char const *fmt, ...);

extern int echo_time_interval(unsigned long sec, unsigned long nsec);

//------------------------------------------------------------------------------

#endif//ndef HOL_ECHOF_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_ECHOF_H__IMPLEMENTATION
#undef HOL_ECHOF_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/xtdio.h>
#include <string.h>
#include <time.h>

//------------------------------------------------------------------------------

static void
echof_timestamp(
	FILE       *echo,
	char const *fmt
) {
	char      buf[20];
	time_t    tt;
	struct tm t;
	time(&tt);
#ifdef _WIN32
	// Windows has different parameter order and return value
	localtime_s(&t, &tt);
#else
	localtime_s(&tt, &t);
#endif//def _WIN32
	strftime(buf, sizeof(buf), fmt, &t);
	if(echo) {
		fputs(buf, echo);
	}
	fputs(buf, stdout);
}

int
vechof(
	char const *fmt,
	va_list     va
) {
#define echof__streq(echof__streq__cs,echof__streq__ct)  (\
	strncmp(echof__streq__cs,echof__streq__ct,sizeof(echof__streq__ct)-1) ? (\
		0 \
	):(\
		echof__streq__cs += sizeof(echof__streq__ct)-1, \
		1 \
	)\
)

	static FILE *echo   = NULL;
	static int   indent = 0;

	int r = 0;

	do if(echof__streq(fmt, "%<")) {
		if(echof__streq(fmt, "+>")) {
			indent++;

		} else if(echof__streq(fmt, "->")) {
			indent--;

		} else if(echof__streq(fmt, "time>")) {
			echof_timestamp(echo, "%T");

		} else if(echof__streq(fmt, "date time>")) {
			echof_timestamp(echo, "%F %T");

		} else if(echof__streq(fmt, "date>")) {
			echof_timestamp(echo, "%F");

		} else if(echof__streq(fmt, "echo on>")) {
			if(echo && (echo != stderr)) {
				fclose(echo);
			}
			echo = stderr;

		} else if(echof__streq(fmt, "echo off>")) {
			if(echo && (echo != stderr)) {
				fclose(echo);
			}
			echo = NULL;

		} else if(echof__streq(fmt, "echo %s>")) {
			char const *cs = va_arg(va, char const *);
			if(cs) {
				if(echo && (echo != stderr)) {
					fclose(echo);
				}
				echo = fopen(cs, "w");
				if(!echo) {
					r = -1;
				}
			}

		} else if(echof__streq(fmt, "indent %i>")) {
			indent = va_arg(va, int);
		}
		continue;

	} else {
		int ra = 0, re = 0;

		if(echo) {
			if(indent > 0) {
				ftabs(indent, echo);
			}
			va_list ve;
			va_copy(ve, va);
			re = vfprintf(echo, fmt, ve);
			va_end(ve);
		}

		if(indent > 0) {
			ftabs(indent, stdout);
		}
		ra = vfprintf(stdout, fmt, va);

		r = (re < 0) ? re : ra;
		break;

	} while(*fmt && (r >= 0))
		;
	return r;

#undef echof__streq
}

int
echof(
	char const *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vechof(fmt, va);
	va_end(va);
	return r;
}

//------------------------------------------------------------------------------

static int
do_echof(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	(void)context;

	int r;
	va_list va;
	va_start(va, fmt);
	r = vechof(fmt, va);
	va_end(va);
	return r;
}

int
echo_time_interval(
	unsigned long sec,
	unsigned long nsec
) {
	struct do__printf context = { do_echof, NULL, { 0 } };
	return do_print_time_interval(&context, sec, nsec);
}

//------------------------------------------------------------------------------

#endif//def HOL_ECHOF_H__IMPLEMENTATION
