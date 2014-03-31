#include "ztable.h"
#include <stdlib.h>
#include <assert.h>

int n_collision = 0;

struct bucket * ztable[CFG_ZTABLE_SIZE];

void mark(game g) {
	int idx = g->zobrist % CFG_ZTABLE_SIZE;

	struct bucket *p = ztable[idx];

	while (p && !equalGame(g, p->g)) {
		n_collision++;
		p = p->next;
	}

	if (p) {
		p->n++;
	} else {
		p = malloc(sizeof *p);
		p->g = copyGame(g);
		p->n = 1;
		p->next = ztable[idx];
		ztable[idx] = p;
	}
}

void unmark(game g) {
	int idx = g->zobrist % CFG_ZTABLE_SIZE;

	struct bucket *p = ztable[idx];

	assert(p);
	if (equalGame(p->g, g)) {
		p->n--;

		if (p->n == 0) {
			struct bucket *t = p->next;
			freeGame(p->g);
			free(p);
			ztable[idx] = t;
		}
	} else {
		while (p->next && !equalGame(g, p->next->g))
			p = p->next;

		assert(p->next);

		p->next->n--;
		if (p->next->n == 0) {
			struct bucket *t = p->next->next;
			freeGame(p->next->g);
			free(p->next);
			p->next = t;
		}
	}
}

int reps(game g) {
	int idx = g->zobrist % CFG_ZTABLE_SIZE;

	struct bucket *p = ztable[idx];

	while (p && !equalGame(g, p->g))
		p = p->next;

	if (p)
		return p->n;
	else
		return 0;
}

