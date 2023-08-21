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

//------------------------------------------------------------------------------

static char const *
get_pattern(
	char const *cs
) {
	size_t n = strlen(cs);
	char  *s = calloc(1, n+1);
	if(s) {
		bool esc = false;
		for(char *t = s; *cs; cs++) {
			if(esc) {
				switch(*cs) {
				default : *t++ = *cs ; break;
				case 'n': *t++ = '\n'; break;
				case 't': *t++ = '\t'; break;
				}
				esc = false;
			} else if(*cs == '\\') {
				esc = true;
			} else {
				*t++ = *cs;
			}
		}
		return s;
	}
	perror();
	fail();
}

static void
fill(
	char const *arg1,
	char const *arg2
) {
	char       *s, *t;
	size_t      size    = strtozs(arg1, &t, 0);
	char const *pattern = get_pattern(arg2);
	size_t      length  = strlen(pattern);
	s = calloc((*t ? length : 1), size+1);
	if(s) {
		memfill(s, (*t ? (length * size) : size), pattern, length);
		fputs(s, stdout);
		free(s);
		return;
	}
	perror();
	fail();
}

//------------------------------------------------------------------------------

int
main(
	int    argc,
	char **argv
) {
	if(argc > 1) {
		do {
			if(argc > 2) {
				fill(argv[1], argv[2]);
				argv += 2;
				argc -= 2;
			} else {
				fill("1x", argv[1]);
				argv += 1;
				argc -= 1;
			}
		} while(argc > 1)
			;
	} else {
		puts("usage: fill [[SIZE] PATTERN]...");
	}
	return 0;
}
