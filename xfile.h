#ifndef HOL_XFILE_H__INCLUDED
#define HOL_XFILE_H__INCLUDED  1
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

#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//------------------------------------------------------------------------------

#define XFILE  struct xfile
XFILE {
	FILE       *stream;
	bool        is_std;
	bool        is_pipe;
	char const *mode;
	char const  name[];
};

//------------------------------------------------------------------------------

extern XFILE *xfclose(XFILE *f);
extern XFILE *xtmpfile(void);
extern XFILE *xfopen(char const *name, char const *mode);
#define xfopen(xfopen__name,...)  (xfopen)((xfopen__name),__VA_ARGS__+0)

//------------------------------------------------------------------------------

static inline FILE *xfstream(XFILE *f) {
	return f ? f->stream : (errno = EINVAL, NULL);
}

static inline bool xfispipe(XFILE *f) {
	return f ? f->is_pipe: (errno = EINVAL, false);
}

static inline bool xfisstd(XFILE *f) {
	return f ? f->is_std: (errno = EINVAL, false);
}

static inline char const *xfmode(XFILE *f) {
	return f ? f->mode : (errno = EINVAL, "");
}

static inline char const *xfname(XFILE *f) {
	return f ? f->name : (errno = EINVAL, "");
}

static inline bool xfisbinary(XFILE *f) {
	return f ? !!strpbrk(f->mode, "b") : (errno = EINVAL, false);
}

static inline bool xfiswrite(XFILE *f) {
	return f ? !!strpbrk(f->mode, "wa+") : (errno = EINVAL, false);
}

static inline bool xfisread(XFILE *f) {
	return f ? !!strpbrk(f->mode, "r+") : (errno = EINVAL, false);
}

//------------------------------------------------------------------------------

static inline void xsetbuf(XFILE *f, void *b) {
	if(f) setbuf(f->stream, b);
}

static inline int xsetvbuf(XFILE *f, void *b, int m, size_t z) {
	return f ? setvbuf(f->stream, b, m, z) : (errno = EINVAL, EOF);
}

static inline int xfflush(XFILE *f) {
	return f ? fflush(f->stream) : (errno = EINVAL, EOF);
}

static inline int xfeof(XFILE *f) {
	return f ? feof(f->stream) : (errno = EINVAL, 0);
}

static inline int xferror(XFILE *f) {
	return f ? ferror(f->stream) : (errno = EINVAL, 1);
}

static inline void xclearerr(XFILE *f) {
	if(f) clearerr(f->stream);
}

static inline void xrewind(XFILE *f) {
	if(f) rewind(f->stream);
}

static inline long xftell(XFILE *f) {
	return f ? ftell(f->stream) : (errno = EINVAL, -1L);
}

static inline long xfseek(XFILE *f, long o, int g) {
	return f ? fseek(f->stream, o, g) : (errno = EINVAL, -1L);
}

static inline long xfgetpos(XFILE *f, fpos_t *p) {
	return f ? fgetpos(f->stream, p) : (errno = EINVAL, -1L);
}

static inline long xfsetpos(XFILE *f, fpos_t const *p) {
	return f ? fsetpos(f->stream, p) : (errno = EINVAL, -1L);
}

//------------------------------------------------------------------------------

static inline size_t xfread(void *b, size_t z, size_t n, XFILE *f) {
	return f ? fread(b, z, n, f->stream) : (errno = EINVAL, 0);
}

static inline size_t xfwrite(void const *b, size_t z, size_t n, XFILE *f) {
	return f ? fwrite(b, z, n, f->stream) : (errno = EINVAL, 0);
}

static inline int xfgetc(XFILE *f) {
	return f ? fgetc(f->stream) : (errno = EINVAL, EOF);
}

static inline int xungetc(int c, XFILE *f) {
	return f ? ungetc(c, f->stream) : (errno = EINVAL, EOF);
}

static inline char *xfgets(char *s, int n, XFILE *f) {
	return f ? fgets(s, n, f->stream) : (errno = EINVAL, NULL);
}

static inline int xvfscanf(XFILE *f, char const *p, va_list v) {
	return f ? vfscanf(f->stream, p, v) : (errno = EINVAL, EOF);
}

static inline int xfscanf(XFILE *f, char const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfscanf(f, p, v);
	va_end(v);
	return r;
}

static inline int xfputc(int c, XFILE *f) {
	return f ? fputc(c, f->stream) : (errno = EINVAL, EOF);
}

static inline int xfputs(char const *s, XFILE *f) {
	return f ? fputs(s, f->stream) : (errno = EINVAL, EOF);
}

static inline int xvfprintf(XFILE *f, char const *p, va_list v) {
	return f ? vfprintf(f->stream, p, v) : (errno = EINVAL, EOF);
}

static inline int xfprintf(XFILE *f, char const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfprintf(f, p, v);
	va_end(v);
	return r;
}

//------------------------------------------------------------------------------

static inline int xfwide(XFILE *f, int m) {
	return f ? fwide(f->stream, m) : (errno = EINVAL, 0);
}

static inline wint_t xfgetwc(XFILE *f) {
	return f ? fgetwc(f->stream) : (errno = EINVAL, WEOF);
}

static inline wint_t xungetwc(wint_t c, XFILE *f) {
	return f ? ungetwc(c, f->stream) : (errno = EINVAL, WEOF);
}

static inline wchar_t *xfgetws(wchar_t *s, int n, XFILE *f) {
	return f ? fgetws(s, n, f->stream) : (errno = EINVAL, NULL);
}

static inline int xvfwscanf(XFILE *f, wchar_t const *p, va_list v) {
	return f ? vfwscanf(f->stream, p, v) : (errno = EINVAL, WEOF);
}

static inline int xfwscanf(XFILE *f, wchar_t const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfwscanf(f, p, v);
	va_end(v);
	return r;
}

static inline wint_t xfputwc(wint_t c, XFILE *f) {
	return f ? fputwc(c, f->stream) : (errno = EINVAL, WEOF);
}

static inline wint_t xfputws(wchar_t const *s, XFILE *f) {
	return f ? fputws(s, f->stream) : (errno = EINVAL, WEOF);
}

static inline int xvfwprintf(XFILE *f, wchar_t const *p, va_list v) {
	return f ? vfwprintf(f->stream, p, v) : (errno = EINVAL, WEOF);
}

static inline int xfwprintf(XFILE *f, wchar_t const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfwprintf(f, p, v);
	va_end(v);
	return r;
}

//------------------------------------------------------------------------------

#if __STDC_WANT_LIB_EXT1__

static inline int xvfscanf_s(XFILE *f, char const *p, va_list v) {
	return f ? vfscanf_s(f->stream, p, v) : (errno = EINVAL, EOF);
}

static inline int xfscanf_s(XFILE *f, char const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfscanf_s(f, p, v);
	va_end(v);
	return r;
}

static inline int xvfprintf_s(XFILE *f, char const *p, va_list v) {
	return f ? vfprintf_s(f->stream, p, v) : (errno = EINVAL, EOF);
}

static inline int xfprintf_s(XFILE *f, char const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfprintf_s(f, p, v);
	va_end(v);
	return r;
}

static inline int xvfwscanf_s(XFILE *f, wchar_t const *p, va_list v) {
	return f ? vfwscanf_s(f->stream, p, v) : (errno = EINVAL, WEOF);
}

static inline int xfwscanf_s(XFILE *f, wchar_t const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfwscanf_s(f, p, v);
	va_end(v);
	return r;
}

static inline int xvfwprintf_s(XFILE *f, wchar_t const *p, va_list v) {
	return f ? vfwprintf_s(f->stream, p, v) : (errno = EINVAL, WEOF);
}

static inline int xfwprintf_s(XFILE *f, wchar_t const *p, ...) {
	va_list v;
	va_start(v, p);
	int r = xvfwprintf_s(f, p, v);
	va_end(v);
	return r;
}

#endif// __STDC_WANT_LIB_EXT1__

//------------------------------------------------------------------------------

#endif//ndef HOL_XFILE_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_XFILE_H__IMPLEMENTATION
#undef HOL_XFILE_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/holib.h>

//------------------------------------------------------------------------------

XFILE *(xfopen)(char const *name, char const *mode) {
	XFILE *f = NULL;
	if(name) {
		if(!mode) mode  = "ab+";
		bool   is_std   = false;
		bool   is_pipe  = (*name == '=');
		size_t mode_len = strlen(mode);
		size_t name_len = strlen(name+is_pipe);
		f = malloc(sizeof(*f) + name_len+1 + mode_len+1);
		if(f) {
			if(strcmp(name, "tmp:") == 0) {
				f->stream = tmpfile();
				is_pipe   = false;
				is_std    = false;
				mode      = "wb+";
			} else if(strcmp(name, "stdin:") == 0) {
				f->stream = stdin;
				is_pipe   = false;
				is_std    = true;
				mode      = "r";
			} else if(strcmp(name, "stdout:") == 0) {
				f->stream = stdout;
				is_pipe   = false;
				is_std    = true;
				mode      = "w";
			} else if(strcmp(name, "stderr:") == 0) {
				f->stream = stderr;
				is_pipe   = false;
				is_std    = true;
				mode      = "w";
			} else {
				f->stream = is_pipe ? popen(name+1, mode) : fopen(name, mode);
			}
			if(f->stream) {
				f->is_std  = is_std;
				f->is_pipe = is_pipe;
				f->mode    = &f->name[name_len+1];
				strcpy((char *)f->name, name+is_pipe);
				strcpy((char *)f->mode, mode);
			} else {
				free(f);
				f = NULL;
			}
		}
	}
	return f;
}
#define xfopen(xfopen__name,...)  (xfopen)((xfopen__name),__VA_ARGS__+0)

XFILE *xtmpfile(void) {
	return xfopen("tmp:");
}

XFILE *xfclose(XFILE *f) {
	if(f) {
		if(f->is_pipe) {
			pclose(f->stream);
		} else if(!f->is_std) {
			fclose(f->stream);
		}
		free(f);
	}
	return NULL;
}

//------------------------------------------------------------------------------

#endif//ndef HOL_XFILE_H__IMPLEMENTATION
