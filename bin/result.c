/*
MIT License

Copyright (c) 2024 Tristan Styles

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

//
// Build with https://github.com/stytri/m
//
// ::compile
// :+  $CC $CFLAGS $SMALL-BINARY
// :+      -DNDEBUG=1 -O3 -o $+^ $"* $"!
//
// ::debug
// :+  $CC $CFLAGS
// :+      -Og -g -o $+: $"!
// :&  $DBG -tui $+: $"*
// :&  $RM $+:
//
// ::clean
// :+  $RM rm *.i *.s *.o *.exe
//
// ::CFLAGS
// :+      -Wall -Wextra $WINFLAGS $INCLUDE
//
// ::SMALL-BINARY
// :+      -fmerge-all-constants -ffunction-sections -fdata-sections
// :+      -fno-unwind-tables -fno-asynchronous-unwind-tables
// :+      -Wl,--gc-sections -s
//
// ::INCLUDE!INCLUDE
// :+      -I ../../../inc
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	if(argc > 1) {
		size_t n = argc - 2;
		for(int argi = 1; argi < argc; argi++) {
			n += strlen(argv[argi]);
		}
		char *s = malloc(n + 1), *t = s;
		if(!s) {
			perror(NULL);
			return EXIT_FAILURE;
		}
		for(int argi = 1; argi < argc; argi++) {
			if(argi > 1) *t++ = ' ';
			strcpy(t, argv[argi]);
			t += strlen(t);
		}
		*t = '\0';
		int r = system(s);
		if(r == EXIT_SUCCESS) {
			puts("SUCCESS");
		} else if(r == EXIT_FAILURE) {
			puts("FAILURE");
		} else {
			printf("%d\n", r);
		}
	}
	return 0;
}
