#ifndef HOL_SPA_H__INCLUDED
#define HOL_SPA_H__INCLUDED 1
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

#include <stddef.h>
#include <stdlib.h>

//------------------------------------------------------------------------------

struct spa {
	size_t n;
	void  *p[];
};

//------------------------------------------------------------------------------

extern void  *spa_lookup(struct spa **ap, int (*cmp)(void const *, void const *, size_t), void const *p, size_t z);
extern void  *spa_remove(struct spa **ap, int (*cmp)(void const *, void const *, size_t), void const *p, size_t z);
extern void **spa_insert(struct spa **ap, int (*cmp)(void const *, void const *, size_t), void const *p, size_t z);
extern void   spa_free  (struct spa **ap, void (*f)(void *));

//------------------------------------------------------------------------------

#endif//ndef HOL_SPA_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_SPA_H__IMPLEMENTATION
#undef HOL_SPA_H__IMPLEMENTATION

//------------------------------------------------------------------------------

void *
spa_lookup(
	struct spa **ap,
	int        (*cmp)(void const *, void const *, size_t),
	void const  *p,
	size_t       z
) {
	struct spa *a = *ap;
	if(a) for(size_t i = 0, n = a->n; i < n; i++) {
		int q = cmp(a->p[i], p, z);
		if(q == 0) return a->p[i];
		if(q >  0) break;
	}
	return NULL;
}

void *
spa_remove(
	struct spa **ap,
	int        (*cmp)(void const *, void const *, size_t),
	void const  *p,
	size_t       z
) {
	struct spa *a = *ap;
	if(a) {
		for(size_t i = 0, n = a->n; i < n; i++) {
			int q = cmp(a->p[i], p, z);
			if(q == 0) {
				a->n--;
				void *r = a->p[i];
				if(--n == 0) {
					*ap = NULL;
					free(a);
				} else {
					for(; i < n; i++) a->p[i] = a->p[i+1];
					if((n & (n-1)) == 0) {
						a = realloc(a, sizeof(*a) + (n * sizeof(a->p[0])));
						if(unlikely(!a)) return NULL;
						*ap = a;
					}
				}
				return r;
			}
			if(q >  0) break;
		}
	}
	return NULL;
}

void **
spa_insert(
	struct spa **ap,
	int        (*cmp)(void const *, void const *, size_t),
	void const  *p,
	size_t       z
) {
	struct spa *a = *ap;
	size_t      i = 0;
	if(a) {
		size_t n = a->n;
		for(; i < n; i++) {
			int q = cmp(a->p[i], p, z);
			if(q == 0) return &a->p[i];
			if(q >  0) break;
		}
		if((n & (n-1)) == 0) {
			a = realloc(a, sizeof(*a) + ((n * 2) * sizeof(a->p[0])));
			if(unlikely(!a)) return NULL;
			*ap = a;
		}
		for(; n > i; --n) a->p[n] = a->p[n-1];
		a->n++;
	} else {
		a = malloc(sizeof(*a) + sizeof(a->p[0]));
		if(unlikely(!a)) return NULL;
		*ap = a;
		a->n = 1;
	}
	a->p[i] = NULL;
	return &a->p[i];
}

void
spa_free(
	struct spa **ap,
	void       (*f)(void *)
) {
	struct spa *a = *ap;
	if(a) {
		if(f) for(*ap = NULL; a->n-- > 0; f(a->p[a->n]));
		else *ap = NULL, a->n = 0;
		free(a);
	}
}

//------------------------------------------------------------------------------

#endif//ndef HOL_SPA_H__IMPLEMENTATION
