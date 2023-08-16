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
#include <setjmp.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

static volatile sig_atomic_t gSignal           = 0;
static volatile sig_atomic_t gSignalJumpEnable = 0;
static          jmp_buf      gSignalJumpContext;

static void
signal_handler(
	int signal
) {
	gSignal = signal;
	if(gSignalJumpEnable) {
		gSignalJumpEnable = 0;
		longjmp(gSignalJumpContext, gSignal);
	}
}

//------------------------------------------------------------------------------

static char const *
copy_to_temporary_file(
	char const *cs,
	int         retries
) {
	char *buf = malloc(BUFSIZE);
	if(!buf) {
		perror();
		return NULL;
	}

	FILE *fin = fopen(cs, "rb");
	if(!fin) {
		perror(cs);
		free(buf);
		return NULL;
	}

	char const *ct;
	FILE       *fout;
	for(;; free((void *)ct)) {
		ct = strdup(tmpnam(NULL));
		if(gSignal || !ct) {
			perror();
			fclose(fin);
			free(buf);
			return NULL;
		}

		fout = fopen(ct, "wb");
		if(fout) break;

		retries -= (retries > 0);
		if(gSignal || (retries == 0)) {
			perror(ct);
			free((void *)ct);
			fclose(fin);
			free(buf);
			return NULL;
		}
	}

	for(size_t n;;) {
		n = fread(buf, 1, BUFSIZE, fin);
		if(n > 0) {
			fwrite(buf, 1, n, fout);
		}
		if(gSignal || ferror(fin) || ferror(fout)) {
			if(ferror(fin)) {
				perror(cs);
			}
			if(ferror(fout)) {
				perror(ct);
			}
			fclose(fout);
			free((void *)ct);
			fclose(fin);
			free(buf);
			return NULL;
		}
		if(feof(fin)) {
			break;
		}
	}

	fclose(fout);
	fclose(fin);
	free(buf);

	return ct;
}

static int
run_as_temporary(
	int          argc,
	char const **argv,
	int          retries,
	bool         report_info
) {
	argv[0] = copy_to_temporary_file(argv[0], retries);
	if(!argv[0]) {
		fail();
	}

	size_t n = 0;
	for(int argi = 0; argi < argc; argi++) {
		n += strlen(argv[argi]) + 1;
	}
	char *s = malloc(n);
	if(!s) {
		perror();
		remove(argv[0]);
		fail();
	}

	size_t o = 0;
	for(int argi = 0; argi < argc; argi++) {
		if(argi > 0) {
			s[o++] = ' ';
		};
		n = strlen(argv[argi]);
		strcpy(s+o, argv[argi]);
		o += n;
	}
	s[o] = '\0';

	int e = 0;

	if((setjmp(gSignalJumpContext) == 0) && !gSignal) {
		if(report_info) {
			fprintf(stderr, "%s\n", s);
		}
		gSignalJumpEnable = 1;
		e = system(s);
		gSignalJumpEnable = 0;
	}

	if(remove(argv[0])) {
		perror(argv[0]);
	}
	free((void *)argv[0]);
	free(s);

	return e;
}

//------------------------------------------------------------------------------

int
main(
	int    argc,
	char **argv
) {
	static struct optget options[] = {
		{  0, "usage: %s [OPTION]... CMD [CMDOPTION]...", NULL },
		{  0, "Run As Temporary:", NULL },
		{ -1, "Copies the CMD file to a temporary file, which is then executed.", NULL },
		{  0, "", NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-r, --retries COUNT",     "set number of retries" },
		{ 98, "-E, --info-to-stderr",    "output information to stderr" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	int  retries           = TMP_MAX;
	bool report_info       = false;
	bool ignore_interrupts = false;

	int argi = 1;
	while((argi < argc) && (*argv[argi] == '-')) {
		char const *args = argv[argi++];
		char const *argp = NULL;
		do {
			int argn   = argc - argi;
			int params = 0;
			switch(optget(n_options - 5, options + 5, &argp, args, argn, &params)) {
			case 1:
				optuse(n_options, options, argv[0], stdout);
				return 0;
			case 2:
				retries = atoi(argv[argi]);
				break;
			case 98:
				report_info = true;
				break;
			case 99:
				ignore_interrupts = true;
				break;
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				fail();
			}
			argi += params;
		} while(argp)
			;
	}

	if(argi >= argc) {
		optuse(n_options, options, argv[0], stdout);
		return 0;
	}

	if(ignore_interrupts) {
		signal(SIGINT, SIG_IGN);
	} else {
		signal(SIGINT, signal_handler);
	}
#ifdef _WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	int          cmdi = 0;
	int          cmdc = argc - argi;
	char const **cmdv = calloc(cmdc + 1, sizeof(char *));
	if(!cmdv) {
		perror();
		fail();
	}

	while(argi < argc) {
		cmdv[cmdi++] = argv[argi++];
	}
	cmdv[cmdc] = NULL;

	int e = run_as_temporary(cmdc, cmdv, retries, report_info);

	free(cmdv);

	return e;
}
