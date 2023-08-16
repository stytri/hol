#ifndef HOL_ARRAY_H__INCLUDED
#define HOL_ARRAY_H__INCLUDED 1
/*
MIT License

Copyright (c) 2023 Tristan Styles

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

#include <hol/xtdlib.h>

//------------------------------------------------------------------------------

struct array {
	size_t   len;
	size_t   cap;
	void   **bin;
};
#define ARRAY(...)   { .len = 0, .cap = 0, .bin = NULL }

//------------------------------------------------------------------------------

extern struct array *new_array(void);
extern void          del_array(struct array *a);

extern void array_clear (struct array *a);
extern bool array_expand(struct array *a, size_t size, size_t count);

extern void *elementof_array(struct array const *a, size_t size, size_t index);

//------------------------------------------------------------------------------

static inline bool
array_reserve(
	struct array *a,
	size_t        size,
	size_t        count
) {
	size_t avail = a->cap - a->len;

	return (avail >= count) || array_expand(a, size, count - avail);
}
#define array_reserve(type,a,count)  (array_reserve)(a, sizeof(type), count)

static inline void *
array_push(
	struct array *a,
	size_t        size
) {
	if(likely((array_reserve)(a, size, 1))) {
		void *p = elementof_array(a, size, a->len);
		if(likely(p)) {
			a->len++;
			return p;
		}
	}
	return NULL;
}
#define array_push(type,a)  (type *)(array_push)(a, sizeof(type))

static inline void *
array_pop(
	struct array *a,
	size_t        size
) {
	if(likely(a->len > 0)) {
		a->len--;
		return elementof_array(a, size, a->len);
	}
	return NULL;
}
#define array_pop(type,a)  (type *)(array_pop)(a, sizeof(type))

static inline void
array_drop(
	struct array *a,
	size_t        size
) {
	(void)size;
	if(likely(a->len > 0)) {
		a->len--;
	}
}
#define array_drop(type,a)  (array_drop)(a, sizeof(type))

static inline void *
array_last(
	struct array const *a,
	size_t              size
) {
	return likely(a->len > 0) ? elementof_array(a, size, a->len - 1) : NULL;
}
#define array_last(type,a)  (type *)(array_last)(a, sizeof(type))

static inline void *
array_at(
	struct array const *a,
	size_t              size,
	size_t              index
) {
	return likely(index < a->len) ? elementof_array(a, size, index) : NULL;
}
#define array_at(type,a,index)  (type *)(array_at)(a, sizeof(type), index)

//------------------------------------------------------------------------------

#define FOREACH(FOREACH__type,FOREACH__var,FOREACH__array,FOREACH__action)  \
do { \
	size_t         FOREACH__len = at_capacity((FOREACH__array)->len) ? (FOREACH__array)->len : (FOREACH__array)->cap / 2; \
	size_t         FOREACH__rem = (FOREACH__array)->len - FOREACH__len; \
	int            FOREACH__end = msbit(FOREACH__len); \
	FOREACH__type *FOREACH__var; \
	for(int FOREACH__i = 0; FOREACH__i < FOREACH__end; FOREACH__i++) { \
		size_t FOREACH__n = (FOREACH__i > 0) ? (((~SIZE_C(0) >> 1) >> (SIZE_BIT - FOREACH__i)) + 1) : 1; \
		for(FOREACH__var = (FOREACH__array)->bin[FOREACH__i]; FOREACH__n--; FOREACH__var++) { \
			FOREACH__action \
		} \
	} \
	if(FOREACH__rem > 0) { \
		for(FOREACH__var = (FOREACH__array)->bin[FOREACH__end]; FOREACH__rem--; FOREACH__var++) { \
			FOREACH__action \
		} \
	} \
} while(0)

//------------------------------------------------------------------------------

#endif//ndef HOL_ARRAY_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_ARRAY_H__IMPLEMENTATION
#undef HOL_ARRAY_H__IMPLEMENTATION

//------------------------------------------------------------------------------

struct array *
new_array(
	void
) {
	struct array *const a = malloc(sizeof(*a));
	if(likely(a)) {
		*a = (struct array)ARRAY();
	}
	return a;
}

void
del_array(
	struct array *const a
) {
	array_clear(a);
	free((void *)a);
}

void
array_clear(
	struct array *const a
) {
	if(likely(a)) {
		void **const bin = a->bin;
		if(bin) {
			for(int n = msbit(a->cap); n--; ) {
				free(bin[n]);
			}
		}
	}
}

bool
array_expand(
	struct array *a,
	size_t        size,
	size_t        count
) {
	size_t cap_max  = (SIZE_MAX / 2) / size;
	size_t capacity = a ? a->cap : cap_max;

	if(likely((0 < count) && (count <= (cap_max - capacity)))) {
		size_t new_capacity = capacity_of(capacity + count);
		int    new_tail     = msbit(new_capacity);
		int    tail         = msbit(capacity);
		void **bin          = a->bin;

		if(!bin || (tail < new_tail)) {
			bin = realloc(a->bin, ((size_t)new_tail * sizeof(void *)));
			if(likely(bin)) {
				a->bin = bin;

				if(capacity == 0) {
					bin[0] = malloc(size);
					if(unlikely(!bin[0])) {
						return false;
					}

					capacity = a->cap = 1;
					++tail;
				}

				for(; tail < new_tail; ++tail) {
					bin[tail] = malloc(capacity * size);
					if(unlikely(!bin[tail])) {
						return false;
					}

					a->cap += capacity;
					capacity += capacity;
				}

				return true;
			}
		}
	}

	return !count;
}

void *
elementof_array(
	struct array const *a,
	size_t              size,
	size_t              index
) {
	int     z = lzcount(index);
	size_t  m = (~SIZE_C(0) >> 1) >> z;
	int     i = BITS(index) - z;
	size_t  x = index & m;
	char   *p = a->bin[i];
	return  p + (x * size);
}

//------------------------------------------------------------------------------

#endif//def HOL_ARRAY_H__IMPLEMENTATION
