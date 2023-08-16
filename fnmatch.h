#ifndef HOL_FNMATCH_H_INCLUDED
#define HOL_FNMATCH_H_INCLUDED
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

#include <wchar.h>

//------------------------------------------------------------------------------

#ifdef _WIN32
#define FNM_CASE    (1 << 0) /* compare case                               */
#define FNM_NOCASE  (0 << 0) /* ignore case                                */
#else
#define FNM_CASE    (0 << 0) /* compare case                               */
#define FNM_NOCASE  (1 << 0) /* ignore case                                */
#endif
#define FNM_NOPATH  (1 << 1) /* '/' may only be matched explicitly         */
#define FNM_PERIOD  (1 << 2) /* initial '.' may only be matched explicitly */
#define FNM_ESCAPE  (1 << 3) /* '\' protects meta-characters               */
#define FNM_NOALT   (1 << 4) /* disable alternation                        */

//------------------------------------------------------------------------------

extern int fnmatch(char const *file, char const *pattern, int flags);
#define fnmatch(File,Pattern,...)  (fnmatch)(File, Pattern, __VA_ARGS__+0)

extern int wfnmatch(wchar_t const *file, wchar_t const *pattern, int flags);
#define wfnmatch(File,Pattern,...)  (wfnmatch)(File, Pattern, __VA_ARGS__+0)

//------------------------------------------------------------------------------

#endif//ndef HOL_FNMATCH_H_INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_FNMATCH_H__IMPLEMENTATION
#undef HOL_FNMATCH_H__IMPLEMENTATION

//------------------------------------------------------------------------------

#include <hol/xtdlib.h>
#include <hol/xtring.h>
#include <ctype.h>

//------------------------------------------------------------------------------

static inline int
fnmatch__pathseparator(
	int c
) {
	return (c == '/')
#ifdef _WIN32
		|| (c == '\\')
#endif
		;
}

int
(fnmatch)(
	char const *file,
	char const *pattern,
	int         flags
) {
	char const *cp     = pattern;
	char const *cf     = file;
#ifdef _WIN32
	int  const  nocase = (0 == (flags & FNM_CASE));
#else
	int  const  nocase = (0 != (flags & FNM_NOCASE));
#endif
	int  const  nopath = (0 != (flags & FNM_NOPATH));
	int  const  period = (0 != (flags & FNM_PERIOD));
	int  const  escape = (0 != (flags & FNM_ESCAPE));
	int  const  noext  = (0 != (flags & FNM_NOALT));

	if(period && ((*cp == '.') ? (*cf != '.') : (*cf == '.'))) {
		return 0;
	}

	for(int cc; (cc = *cp) != '\0'; ) switch(cc) {
		int         invert;
		char const *cb, *ct;
		size_t      nc;
	case '[':
		if(*cf == '\0') {
			return 0;
		}
		++cp;
		invert = (*cp == '^');
		cp += invert;
		cb  = cp;
		if((cc = *cp) == ']'){  /* an immediately following ']' specifies ']' */
			cc = *++cp;
		}
		while((cc != '\0') && (cc != ']')) {
			cc = *++cp;
		}
		if(cc == '\0') { /* no closing ']' so match '[' */
			if(*cf != '[') {
				return 0;
			}
			cp = cb;
			cp -= invert;
			break;
		}
		cc = nocase ? toupper(*cf) : *cf;
		ct = cp;
		for(cp = cb; cp < ct; ++cp) {
			int lo = nocase ? toupper(*cp) : *cp;
			int hi = nocase ? toupper(*(cp + 2)) : *(cp + 2);
			if((*(cp + 1) == '-')
					&& ((isdigit(lo) && isdigit(hi))
						|| (isupper(lo) && isupper(hi))
						|| (islower(lo) && islower(hi)))
			) {
				cp += 2;
				if(isdigit(lo) && !isdigit(cc)) {
					continue;
				}
				if(isupper(lo) && !isupper(cc)) {
					continue;
				}
				if(islower(lo) && !islower(cc)) {
					continue;
				}
				if((lo > hi) ? ((cc > lo) || (cc < hi)) : ((cc < lo) || (cc > hi))) {
					continue;
				}
			} else if(cc != lo) {
				continue;
			}
			break;
		}
		if(invert ? (cp < ct) : (cp == ct)) {
			return 0;
		}
		cp = ct + 1;
		++cf;
		break;
	case '(':
		if(noext || (cc != '(')) {
			goto match_single_char;
		}
		++cp;
		invert = (*cp == '^');
		cp += invert;
		cb  = cp;
		cc  = *cp;
		while((cc != '\0') && (cc != ')')) {
			cc = *++cp;
		}
		if(cc == '\0') { /* no closing ')' so match '(' */
			if(*cf != '(') {
				return 0;
			}
			cp = cb;
			cb -= invert;
			break;
		}
		ct = cp;
		nc = 0;
		for(cp = cb; cp < ct; nc = 0) {
			while((cp < ct) && (*cp != ':')) {
				++cp, ++nc;
			}
			if(nocase ? (!strncasecmp(cf, cp - nc, nc)) : (!strncmp(cf, cp - nc, nc))) {
				break;
			}
			if(cp < ct) {
				++cp;
			}
		}
		if(invert ? (nc > 0) : (nc == 0)) {
			return 0;
		}
		cp = ct + 1;
		cf += nc;
		if(!invert || (*cp != '\0')) {
			break;
		}
		--cp;
		nobreak;
	case '*':
		/* condense sequential '?' and '*'
		 */
		for(cc = *++cp; (cc == '*') || (cc == '?'); cc = *++cp) {
			if(cc == '?') {
				if((*cf == '\0')
					|| (nopath && fnmatch__pathseparator(*cf))
					|| (period && (*cf == '.') && fnmatch__pathseparator(*(cf - 1)))
				) {
					return 0;
				}
				++cf;
			}
		}
		cb = cf;
		while(!((*cf == '\0')
			 || (nopath && fnmatch__pathseparator(*cf))
			 || (period && (*cf == '.') && fnmatch__pathseparator(*(cf - 1))))
		) {
			++cf;
		}
		++cf;
		do {
			--cf;
			if((fnmatch)(cf, cp, flags & ~FNM_PERIOD)) {
				return 1;
			}
		} while(cf > cb)
			;
		return 0;
	case '?':
		if((*cf == '\0')
			|| (nopath && fnmatch__pathseparator(*cf))
			|| (period && (*cf == '.') && fnmatch__pathseparator(*(cf - 1)))
		) {
			return 0;
		}
		++cp;
		++cf;
		break;
	case '\\':
		if(escape && (*(cp + 1) != '\0')) {
			cc = *++cp;
		}
		nobreak;
	default:
match_single_char:
		if((*cf == '\0') || (nocase ? (toupper(*cf) != toupper(cc)) : (*cf != cc))) {
			return 0;
		}
		++cp;
		++cf;
		break;
	}

	return (*cf == '\0');
}

//------------------------------------------------------------------------------

static inline int
wcsncasecmp(
	wchar_t const *cs,
	wchar_t const *ct,
	size_t         n
) {
	if(n > 0) {
		wint_t sc = towlower(*cs);
		wint_t tc = towlower(*ct);
		while(sc && (sc == tc)) {
			if(--n > 0) {
				cs++, ct++;
				sc = towlower(*cs);
				tc = towlower(*ct);
				continue;
			}
			return 0;
		}
		return sc - tc;
	}
	return 0;
}

static inline int
wfnmatch__pathseparator(
	wint_t c
) {
	return (c == L'/')
#ifdef _WIN32
		|| (c == L'\\')
#endif
		;
}

int
(wfnmatch)(
	wchar_t const *file,
	wchar_t const *pattern,
	int            flags
) {
	wchar_t const *cp     = pattern;
	wchar_t const *cf     = file;
#ifdef _WIN32
	int     const  nocase = (0 == (flags & FNM_CASE));
#else
	int     const  nocase = (0 != (flags & FNM_NOCASE));
#endif
	int     const  nopath = (0 != (flags & FNM_NOPATH));
	int     const  period = (0 != (flags & FNM_PERIOD));
	int     const  escape = (0 != (flags & FNM_ESCAPE));
	int     const  noext  = (0 != (flags & FNM_NOALT));

	if(period && ((*cp == L'.') ? (*cf != L'.') : (*cf == L'.'))) {
		return 0;
	}

	for(wint_t cc; (cc = *cp) != L'\0'; ) switch(cc) {
		int            invert;
		wchar_t const *cb, *ct;
		size_t         nc;
	case L'[':
		if(*cf == L'\0') {
			return 0;
		}
		++cp;
		invert = (*cp == L'^');
		cp += invert;
		cb  = cp;
		if((cc = *cp) == L']'){  /* an immediately following ']' specifies ']' */
			cc = *++cp;
		}
		while((cc != L'\0') && (cc != L']')) {
			cc = *++cp;
		}
		if(cc == L'\0') { /* no closing ']' so match '[' */
			if(*cf != L'[') {
				return 0;
			}
			cp = cb;
			cp -= invert;
			break;
		}
		cc = nocase ? towupper(*cf) : *cf;
		ct = cp;
		for(cp = cb; cp < ct; ++cp) {
			int lo = nocase ? towupper(*cp) : *cp;
			int hi = nocase ? towupper(*(cp + 2)) : *(cp + 2);
			if((*(cp + 1) == L'-')
					&& ((iswdigit(lo) && iswdigit(hi))
						|| (iswupper(lo) && iswupper(hi))
						|| (iswlower(lo) && iswlower(hi)))
			) {
				cp += 2;
				if(iswdigit(lo) && !iswdigit(cc)) {
					continue;
				}
				if(iswupper(lo) && !iswupper(cc)) {
					continue;
				}
				if(iswlower(lo) && !iswlower(cc)) {
					continue;
				}
				if((lo > hi) ? ((cc > lo) || (cc < hi)) : ((cc < lo) || (cc > hi))) {
					continue;
				}
			} else if(cc != lo) {
				continue;
			}
			break;
		}
		if(invert ? (cp < ct) : (cp == ct)) {
			return 0;
		}
		cp = ct + 1;
		++cf;
		break;
	case L'(':
		if(noext || (cc != L'(')) {
			goto match_single_char;
		}
		++cp;
		invert = (*cp == L'^');
		cp += invert;
		cb  = cp;
		cc  = *cp;
		while((cc != L'\0') && (cc != L')')) {
			cc = *++cp;
		}
		if(cc == L'\0') { /* no closing ')' so match '(' */
			if(*cf != L'(') {
				return 0;
			}
			cp = cb;
			cb -= invert;
			break;
		}
		ct = cp;
		nc = 0;
		for(cp = cb; cp < ct; nc = 0) {
			while((cp < ct) && (*cp != L':')) {
				++cp, ++nc;
			}
			if(nocase ? (!wcsncasecmp(cf, cp - nc, nc)) : (!wcsncmp(cf, cp - nc, nc))) {
				break;
			}
			if(cp < ct) {
				++cp;
			}
		}
		if(invert ? (nc > 0) : (nc == 0)) {
			return 0;
		}
		cp = ct + 1;
		cf += nc;
		if(!invert || (*cp != L'\0')) {
			break;
		}
		--cp;
		nobreak;
	case '*':
		/* condense sequential '?' and '*'
		 */
		for(cc = *++cp; (cc == L'*') || (cc == L'?'); cc = *++cp) {
			if(cc == L'?') {
				if((*cf == '\0')
					|| (nopath && wfnmatch__pathseparator(*cf))
					|| (period && (*cf == L'.') && wfnmatch__pathseparator(*(cf - 1)))
				) {
					return 0;
				}
				++cf;
			}
		}
		cb = cf;
		while(!((*cf == L'\0')
			 || (nopath && wfnmatch__pathseparator(*cf))
			 || (period && (*cf == L'.') && wfnmatch__pathseparator(*(cf - 1))))
		) {
			++cf;
		}
		++cf;
		do {
			--cf;
			if((wfnmatch)(cf, cp, flags & ~FNM_PERIOD)) {
				return 1;
			}
		} while(cf > cb)
			;
		return 0;
	case L'?':
		if((*cf == '\0')
			|| (nopath && wfnmatch__pathseparator(*cf))
			|| (period && (*cf == L'.') && wfnmatch__pathseparator(*(cf - 1)))
		) {
			return 0;
		}
		++cp;
		++cf;
		break;
	case L'\\':
		if(escape && (*(cp + 1) != L'\0')) {
			cc = *++cp;
		}
		nobreak;
	default:
match_single_char:
		if((*cf == L'\0') || (nocase ? (towupper(*cf) != towupper(cc)) : (*cf != cc))) {
			return 0;
		}
		++cp;
		++cf;
		break;
	}

	return (*cf == L'\0');
}

//------------------------------------------------------------------------------

#endif//def HOL_FNMATCH_H__IMPLEMENTATION
