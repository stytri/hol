#ifndef HOL_HAMPT_H__INCLUDED
#define HOL_HAMPT_H__INCLUDED 1
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

#include <hol/xtdlib.h>

//------------------------------------------------------------------------------

struct hampt {
	void *p;
};
#define HAMPT(...)  { .p = NULL }

extern void **hampt_lookup(struct hampt const *mp, uint64_t h);
extern void **hampt_insert(struct hampt       *mp, uint64_t h);
extern int    hampt_walk  (struct hampt       *mp, int (*cb)(void *, void *, int), void *c, int d);

static inline void
hampt_clear(
	struct hampt *mp,
	void        (*cb)(void *)
) {
	extern void hampt_clear__with_callback(struct hampt *mp, void (*cb)(void *));
	extern void hampt_clear__no_callback  (struct hampt *mp);
	if(mp->p) {
		if(cb) hampt_clear__with_callback(mp, cb);
		else   hampt_clear__no_callback(mp);
	}
}
#define hampt_clear(hampt_clear__mp,...)  (hampt_clear)(hampt_clear__mp,__VA_ARGS__+0)

static inline void
hampt_remove(
	struct hampt *mp,
	uint64_t      h
) {
	extern int hampt_remove__node(struct hampt *mp, uint64_t h, int o);
	if(mp->p) hampt_remove__node(mp, h, 0);
}

//------------------------------------------------------------------------------

#endif//ndef HOL_HAMPT_H__INCLUDED

//------------------------------------------------------------------------------

#ifdef HOL_HAMPT_H__IMPLEMENTATION
#undef HOL_HAMPT_H__IMPLEMENTATION

//------------------------------------------------------------------------------

enum {
	HAMPT_BITS_PER_HASH = sizeof(uint64_t) * CHAR_BIT,
	HAMPT_BITS_PER_NODE = 3 + (HAMPT_BITS_PER_HASH > 8) + (HAMPT_BITS_PER_HASH > 16) + (HAMPT_BITS_PER_HASH > 32),
	HAMPT_NODE_BIT_MASK = (1u << HAMPT_BITS_PER_NODE) - 1
};

struct hampt_node {
	uint64_t     pop;
	struct hampt map[];
};

struct hampt_leaf {
	uint64_t hash;
	void    *p;
};

//------------------------------------------------------------------------------

void
hampt_clear__no_callback(
	struct hampt *mp
) {
	int is_node = is_tagged_pointer(mp->p);
	struct hampt_node *np = untag_pointer(mp->p);
	if(is_node) {
		for(int i = 0, n = popcount(np->pop); i < n; i++) {
			hampt_clear__no_callback(&np->map[i]);
		}
	}
	free(np);
	return;
}

void
hampt_clear__with_callback(
	struct hampt *mp,
	void        (*cb)(void *)
) {
	int is_node = is_tagged_pointer(mp->p);
	if(is_node) {
		struct hampt_node *np = untag_pointer(mp->p);
		for(int i = 0, n = popcount(np->pop); i < n; i++) {
			hampt_clear__with_callback(&np->map[i], cb);
		}
		free(np);
		return;
	}
	struct hampt_leaf *lp = mp->p;
	cb(lp->p);
	free(lp);
	return;
}

int
hampt_remove__node(
	struct hampt *mp,
	uint64_t      h,
	int           o
) {
	int is_node = is_tagged_pointer(mp->p);
	if(is_node) {
		struct hampt_node *np = untag_pointer(mp->p);
		size_t             i  = (h >> o) & HAMPT_NODE_BIT_MASK;
		uint64_t           b  = UINT64_C(1) << i;
		if(!(np->pop & b)) return 0;

		uint64_t x = np->pop & (b - 1);
		size_t   j = popcount(x);
		int q = hampt_remove__node(&np->map[j], h, o + HAMPT_BITS_PER_NODE);
		if(!q) return 0;

		if(q < 0) {
			np->pop &= ~b;
			if(!np->pop) {
				mp->p = NULL;
				free(np);
				return -1;
			}
			size_t k = popcount(np->pop);
			for(; j < k; j++) np->map[j] = np->map[j+1];
			if(at_capacity(k)) {
				size_t z = sizeof(*np) + (sizeof(np->map[0]) * k);
				np = realloc(np, z);
				if(unlikely(!np)) return 0;
				mp->p = tag_pointer(np);
			}
			q = !is_tagged_pointer(np->map->p);
		}
		int k = popcount(np->pop);
		if(k == q) {
			*mp = *np->map;
			free(np);
			return 1;
		}
		return 0;
	}
	struct hampt_leaf *lp = mp->p;
	mp->p = NULL;
	free(lp);
	return -1;
}

void **
hampt_lookup(
	struct hampt const *mp,
	uint64_t            h
) {
	if(mp->p) for(int o = 0; ; o += HAMPT_BITS_PER_NODE) {
		int is_node = is_tagged_pointer(mp->p);
		if(is_node) {
			struct hampt_node const *np = untag_pointer(mp->p);
			size_t                   i  = (h >> o) & HAMPT_NODE_BIT_MASK;
			uint64_t                 b  = UINT64_C(1) << i;
			uint64_t                 x  = np->pop & (b - 1);
			size_t                   j  = popcount(x);
			if(!(np->pop & b)) break;
			mp = &np->map[j];
			continue;
		}

		struct hampt_leaf *lp = mp->p;
		if(lp->hash == h) return &lp->p;
		break;
	}
	return NULL;
}

void **
hampt_insert(
	struct hampt *mp,
	uint64_t      h
) {
	struct hampt_leaf *lp;
	size_t            z;

	if(mp->p) for(int o = 0; ; o += HAMPT_BITS_PER_NODE) {
		int               is_node = is_tagged_pointer(mp->p);
		struct hampt_node *np      = untag_pointer(mp->p);
		if(!is_node) {
			lp = mp->p;
			if(lp->hash == h) return &lp->p;

			size_t   i = (lp->hash >> o) & HAMPT_NODE_BIT_MASK;
			uint64_t b = UINT64_C(1) << i;
			z  = sizeof(*np) + sizeof(np->map[0]);
			np = malloc(z);
			if(unlikely(!np)) return NULL;
			np->map[0].p = lp;
			np->pop      = b;

			mp->p = tag_pointer(np);
		}

		size_t   i = (h >> o) & HAMPT_NODE_BIT_MASK;
		uint64_t b = UINT64_C(1) << i;
		uint64_t x = np->pop & (b - 1);
		size_t   j = popcount(x);
		if(np->pop & b) {
			mp = &np->map[j];
			continue;
		}

		size_t k = popcount(np->pop);
		if(at_capacity(k)) {
			z  = sizeof(*np) + (sizeof(np->map[0]) * (k * 2));
			np = realloc(np, z);
			if(unlikely(!np)) return NULL;
			mp->p = tag_pointer(np);
		}
		mp = &np->map[j];
		for(; k-- > j; np->map[k+1] = np->map[k]);
		np->pop |= b;
		break;
	}

	z  = sizeof(*lp);
	lp = malloc(z);
	if(unlikely(!lp)) return NULL;
	mp->p    = lp;
	lp->hash = h;
	lp->p    = NULL;
	return &lp->p;
}

int
hampt_walk(
	struct hampt *mp,
	int         (*cb)(void *, void *, int),
	void         *c,
	int           d
) {
	int rc = 0;
	if(mp->p) {
		int is_node = is_tagged_pointer(mp->p);
		if(is_node) {
			struct hampt_node *np = untag_pointer(mp->p);
			int               d1 = d + 1;
			for(size_t i = 0, n = popcount(np->pop); i < n; i++) {
				if((rc = hampt_walk(&np->map[i], cb, c, d1)) != 0) break;
			}
		} else {
			struct hampt_leaf *lp = mp->p;
			rc = cb(lp->p, c, d);
		}
	}
	return rc;
}

//------------------------------------------------------------------------------

#endif//def HOL_HAMPT_H__IMPLEMENTATION
