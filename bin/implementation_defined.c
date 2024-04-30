#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <float.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <uchar.h>
#include <math.h>
#include <time.h>

#define TOSTR_(TOSTR__Value)        #TOSTR__Value
#define TOSTR(TOSTR__Value)   TOSTR_(TOSTR__Value)

enum type {
	TYPEOF_VOID,

	TYPEOF_CHAR,
	TYPEOF_UCHAR,
	TYPEOF_SHORT,
	TYPEOF_USHORT,
	TYPEOF_INT,
	TYPEOF_UINT,
	TYPEOF_LONG,
	TYPEOF_ULONG,
	TYPEOF_LLONG,
	TYPEOF_ULLONG,
	TYPEOF_FLOAT,
	TYPEOF_DOUBLE,
	TYPEOF_LDOUBLE,
	TYPEOF_STRING,
	TYPEOF_POINTER,

	TYPEOF_UNKNOWN,

	N_TYPES
};

char const *const typename[] = {
	"void",
	"signed char",
	"unsigned char",
	"signed short",
	"unsigned short",
	"signed int",
	"unsigned int",
	"signed long",
	"unsigned long",
	"signed long long",
	"unsigned long long",
	"float",
	"double",
	"long double",
	"char *",
	"void *",
	"unknown"
};

#define TYPEOF(TYPEOF__x)  _Generic((TYPEOF__x), \
	signed char : TYPEOF_CHAR, \
	unsigned char : TYPEOF_UCHAR, \
	signed short : TYPEOF_SHORT, \
	unsigned short : TYPEOF_USHORT, \
	signed int : TYPEOF_INT, \
	unsigned int : TYPEOF_UINT, \
	signed long : TYPEOF_LONG, \
	unsigned long : TYPEOF_ULONG, \
	signed long long : TYPEOF_LLONG, \
	unsigned long long : TYPEOF_ULLONG, \
	float : TYPEOF_FLOAT, \
	double : TYPEOF_DOUBLE, \
	long double : TYPEOF_LDOUBLE, \
	char * : TYPEOF_STRING, \
	void * : TYPEOF_POINTER, \
	default : TYPEOF_UNKNOWN \
	)

static struct symbol {
	enum type   type;
	size_t      size;
	char const *name;
	char const *definition;
} const symbol[] = {
#define TYPE(TYPE__name)  { \
		TYPEOF((TYPE__name){0}), sizeof(TYPE__name), #TYPE__name, typename[TYPEOF((TYPE__name){0})] \
	}
#define MACRO(MACRO__name)  { \
		TYPEOF(MACRO__name), sizeof(MACRO__name), #MACRO__name, TOSTR(MACRO__name) \
	}
	MACRO(__STDC_VERSION__),
	MACRO(__STDC_HOSTED__),
	MACRO(__STDC__),
#ifdef __STDC_ISO_10646__
	MACRO(__STDC_ISO_10646__),
#endif
#ifdef __STDC_IEC_559__
	MACRO(__STDC_IEC_559__),
#endif
#ifdef __STDC_IEC_559_COMPLEX__
	MACRO(__STDC_IEC_559_COMPLEX__),
#endif
#ifdef __STDC_UTF_16__
	MACRO(__STDC_UTF_16__),
#endif
#ifdef __STDC_UTF_32__
	MACRO(__STDC_UTF_32__),
#endif
#ifdef __STDC_MB_MIGHT_NEQ_WC__
	MACRO(__STDC_MB_MIGHT_NEQ_WC__),
#endif
#ifdef __STDC_ANALYZABLE__
	MACRO(__STDC_ANALYZABLE__),
#endif
#ifdef __STDC_LIB_EXT1__
	MACRO(__STDC_LIB_EXT1__),
#endif
#ifdef __STDC_NO_ATOMICS__
	MACRO(__STDC_NO_ATOMICS__),
#endif
#ifdef __STDC_NO_COMPLEX__
	MACRO(__STDC_NO_COMPLEX__),
#endif
#ifdef __STDC_NO_THREADS__
	MACRO(__STDC_NO_THREADS__),
#endif
#ifdef __STDC_NO_VLA__
	MACRO(__STDC_NO_VLA__),
#endif
#ifdef __STDC_IEC_60559_BFP__
	MACRO(__STDC_IEC_60559_BFP__),
#endif
#ifdef __STDC_IEC_60559_DFP__
	MACRO(__STDC_IEC_60559_DFP__),
#endif
#ifdef __STDC_IEC_60559_COMPLEX__
	MACRO(__STDC_IEC_60559_COMPLEX__),
#endif
	MACRO(NULL),
	MACRO(CHAR_BIT),
	MACRO(CHAR_MIN),
	MACRO(CHAR_MAX),
	MACRO(SCHAR_MIN),
	MACRO(SCHAR_MAX),
	MACRO(UCHAR_MAX),
	MACRO(SHRT_MIN),
	MACRO(SHRT_MAX),
	MACRO(USHRT_MAX),
	MACRO(INT_MIN),
	MACRO(INT_MAX),
	MACRO(UINT_MAX),
	MACRO(LONG_MIN),
	MACRO(LONG_MAX),
	MACRO(ULONG_MAX),
	MACRO(LLONG_MIN),
	MACRO(LLONG_MAX),
	MACRO(ULLONG_MAX),
	MACRO(FLT_RADIX),
	MACRO(FLT_ROUNDS),
	MACRO(FLT_EVAL_METHOD),
	MACRO(DECIMAL_DIG),
	MACRO(INFINITY),
	MACRO(NAN),
	TYPE(float_t),
	MACRO(HUGE_VALF),
	MACRO(FLT_DECIMAL_DIG),
	MACRO(FLT_DIG),
	MACRO(FLT_EPSILON),
	MACRO(FLT_HAS_SUBNORM),
	MACRO(FLT_MANT_DIG),
	MACRO(FLT_MAX),
	MACRO(FLT_MAX_10_EXP),
	MACRO(FLT_MAX_EXP),
	MACRO(FLT_MIN),
	MACRO(FLT_MIN_10_EXP),
	MACRO(FLT_MIN_EXP),
	MACRO(FLT_TRUE_MIN),
	MACRO(FLT_HAS_SUBNORM),
	TYPE(double_t),
	MACRO(HUGE_VAL),
	MACRO(DBL_DECIMAL_DIG),
	MACRO(DBL_DIG),
	MACRO(DBL_EPSILON),
	MACRO(DBL_HAS_SUBNORM),
	MACRO(DBL_MANT_DIG),
	MACRO(DBL_MAX),
	MACRO(DBL_MAX_10_EXP),
	MACRO(DBL_MAX_EXP),
	MACRO(DBL_MIN),
	MACRO(DBL_MIN_10_EXP),
	MACRO(DBL_MIN_EXP),
	MACRO(DBL_TRUE_MIN),
	MACRO(DBL_HAS_SUBNORM),
	MACRO(HUGE_VALL),
	MACRO(LDBL_DECIMAL_DIG),
	MACRO(LDBL_DIG),
	MACRO(LDBL_EPSILON),
	MACRO(LDBL_HAS_SUBNORM),
	MACRO(LDBL_MANT_DIG),
	MACRO(LDBL_MAX),
	MACRO(LDBL_MAX_10_EXP),
	MACRO(LDBL_MAX_EXP),
	MACRO(LDBL_MIN),
	MACRO(LDBL_MIN_10_EXP),
	MACRO(LDBL_MIN_EXP),
	MACRO(LDBL_TRUE_MIN),
	MACRO(LDBL_HAS_SUBNORM),
	TYPE(size_t),
	MACRO(SIZE_MAX),
#ifdef RSIZE_MAX
	TYPE(rsize_t),
	MACRO(RSIZE_MAX),
#endif
	TYPE(intmax_t),
	MACRO(INTMAX_MAX),
	TYPE(uintmax_t),
	MACRO(UINTMAX_MAX),
	TYPE(max_align_t),
	TYPE(ptrdiff_t),
	MACRO(PTRDIFF_MIN),
	MACRO(PTRDIFF_MAX),
	TYPE(sig_atomic_t),
	MACRO(SIG_ATOMIC_MIN),
	MACRO(SIG_ATOMIC_MAX),
	TYPE(wchar_t),
	TYPE(wint_t),
	TYPE(wctype_t),
	TYPE(wctrans_t),
	MACRO(WCHAR_MIN),
	MACRO(WCHAR_MAX),
	MACRO(WINT_MIN),
	MACRO(WINT_MAX),
	TYPE(char16_t),
	TYPE(char32_t),
	MACRO(MB_LEN_MAX),
	MACRO(MB_CUR_MAX),
	TYPE(errno_t),
	TYPE(fpos_t),
	MACRO(FOPEN_MAX),
	MACRO(FILENAME_MAX),
#ifdef PATH_MAX
	MACRO(PATH_MAX),
#endif
	MACRO(BUFSIZ),
	MACRO(TMP_MAX),
	MACRO(L_tmpnam),
	MACRO(TMP_MAX_S),
	MACRO(L_tmpnam_s),
	TYPE(time_t),
	TYPE(clock_t),
	MACRO(CLOCKS_PER_SEC),
	MACRO(RAND_MAX),
#undef MACRO
#undef TYPE
};
static size_t const n_symbols = sizeof(symbol) / sizeof(symbol[0]);

static int
ndigits(
	size_t x
) {
	return (x > 99) + (x > 9) + 1;
}

int
main(
	int    argc,
	char **argv
) {
	(void)argc;
	(void)argv;

	int type_len = 0;
	int size_len = 0;
	int name_len = 0;
	for(size_t i = 0; i < n_symbols; i++) {
		int n;
		n = strlen(typename[symbol[i].type]);
		if(type_len < n) type_len = n;
		n = ndigits(symbol[i].size);
		if(size_len < n) size_len = n;
		n = strlen(symbol[i].name);
		if(name_len < n) name_len = n;
	}
	for(size_t i = 0; i < n_symbols; i++) {
		printf("%*s [%*zu] %*s: %s\n",
			type_len, typename[symbol[i].type],
			size_len, symbol[i].size,
			name_len, symbol[i].name,
			symbol[i].definition
		);
	}
	return 0;
}
