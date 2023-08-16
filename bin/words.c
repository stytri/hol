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
#include <errno.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

//------------------------------------------------------------------------------

struct word {
	int   n;
	char *s;
};

static int wordcmp(void const *p, void const *k, size_t z) {
	struct word const *s = p;
	struct word const *t = k;
	return mcompare(s->s, s->n, t->s, t->n);
	(void)z;
}

struct word_list {
	size_t            n;
	size_t            z;
	struct word     **a;
	struct word_list *p;
};

static struct word_list *
new_word_list(
	struct word_list *p
) {
	struct word_list *l = malloc(sizeof(*l));
	if(likely(l)) {
		l->n = 0;
		l->z = 0;
		l->a = NULL;
		l->p = p;
	}
	return l;
}

static struct word_list *
del_word_list(
	struct word_list *l
) {
	struct word_list *p = NULL;
	if(l) {
		p = l->p;
		while(l->n-- > 0) {
			free(l->a[l->n]);
		}
		l->z = 0;
		free(l->a), l->a = NULL;
		free(l);
	}
	return p;
}

static int
add_word_list(
	struct word *w,
	void        *p
) {
	struct word_list *l = p;
	if(l->n == l->z) {
		l->z = (l->z + 1) + (l->z >> 1);
		void *p = realloc(l->a, l->z * sizeof(*l->a));
		if(unlikely(!p)) return 0;
		l->a = p;
	}
	l->a[l->n++] = w;
	return 1;
}

//------------------------------------------------------------------------------

static size_t       insertion_count = 0;
static size_t       deletion_count  = 0;
static struct hampt word_table      = HAMPT();
static size_t       word_count      = 0;
static size_t       collisions      = 0;
static size_t       repetitions     = 0;
static int          max_depth       = 0;
static size_t       max_chain       = 0;

//------------------------------------------------------------------------------

struct context {
	uint64_t mask;
	int      minlen;
	int      maxlen;
};

static int
add_word(
	struct word *w,
	void        *p
) {
	struct context const *ctx = p;

	if((w->n < ctx->minlen) || (w->n > ctx->maxlen)) {
		return 0;
	}

	insertion_count++;

	uint64_t h  = memhash(w->s, w->n) & ctx->mask;
	void   **qp = hampt_insert(&word_table, h);
	void    *q  = *qp;
	if(q) {
		struct spa *a  = NULL;
		if(is_tagged_pointer(q)) {
			a = untag_pointer(q);
			void **pp = spa_insert(&a, wordcmp, w, 0);
			if(*pp) {
				repetitions++;
				return 0;
			}
			*pp = w;
		} else {
			if(wordcmp(q, w, 0) == 0) {
				repetitions++;
				return 0;
			}
			*spa_insert(&a, wordcmp, q, 0) = q;
			*spa_insert(&a, wordcmp, w, 0) = w;
		}
		collisions++;
		*qp = tag_pointer(a);
	} else {
		*qp = w;
	}
	word_count++;
	return 1;
}

static int
del_word(
	struct word *w,
	void        *p
) {
	struct context const *ctx = p;

	if((w->n < ctx->minlen) || (w->n > ctx->maxlen)) {
		return 0;
	}

	deletion_count++;

	uint64_t h  = memhash(w->s, w->n) & ctx->mask;
	void   **qp = hampt_lookup(&word_table, h);
	if(qp) {
		void *q = *qp;
		if(is_tagged_pointer(q)) {
			struct spa *a = untag_pointer(q);
			w = spa_remove(&a, wordcmp, w, 0);
			if(w) {
				if(!a) hampt_remove(&word_table, h);
				else if(a->n > 1) *qp = tag_pointer(a);
				else *qp = a->p[0], free(a);
				word_count--;
				return -1;
			}
		} else {
			if(wordcmp(q, w, 0) == 0) {
				hampt_remove(&word_table, h);
				word_count--;
				return -1;
			}
		}
	}
	return 0;
}

static void freewords(void *p) {
	if(!is_tagged_pointer(p)) free(p);
	else {
		struct spa *a = untag_pointer(p);
		spa_free(&a, free);
	}
}

static void
scanwords(
	int (*is_ctype)(int c),
	int (*action  )(struct word *w, void *),
	void *p
) {
	for(int c;;) {
		while(((c = getchar()) != EOF) && !is_ctype(c))
			;
		if(c == EOF) break;
		if((is_ctype == isident) && isdigit(c)) {
			while(((c = getchar()) != EOF) && is_ctype(c))
				;
			if(c == EOF) break;
			continue;
		}
		size_t       z = 0;
		struct word *w = malloc(sizeof(*w)+1);
		if(unlikely(!w)) return;
		w->s = (char *)(w+1);
		w->n = 0;
		do {
			if((size_t)w->n == z) {
				void *b = realloc(w, sizeof(*w)+(z=(z+1)+(z>>2))+1);
				if(unlikely(!b)) {
					free(w);
					return;
				}
				w = b;
				w->s = (char *)(w+1);
			}
			w->s[w->n++] = c;
		} while(((c = getchar()) != EOF) && is_ctype(c))
			;
		w->s[w->n] = '\0';
		if(action(w, p) <= 0) free(w);
		if(c == EOF) break;
		ungetc(c, stdin);
	}
}

static void
print_indent(
	int d
) {
	static char const tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	putstr(&tabs[16-d]);
}

static int print_word(
	void *p,
	void *c,
	int   d
) {
	(void)c;
	if(max_depth < d) max_depth = d;
	if(!is_tagged_pointer(p)) {
		if(max_chain < 1) max_chain = 1;
		struct word *w = p;
		printf("%*s\n", (int)w->n, w->s);
		return 0;
	}
	struct spa *a = untag_pointer(p);
	if(max_chain < a->n) max_chain = a->n;
	for(size_t i = 0; i < a->n; i++) {
		struct word *w = a->p[i];
		printf("%*s\n", (int)w->n, w->s);
	}
	return 0;
}

static int print_indented_word(
	void *p,
	void *c,
	int   d
) {
	(void)c;
	if(max_depth < d) max_depth = d;
	if(!is_tagged_pointer(p)) {
		if(max_chain < 1) max_chain = 1;
		struct word *w = p;
		print_indent(d); printf("%*s\n", (int)w->n, w->s);
		return 0;
	}
	struct spa *a = untag_pointer(p);
	if(max_chain < a->n) max_chain = a->n;
	for(size_t i = 0; i < a->n; i++) {
		struct word *w = a->p[i];
		print_indent(d); printf("%*s\n", (int)w->n, w->s);
	}
	return 0;
}

static int find_maximum_depth(
	void *p,
	void *c,
	int   d
) {
	(void)c;
	if(max_depth < d) max_depth = d;
	if(!is_tagged_pointer(p)) {
		if(max_chain < 1) max_chain = 1;
		return 0;
	}
	struct spa *a = untag_pointer(p);
	if(max_chain < a->n) max_chain = a->n;
	return 0;
}

//------------------------------------------------------------------------------

static volatile sig_atomic_t gSignal = 0;

static void
signal_handler(
	int signal
) {
    gSignal = signal;
}

//------------------------------------------------------------------------------

#ifndef NDEBUG
int
main(
	int   argc,
	char *argv__actual[]
) {
	char *argv[] = {
		argv__actual[0],
		"-MTqs",
		"allwords.txt",
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
		{  0, "usage: %s [options] [FILE]", NULL },
		{  0, "options:",          NULL },
		{  1, "-h, --help",        "display help" },
		{  2, "-o, --output FILE", "output to FILE" },

		{ 80, "-T, --utc-time",    "timed execution" },
#ifdef TIMESTAMP_REALTIME
		{ 81, "    --realtime",    NULL },
#endif
#ifdef TIMESTAMP_MONOTIME
		{ 82, "    --monotime",    NULL },
#endif
#ifdef TIMESTAMP_CPU_TIME
		{ 83, "    --cpu-time",    NULL },
#endif
		{  6, "-M, --in-memory",   "pre-load FILE words into memory" },
		{ 99, "-I, --ignore-interrupts", "ignore interrupt signals" },

		{ 10, "-i, --ident",       "filter for identifier characters (default)" },
		{ 11, "-p, --print",       "filter for printing characters" },
		{ 12, "-g, --graph",       "filter for graphic characters" },
		{ 13, "-n, --alnum",       "filter for alpha-numeric characters" },
		{ 14, "-a, --alpha",       "filter for alphabetic characters" },
		{ 15, "-z, --size MIN MAX","filter for size between MIN and MAX" },

		{ 24, "-8, --64-bit",      "use 8-byte/64-bit hash mask (default)" },
		{ 23, "-6, --48-bit",      "use 6-byte/48-bit hash mask" },
		{ 22, "-4, --32-bit",      "use 4-byte/32-bit hash mask" },
		{ 21, "-2, --16-bit",      "use 2-byte/16-bit hash mask" },

		{ 31, "-f, --flat",        "output unindented word list (default)" },
		{ 32, "-t, --tree",        "output indented word list" },
		{ 33, "-s, --stats",       "output some statistics" },
		{ 30, "-q, --quiet",       "do not output words" },
	};
	static size_t const n_options = (sizeof(options) / sizeof(options[0]));

	if(argc == 1) {
		optuse(n_options, options, argv[0], stdout);
		return 0;
	}

	struct word_list *word_list                        = NULL;
	bool              inmem                            = false;
	int               timed                            = NO_TIMESTAMP;
	bool              ignore_interrupts                = false;
	int             (*walker  )(void *, void *, int)   = print_word;
	int             (*is_ctype)(int)                   = isident;
	int             (*action  )(struct word *, void *) = add_word;
	bool              stats                            = false;
	struct context    ctx = {
		.mask   = UINT64_C(~0),
		.minlen = INT_MIN,
		.maxlen = INT_MAX,
	};

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
				if(!freopen(argv[argi], "w", stdout)) {
					perror(argv[argi]);
					fail();
				}
				break;
			case 6: inmem = true; break;
			case 10: is_ctype = isident; break;
			case 11: is_ctype = isprint; break;
			case 12: is_ctype = isgraph; break;
			case 13: is_ctype = isalnum; break;
			case 14: is_ctype = isalpha; break;
			case 15:
				ctx.minlen = atoi(argv[argi]);
				ctx.maxlen = atoi(argv[argi+1]);
				break;
			case 24: ctx.mask = UINT64_C(~0);       break;
			case 23: ctx.mask = UINT64_C(~0) >> 16; break;
			case 22: ctx.mask = UINT64_C(~0) >> 32; break;
			case 21: ctx.mask = UINT64_C(~0) >> 48; break;
			case 30: walker = find_maximum_depth;  break;
			case 31: walker = print_word;          break;
			case 32: walker = print_indented_word; break;
			case 33: stats = true; break;
			case 80: timed = TIMESTAMP_UTC_TIME; break;
#ifdef TIMESTAMP_REALTIME
			case 81: timed = TIMESTAMP_REALTIME; break;
#endif
#ifdef TIMESTAMP_MONOTIME
			case 82: timed = TIMESTAMP_MONOTIME; break;
#endif
#ifdef TIMESTAMP_CPU_TIME
			case 83: timed = TIMESTAMP_CPU_TIME; break;
#endif
			case 99: ignore_interrupts = true; break;
			default:
				errorf("invalid option: %s", args);
				optuse(n_options, options, argv[0], stderr);
				fail();
			}
			argi += params;
		} while(argp)
			;
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
	struct timespec insertion_time = { .tv_sec = 0, .tv_nsec = 0 };
	struct timespec deletion_time  = { .tv_sec = 0, .tv_nsec = 0 };
	struct timespec rt1, rt2;
	timestamp(timed, &rt1);

	for(char const *file = NULL; !gSignal && (argi < argc); ) {
		file = argv[argi++];
		if(*file == '-') {
			file++;
			action = del_word;
		} else if(*file == '+') {
			file++;
			action = add_word;
		}
		if(!freopen(file, "r", stdin)) {
			perror(file);
			fail();
		}

		struct timespec t1, t2;
		if(inmem) {
			word_list = new_word_list(word_list);
			scanwords(is_ctype, add_word_list, word_list);
			size_t const N = word_list->n;
			timestamp(timed, &t1);
			for(size_t i = 0; i < N; i++) {
				struct word *w = word_list->a[i];
				action(w, &ctx);
			}
			timestamp(timed, &t2);
		} else {
			timestamp(timed, &t1);
			scanwords(is_ctype, action, &ctx);
			timestamp(timed, &t2);
		}
		time_interval(&t1, &t2, &t2);
		if(action == add_word) {
			insertion_time.tv_sec += t2.tv_sec;
			insertion_time.tv_nsec += t2.tv_nsec;
			if(insertion_time.tv_nsec >= 1000000000l) {
				insertion_time.tv_nsec -= 1000000000l;
				insertion_time.tv_sec += 1;
			}
		} else {
			deletion_time.tv_sec += t2.tv_sec;
			deletion_time.tv_nsec += t2.tv_nsec;
			if(deletion_time.tv_nsec >= 1000000000l) {
				deletion_time.tv_nsec -= 1000000000l;
				deletion_time.tv_sec += 1;
			}
		}
	}

	hampt_walk(&word_table, walker, NULL, 1);
	if(inmem) {
		while(word_list) word_list = del_word_list(word_list);
		hampt_clear(&word_table);
	} else {
		hampt_clear(&word_table, freewords);
	}

	timestamp(timed, &rt2);

	size_t n = (insertion_count > word_count) ? insertion_count : word_count;
	int    d = ndigits(n);
	if(stats) {
		printf("%*zu words\n", d, word_count);
		printf("%*zu repetitions\n", d, repetitions);
		printf("%*zu collisions\n", d, collisions);
		printf("%*i.%zu depth\n", d, max_depth, max_chain);
	}
	if(timed) {
		if(insertion_count > 0) {
			printf("%*zu insertions in ", d, insertion_count);
			print_time_interval(insertion_time.tv_sec, insertion_time.tv_nsec);
			putstr(" = ");
			time_per(&insertion_time, insertion_count, &insertion_time);
			print_time_interval(insertion_time.tv_sec, insertion_time.tv_nsec);
			puts(" per word");
		}
		if(deletion_count > 0) {
			printf("%*zu deletions in ", d, deletion_count);
			print_time_interval(deletion_time.tv_sec, deletion_time.tv_nsec);
			putstr(" = ");
			time_per(&deletion_time, deletion_count, &deletion_time);
			print_time_interval(deletion_time.tv_sec, deletion_time.tv_nsec);
			puts(" per word");
		}
		time_interval(&rt1, &rt2, &rt2);
		print_time_interval(rt2.tv_sec, rt2.tv_nsec);
		putchar('\n');
	}

	return 0;
}
