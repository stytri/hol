#ifndef HOL_XTDIO_H__INCLUDED
#define HOL_XTDIO_H__INCLUDED 1
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

#include <hol/xtdlib.h>
#include <inttypes.h>
#include <stdio.h>

//------------------------------------------------------------------------------

#ifndef BUFSIZE
#	if __MINGW64__
#		define BUFSIZE  (8 Ki)
#	elif __MINGW32__
#		define BUFSIZE  (4 Ki)
#	else
#		define BUFSIZE  BUFSIZ
#	endif
#endif

//------------------------------------------------------------------------------

struct do__printf {
	int  (*print)(
		struct do__printf *context,
		char const        *fmt,
		...
	);
	void  *stream;
	union {
	size_t  size;
#if __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__
	rsize_t rsize;
#endif// __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__
	};
};

//------------------------------------------------------------------------------

static inline int ungetchar(int c) { return ungetc(c, stdin); }

static inline int putstr(char const *cs) { return fputs(cs, stdout); }

extern int do_printf(struct do__printf *context, char const *fmt, ...);
extern int do_fprintf(struct do__printf *context, char const *fmt, ...);
extern int do_sprintf(struct do__printf *context, char const *fmt, ...);
extern int do_snprintf(struct do__printf *context, char const *fmt, ...);
#if __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__
extern int do_printf_s(struct do__printf *context, char const *fmt, ...);
extern int do_fprintf_s(struct do__printf *context, char const *fmt, ...);
extern int do_sprintf_s(struct do__printf *context, char const *fmt, ...);
extern int do_snprintf_s(struct do__printf *context, char const *fmt, ...);
#endif//  __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__

extern int do_print_time_interval(struct do__printf *context, unsigned long sec, unsigned long nsec);

extern void fchars(int c, size_t n, FILE *out);
extern void fspaces(size_t n, FILE *out);
extern void ftabs(size_t n, FILE *out);

extern size_t fputln(FILE *out, ...);

extern char *mfgets(FILE *in);

static inline char *mgets(void) { return mfgets(stdin); }

extern char *loadfile(char const *file, size_t *np, size_t *zp);
extern char *loadrecursive(char const *file, char const *sep, size_t *np, size_t *zp);

extern void errorf(char const *fmt, ...);

extern void perror__with_file_and_line(char const *file, int line, char const *cs);
#define perror(...)  do { \
	perror__with_file_and_line(__FILE__,__LINE__,__VA_ARGS__+0); \
} while(0)

extern void fatalerror__print_and_abort(char const *file, int line, int errn);
#define fatalerror(...)  do { \
	int fatalerror__errno = (__VA_ARGS__+0); \
	fatalerror__print_and_abort(__FILE__, __LINE__, fatalerror__errno ? fatalerror__errno : errno); \
} while(0)

//------------------------------------------------------------------------------

static inline int
print_time_interval(
	unsigned long sec,
	unsigned long nsec
) {
	struct do__printf context = { do_printf, NULL, { 0 } };
	return do_print_time_interval(&context, sec, nsec);
}

static inline int
fprint_time_interval(
	FILE         *fp,
	unsigned long sec,
	unsigned long nsec
) {
	struct do__printf context = { do_fprintf, fp, { 0 } };
	return do_print_time_interval(&context, sec, nsec);
}

#if __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__
static inline int
sprint_time_interval_s(
	char         *s,
	rsize_t       n,
	unsigned long sec,
	unsigned long nsec
) {
	struct do__printf context = { do_sprintf_s, s, { .rsize = n } };
	return do_print_time_interval(&context, sec, nsec);
}

static inline int
snprint_time_interval_s(
	char         *s,
	rsize_t       n,
	unsigned long sec,
	unsigned long nsec
) {
	struct do__printf context = { do_snprintf_s, s, { .rsize = n } };
	return do_print_time_interval(&context, sec, nsec);
}
#endif// __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__

//------------------------------------------------------------------------------

#endif//ndef HOL_XTDIO_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XTDIO_H__IMPLEMENTATION
#undef HOL_XTDIO_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/xtype.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

//------------------------------------------------------------------------------

int
do_printf(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	(void)context;

	int r;
	va_list va;
	va_start(va, fmt);
	r = vprintf(fmt, va);
	va_end(va);
	return r;
}

int
do_fprintf(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vfprintf(context->stream, fmt, va);
	va_end(va);
	return r;
}

int
do_sprintf(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vsprintf(context->stream, fmt, va);
	va_end(va);
	return r;
}

int
do_snprintf(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vsnprintf(context->stream, context->size, fmt, va);
	va_end(va);
	return r;
}

#if __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__

int
do_printf_s(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vprintf_s(fmt, va);
	va_end(va);
	return r;
}

int
do_fprintf_s(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vfprintf_s(context->stream, fmt, va);
	va_end(va);
	return r;
}

int
do_sprintf_s(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vsprintf(context->stream, context->rsize, fmt, va);
	va_end(va);
	return r;
}

int
do_snprintf_s(
	struct do__printf *context,
	char const        *fmt,
	...
) {
	int r;
	va_list va;
	va_start(va, fmt);
	r = vsnprintf(context->stream, context->rsize, fmt, va);
	va_end(va);
	return r;
}

#endif//  __STDC_LIB_EXT1__ && __STDC_WANT_LIB_EXT1__

//------------------------------------------------------------------------------

int
do_print_time_interval(
	struct do__printf *context,
	unsigned long      sec,
	unsigned long      nsec
) {
	unsigned long h  = (sec / 60lu) / 60lu;
	unsigned      m  = (sec / 60lu) % 60u;
	unsigned      s  =  sec         % 60u;
	unsigned      ms = (nsec / 1000lu) / 1000u;
	unsigned      us = (nsec / 1000lu) % 1000u;
	unsigned      ns =  nsec           % 1000u;

	for(;;) if(h > 0) {
		if(s >= 30) {
			m  = (m + 1) % 60;
			h += !m;
			s = ms = us = ns = 0;
			continue;
		}
		return context->print(context, "%lu:%2.2u""h", h, m);
	} else if(m > 0) {
		if(ms >= 500) {
			s  = (s + 1) % 60;
			m  = (m + !s) % 60;
			h += !m;
			ms = us = ns = 0;
			continue;
		}
		return context->print(context, "%u:%2.2u""m", m, s);
	} else if((s > 0) || ((ms|us|ns) == 0)) {
		if(us >= 500) {
			ms = (ms + 1) % 1000;
			s  = (s  + !ms) % 60;
			m  = (m  + !s) % 60;
			us = ns = 0;
			continue;
		}
		return context->print(context, "%u.%3.3u""s", s, ms);
	} else if(ms > 0) {
		if(ns >= 500) {
			us = (us + 1) % 1000;
			ms = (ms + !us) % 1000;
			s  = (s  + !ms) % 60;
			ns = 0;
			continue;
		}
		return context->print(context, "%u.%3.3u""ms", ms, us);
	} else if(us > 0) {
		return context->print(context, "%u.%3.3u""us", us, ns);
	} else {
		return context->print(context, "%u""ns", ns);
	}
}

//------------------------------------------------------------------------------

inline void
fchars(
	int    c,
	size_t n,
	FILE  *out
) {
	char chars[8 + 1];
	memset(chars, c, (sizeof(chars) - 1));
	chars[sizeof(chars) - 1] = '\0';

	for(; n >= (sizeof(chars) - 1); n -= (sizeof(chars) - 1)) {
		fputs(chars, out);
	}
	fputs(chars + ((sizeof(chars) - 1) - n), out);

	return;
}

void
fspaces(
	size_t n,
	FILE  *out
) {
	fchars(' ', n, out);
}

void
ftabs(
	size_t n,
	FILE  *out
) {
	fchars('\t', n, out);
}

//------------------------------------------------------------------------------

size_t
fputln(
	FILE *out,
	...
) {
	size_t n = 0;

	va_list va;
	va_start(va, out);
	for(char const *cs; (cs = va_arg(va, char const *)) != NULL; n++) {
		if(unlikely(fputs(cs, out) == EOF)) break;
		if(unlikely(fputc('\n', out) == EOF)) break;
	}
	va_end(va);

	return n;
}

//------------------------------------------------------------------------------

char *
mfgets(
	FILE *in
) {
	char *buf = malloc(1);
	if(likely(buf)) {
		size_t z = 0, n = 0;
		for(int c;
			(buf[n] = '\0') || (((c = fgetc(in)) != EOF) && !iseol(c));
			buf[n++] = (char)c
		) {
			if(n == z) {
				void *b = realloc(buf, (z=((z+1)+(z>>2)))+1);
				if(unlikely(!b)) break;
				buf = b;
			}
		}
	}
	return buf;
}

char *
loadfile(
	char const *file,
	size_t     *np,
	size_t     *zp
) {
	size_t  n = 0;
	size_t  z = 0;
	char   *s = NULL;
	FILE   *f = fopen(file, "rb");
	if(f) {
		size_t m = z = BUFSIZE - 1;
		if(fseek(f, 0, SEEK_END) == 0) {
			long l = ftell(f);
			if(l > 0) {
				m = (size_t)l + 1;
				z = roundup(BUFSIZE, m) - 1;
			}
		}
		rewind(f);
		s = malloc(z+1);
		if(likely(s)) {
			for(size_t c; (c = fread(s + n, 1, m, f)) > 0; ) {
				n += c;
				if(c < m) break;
				if(feof(f) || ferror(f)) break;
				m = BUFSIZE;
				z = n + m;
				void *p = realloc(s, z + 1);
				if(unlikely(!p)) break;
				s = p;
			}
			s[n] = '\0';
			if(ferror(f)) {
				free(s);
				s = NULL;
				n = z = 0;
			}
			fclose(f);
		}
	}
	if(np) *np = n;
	if(zp) *zp = z;
	return s;
}

static bool
recursiveloadfile(
	char const *file,
	char      **sp,
	size_t     *np,
	size_t     *zp,
	char const *sep
) {
	bool        append_sep = false;
	char const *cs = strrchr(file, '/');
	size_t      z, n;
	char       *t;
#ifdef _WIN32
	if(!cs) cs = strrchr(file, '\\');
#endif
	if(cs) {
		char const *ct = cs++;
		while(ct > file) {
			ct--;
			if((*ct == '/')
#ifdef _WIN32
				|| (*ct == '\\')
#endif
			) {
				break;
			}
		}
		if(ct == file) {
			return false;
		}
		ct++;
		z = ct - file;
		n = strlen(cs);
		t = malloc(z + n + 1);
		if(!t) goto fail;
		strcpy(strncpy(t, file, z) + z, cs);
		append_sep = recursiveloadfile(t, sp, np, zp, sep);
		free(t);
	}
	FILE *f = fopen(file, "rb");
	if(f) {
		if(append_sep) {
			n = strlen(sep);
			if(n > (*zp - *np)) {
				z = roundup(BUFSIZE, *zp + n + 1);
				t = realloc(*sp, z);
				if(!t) goto fail;
				*sp = t;
				*zp = z;
			}
			memcpy(*sp + *np, sep, n);
			*np += n;
		}
		n = BUFSIZE - 1;
		if(fseek(f, 0, SEEK_END) == 0) {
			long l = ftell(f);
			if(l > 0) {
				n = (size_t)l + 1;
			}
		}
		rewind(f);
		z = roundup(BUFSIZE, *zp + n + 1);
		t = realloc(*sp, z);
		if(!t) goto fail;
		*sp = t;
		*zp = z;
		for(size_t c; (c = fread(*sp + *np, 1, n, f)) > 0; ) {
			*np += c;
			if(c < n) break;
			if(feof(f) || ferror(f)) break;
			n = BUFSIZE;
			z = roundup(BUFSIZE, *zp + n + 1);
			t = realloc(*sp, z);
			if(!t) goto fail;
			*sp = t;
			*zp = z;
		}
		if(!ferror(f)) {
			fclose(f);
			return (sep && *sep);
		}
fail:
		free(*sp);
		*sp = NULL;
		*np = *zp = 0;
	}
	return false;
}

char *
loadrecursive(
	char const *file,
	char const *sep,
	size_t     *np,
	size_t     *zp
) {
	size_t  n = 0;
	size_t  z = 0;
	char   *s = NULL;
	recursiveloadfile(file, &s, &n, &z, sep);
	if(s) s[n] = '\0';
	if(np) *np = n;
	if(zp) *zp = z;
	return s;
}

//------------------------------------------------------------------------------

void
errorf(
	char const *fmt,
	...
) {
	va_list val;
	va_start(val, fmt);

	vfprintf(stderr, fmt, val);

	fputc('\n', stderr);
	fflush(stderr);

	va_end(val);
	return;
}

void
perror__with_file_and_line(
	char const *file,
	int         line,
	char const *cs
) {
	fprintf(stderr, "%s:%i: ", file, line);
	(perror)(cs);
	fflush(stderr);
}

void
fatalerror__print_and_abort(
	char const *file,
	int         line,
	int         errn
) {
	fprintf(stderr, "%s:%i: %s", file, line, strerror(errn));
	fflush(stderr);
	abort();
}

//------------------------------------------------------------------------------

#endif//def HOL_XTDIO_H__IMPLEMENTATION
