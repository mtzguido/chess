#include "ztable.h"
#include "common.h"
#include <stdlib.h>
#include <assert.h>

int n_collision = 0;

struct bucket {
	u64 key;
	struct bucket *next;
	int n;
};

struct bucket * ztable[CFG_ZTABLE_SIZE] __attribute__((aligned(0x1000)));

void mark() {
	u64 key = G->zobrist;
	u64 idx = key % CFG_ZTABLE_SIZE;

	struct bucket *p = ztable[idx];

	while (p && key != p->key) {
		n_collision++;
		p = p->next;
	}

	if (p) {
		p->n++;
	} else {
		p = (struct bucket *) malloc(sizeof *p);
		p->key = G->zobrist;
		p->n = 1;
		p->next = ztable[idx];
		ztable[idx] = p;
	}
}

void unmark() {
	u64 key = G->zobrist;
	u64 idx = key % CFG_ZTABLE_SIZE;

	struct bucket *p = ztable[idx];

	assert(p);
	if (p->key == key) {
		p->n--;

		if (p->n == 0) {
			struct bucket *t = p->next;
			free(p);
			ztable[idx] = t;
		}
	} else {
		while (p->next && key != p->next->key)
			p = p->next;

		assert(p->next);

		p->next->n--;
		if (p->next->n == 0) {
			struct bucket *t = p->next->next;
			free(p->next);
			p->next = t;
		}
	}
}

int reps() {
	u64 key = G->zobrist;
	u64 idx = key % CFG_ZTABLE_SIZE;

	struct bucket *p = ztable[idx];

	while (p && key != p->key)
		p = p->next;

	if (p)
		return p->n;
	else
		return 0;
}
