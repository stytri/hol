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
#include <stdio.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

static volatile sig_atomic_t gSignal = 0;

static void
signal_handler(
	int signal
) {
	gSignal = signal;
}

//------------------------------------------------------------------------------

struct btee_file {
	FILE       *stream;
	bool        is_pipe;
	char const *name;
};

#ifndef NDEBUG
int
main(
	int   argc,
	char *argv__actual[]
) {
	char *argv[] = {
		argv__actual[0],
		"-I",
		NULL
	};
	argc = (sizeof(argv) / sizeof(argv[0])) - 1;
#else
int
main(
	int   argc,
	char *argv[]
) {
#endif
	static struct optget options[] = {
		{  0, "usage: %s [options]... [FILE]...", NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{  2, "-a, --append",            "append to FILEs" },
		{  9, "-l, --line-buffered",     "line buffered" },
		{  3, "-b, --buffer-size SIZE",  "set buffer SIZE" },
		{  4, "-n, --no-standard-out",   "no output to standard out" },
		{  5, "-e, --no-error-messages", "no error messages" },
		{  6, "-c, --close-on-error",    "close output on error" },
		{  7, "-1, --halt-first-error",  "halt on first output error" },
		{  8, "-0, --halt-no-outputs",   "halt if no active outputs" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	if(argc == 1) {
		optuse(n_options, options, argv[0], stdout);
		return 0;
	}

	int exit_code = EXIT_SUCCESS;

	size_t sizeof_buffer     = BUFSIZE;
	bool   line_buffered     = false;
	bool   append_to_file    = false;
	bool   ignore_interrupts = false;
	bool   standard_out      = true;
	bool   error_messages    = true;
	bool   close_on_error    = false;
	bool   halt_first_error  = false;
	bool   halt_no_output    = false;

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
			case 2:
				append_to_file = true;
				break;
			case 3:
				sizeof_buffer = streval(argv[argi], NULL, 0);
				if(sizeof_buffer == 0) sizeof_buffer = BUFSIZE;
				break;
			case 4:
				standard_out = false;
				break;
			case 5:
				error_messages = false;
				break;
			case 6:
				close_on_error = true;
				break;
			case 7:
				halt_first_error = true;
				break;
			case 8:
				halt_no_output = true;
				break;
			case 9:
				line_buffered = true;
				break;
			case 99:
				ignore_interrupts = true;
				break;
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				return EXIT_FAILURE;
			}
			argi += params;
		} while(argp)
			;
	}

	size_t            nfiles = ((argi > 0) && (argc > argi)) ? (argc - argi) + 1 : 0;
	struct btee_file *file   = calloc(nfiles, sizeof(struct btee_file));
	uint8_t          *buffer = malloc(sizeof_buffer);
	if(!(file || buffer)) {
		if(error_messages) perror();
		exit(EXIT_FAILURE);
	}

	if(ignore_interrupts) {
		signal(SIGINT, SIG_IGN);
	} else {
		signal(SIGINT, signal_handler);
	}
#ifdef _WIN32
	if(!line_buffered) {
		_setmode(_fileno(stdin), _O_BINARY);
		_setmode(_fileno(stdout), _O_BINARY);
	}
#endif
	file[0].name    = NULL;
	file[0].is_pipe = false;
	file[0].stream  = standard_out ? stdout : NULL;
	for(size_t v = argi, i = 1; i < nfiles; i++) {
		char *args = argv[v++];
		file[i].is_pipe = *args == '=';
		if(file[i].is_pipe) {
			file[i].stream = NULL;
			file[i].name   = ++args;
			file[i].stream = popen(file[i].name, line_buffered ? "w" : "wb");
		} else {
			file[i].name   = args;
			file[i].stream = fopen(file[i].name, append_to_file ? (line_buffered ? "a" : "ab") : (line_buffered ? "w" : "wb"));
		}
		if(!file[i].stream) {
			if(error_messages) perror(file[i].name);
			if(halt_first_error) {
				exit_code = EXIT_FAILURE;
				break;
			}
		}
	}

	while(!gSignal && (exit_code == EXIT_SUCCESS)) {
		size_t n;
		if(line_buffered) {
			char const *cs = fgets((char *)buffer, sizeof_buffer, stdin);
			n = cs ? strlen((char *)buffer) : 0;
		} else {
			n = fread(buffer, 1, sizeof_buffer, stdin);
		}
		if(n > 0) {
			size_t o = 0;
			for(size_t i = 0; i < nfiles; i++) {
				if(file[i].stream) {
					if(line_buffered) {
						fputs((char *)buffer, file[i].stream);
					} else {
						fwrite(buffer, 1, n, file[i].stream);
					}
					if(ferror(file[i].stream)) {
						if(error_messages) perror(file[i].name);
						if(halt_first_error) {
							exit_code = EXIT_FAILURE;
							break;
						}
						if(close_on_error) {
							if(file[i].is_pipe) {
								pclose(file[i].stream);
							} else if(i > 0) {
								fclose(file[i].stream);
							}
							file[i].stream = NULL;
							continue;
						}
						clearerr(file[i].stream);
					}
					o++;
				}
			}
			if((o == 0) && halt_no_output) {
				exit_code = EXIT_FAILURE;
				break;
			}
		}
		if(ferror(stdin)) {
			if(1
#ifdef EPIPE
				&& (errno != EPIPE)
#endif
			) {
				if(error_messages) perror();
			}
			exit_code = EXIT_FAILURE;
			break;
		}
		if(feof(stdin)) break;
	}

	for(size_t i = 1; i < nfiles; i++) {
		if(file[i].stream) {
			if(file[i].is_pipe) {
				pclose(file[i].stream);
			} else {
				fclose(file[i].stream);
			}
		}
	}

	free(buffer);
	free(file);

	return exit_code;
}
