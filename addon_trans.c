#include "addon_trans.h"
#include "addon.h"

#include <stdio.h>
#include <assert.h>

#define TRANS_SCORE 10000

struct tt_bucket {
	game g;
	move m;
	score s;
	int seq;
	struct tt_bucket *next;
};

static struct tt_bucket * tt_table[CFG_TTABLE_SIZE] = { NULL };
static uint32_t seq = 0;
static uint32_t flush = 0;

static void trans_reset() {
}

static void trans_notify_return(game g, move m, score s, int depth) {
	int idx = g->zobrist % CFG_TTABLE_SIZE;

	struct tt_bucket *p = tt_table[idx];

	assert(m.move_type != -1);
	assert(m.who == g->turn);

	while (p && !equalGame(g, p->g))
		p = p->next;

	if (p) {
		p->m = m;
		p->s = s;
	} else {
		p = malloc(sizeof *p);
		p->g = copyGame(g);
		p->m = m;
		p->s = s;
		p->next = tt_table[idx];
		p->seq = seq++;
		tt_table[idx] = p;
	}
}

static void trans_sort_succs(game g, const move *succs, score *vals, int nsucc, int depth) {
	int idx = g->zobrist % CFG_TTABLE_SIZE;

	struct tt_bucket *p = tt_table[idx];

	while (p && !equalGame(g, p->g))
		p = p->next;

	if (!p)
		return;

	int i;

	for (i=0; i<nsucc; i++) {
		if (equalMove(p->m, succs[i])) {
			p->seq = seq++;
			vals[i] += TRANS_SCORE;
			return;
		}
	}
}

static void trans_free_mem() {
	static int i = 0;
	fprintf(stderr, "Whoops! Need to free some memory! (%i)\n", i++);

	int idx;
	int threshold = (flush + (seq-flush)/2);
	for (idx=0; idx<CFG_TTABLE_SIZE; idx++) {
		struct tt_bucket sentinel;
		struct tt_bucket *p;

		sentinel.next = tt_table[idx];
		p = &sentinel;

		while (p->next) {
			if (p->next->seq < threshold) {
				struct tt_bucket *temp;
				temp = p->next->next;

				freeGame(p->next->g);
				free(p->next);

				p->next = temp;
			} else {
				p = p->next;
			}
		}
		tt_table[idx] = sentinel.next;
	}

	flush = threshold;
}

static bool trans_notify_entry(game g, int depth, score *ret) {
	int idx = g->zobrist % CFG_TTABLE_SIZE;

	/* Anda esto, sin el alpha-beta?
	 * tiene sentido? */

	struct tt_bucket *p = tt_table[idx];

	while (p && !equalGame(g, p->g))
		p = p->next;

	if (!p)
		return false;

	*ret = p->s;
	return true;
}

static int trans_suggest(game g, move *arr, int depth) {
	int idx = g->zobrist % CFG_TTABLE_SIZE;
	struct tt_bucket *p = tt_table[idx];

	while (p && !equalGame(g, p->g))
		p = p->next;

	if (!p)
		return 0;

	*arr = p->m;

	assert(p->m.who == g->turn);
	return 1;
}

static struct addon trans_addon __attribute__((unused)) = {
	.reset = trans_reset,
	.score_succs = trans_sort_succs,
	.notify_return = trans_notify_return,
	.free_mem = trans_free_mem,
	.suggest = trans_suggest,
//	.notify_entry = trans_notify_entry,
};

void addon_trans_init() {
#ifdef CFG_TRANSPOSITION
	addon_register(trans_addon);
#else
	;
#endif
}

