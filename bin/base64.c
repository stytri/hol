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

#include <hol/holibc.h>

//------------------------------------------------------------------------------

int main(int argc, char **argv) {
	static struct optget options[] = {
		{  0, "usage: %s [OPTION]... [TEXT]...",   NULL },
		{  0, "options:",                NULL },
		{  1, "-h, --help",              "display help" },
		{ 10, "-d, --decode",            "decode TEXT from base 64" },
		{ 11, "-e, --encode",            "encode TEXT to base 64" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	bool decode = true;

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
			case 10:
				decode = true;
				break;
			case 11:
				decode = false;
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

	while(argi < argc) {
		char const *args = argv[argi++];
		size_t n = strlen(args);
		size_t u = decode ? base64declen(n) : base64enclen(n);
		char  *s = calloc(u+1, sizeof(*s));
		if(!s) {
			perror();
			fail();
		}
		if(decode) {
			base64decode(u, s, args, 0);
		} else {
			base64encode(n, args, s);
		}
		puts(s);
		free(s);
	}

	return 0;
}
