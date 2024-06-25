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

//
// Build with https://github.com/stytri/m
//
// ::compile
// :+  $CC $CFLAGS $XFLAGS $SMALL-BINARY
// :+      -o $+^ $"* $"!
//
// ::clean
// :+  $RM *.i *.s *.o *.exe
//
// ::-
// :+  $CC $CFLAGS
// :+      -Og -g -DDEBUG_$: -o $+: $"!
// :&  $DBG -tui --args $+: $"*
// :&  $RM $+:
//
// ::CFLAGS
// :+      -Wall -Wextra $WINFLAGS $INCLUDE
//
// ::XFLAGS
// :+      -DNDEBUG=1 -O3
//
// ::SMALL-BINARY
// :+      -fmerge-all-constants -ffunction-sections -fdata-sections
// :+      -fno-unwind-tables -fno-asynchronous-unwind-tables
// :+      -Wl,--gc-sections -s
//
// ::windir?WINFLAGS
// :+      -D__USE_MINGW_ANSI_STDIO=1
//
// ::INCLUDE!INCLUDE
// :+      -I ../../../inc
//

//------------------------------------------------------------------------------

#include <hol/holibc.h>
#include <sys/stat.h>

//------------------------------------------------------------------------------

int
main(
	int    argc,
	char **argv
) {
	static struct optget options[] = {
		{  0, "usage: %s [OPTION]... [NAME]...",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{ 10, "-a, --any",               "any" },
		{ 11, "-b, --block",             "only block device" },
		{ 12, "-c, --character",         "only character device" },
		{ 13, "-d, --directory",         "only directory" },
		{ 14, "-f, --file",              "only file" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	int type = 'a';

	int argi = 1;
	while((argi < argc) && (*argv[argi] == '-')) {
		char const *args = argv[argi++];
		char const *argp = NULL;
		do {
			int argn   = argc - argi;
			int params = 0;
			switch(optget(n_options - 2, options + 2, &argp, args, argn, &params)) {
			case 1:
				optuse(n_options, options, argv[0], stdout);
				return 0;
			case 10: type = 'a'; break;
			case 11: type = 'b'; break;
			case 12: type = 'c'; break;
			case 13: type = 'd'; break;
			case 14: type = 'f'; break;
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				fail();
			}
			argi += params;
		} while(argp)
			;
	}

	if(argi == argc) {
		fail();
	}

	while(argi < argc) {
		char const *args = argv[argi++];
		struct stat s;
		if(stat(args, &s) == 0) switch(type) {
		default:
			break;
		case 'a':
			continue;
		case 'b':
			if(S_ISBLK(s.st_mode)) continue;
			break;
		case 'c':
			if(S_ISCHR(s.st_mode)) continue;
			break;
		case 'd':
			if(S_ISDIR(s.st_mode)) continue;
			break;
		case 'f':
			if(S_ISREG(s.st_mode)) continue;
			break;
		}
		fail();
	}

	return 0;
}
