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

//------------------------------------------------------------------------------

#include <hol/holibc.h>

int main(int argc, char **argv) {
	static struct optget options[] = {
		{  0, "usage: %s [options] COMMAND...", NULL },
		{  0, "options:",          NULL },
		{  1, "-h, --help",        "display help" },
		{ 10, "--utc-time",        "(default)" },
#ifdef TIMESTAMP_REALTIME
		{ 11, "--realtime",        NULL },
#endif
#ifdef TIMESTAMP_MONOTIME
		{ 12, "--monotime",        NULL },
#endif
#ifdef TIMESTAMP_CPU_TIME
		{ 13, "--cpu-time",        NULL },
#endif
#ifdef TIMESTAMP_PROCTIME
		{ 14, "--proctime",        NULL },
#endif
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	if(argc == 1) {
		optuse(n_options, options, argv[0], stdout);
		return 0;
	}

	int r = 0;
	int timer = TIMESTAMP_UTC_TIME;

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
			case 10: timer = TIMESTAMP_UTC_TIME; break;
#ifdef TIMESTAMP_REALTIME
			case 11: timer = TIMESTAMP_REALTIME; break;
#endif
#ifdef TIMESTAMP_MONOTIME
			case 12: timer = TIMESTAMP_MONOTIME; break;
#endif
#ifdef TIMESTAMP_CPU_TIME
			case 13: timer = TIMESTAMP_CPU_TIME; break;
#endif
#ifdef TIMESTAMP_PROCTIME
			case 14: timer = TIMESTAMP_PROCTIME; break;
#endif
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				fail();
			}
			argi += params;
		} while(argp)
			;
	}

	size_t n = argc - argi - 1;
	for(int i = argi; i < argc; i++) {
		n += strlen(argv[i]);
	}
	char *s = malloc(n + 1), *t = s;
	if(!s) {
		perror();
		fail();
	}
	for(int i = argi; i < argc; i++) {
		if(i > 1) *t++ = ' ';
		strcpy(t, argv[i]);
		t += strlen(t);
	}
	*t = '\0';
	struct timespec t1, t2;
	timestamp(timer, &t1);
	r = system(s);
	timestamp(timer, &t2);
	time_interval(&t1, &t2, &t2);
	fprint_time_interval(stderr, t2.tv_sec, t2.tv_nsec);
	fputc('\n', stderr);
	return r;
}
