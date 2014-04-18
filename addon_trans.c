#include "addon_trans.h"
#include "addon.h"

#include <stdio.h>
#include <assert.h>

#define TRANS_SCORE 10000

static inline score max(score a, score b) { return a > b ? a : b; }
static inline score min(score a, score b) { return a < b ? a : b; }

static int seq = 0;

struct tt_entry {
	u64    key;
	int    depth;
	score  score;
	u8     flag;
	int seq;
};

struct tt_entry tt[CFG_TTABLE_SIZE] __attribute__((aligned(4096)));

static void trans_reset() {
	seq++;
}

static void trans_notify_return(game g, move m, int depth, score score, flag_t flag) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	m=m; /* !!!! */

	tt[idx].key   = key;
	tt[idx].score = score;
	tt[idx].depth = depth;
	tt[idx].flag  = flag;
	tt[idx].seq   = seq;
}

static void trans_notify_entry(game g, int depth, score *alpha, score *beta) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	if (tt[idx].seq < seq)
		return;

	if (tt[idx].key != key)
		return;

	if (tt[idx].depth > depth)
		return;

	switch (tt[idx].flag) {
	case FLAG_EXACT:
		*alpha = *beta = tt[idx].score;
		break;
	case FLAG_LOWER_BOUND:
		*alpha = max(*alpha, tt[idx].score);
		break;
	case FLAG_UPPER_BOUND:
		*beta = min(*beta, tt[idx].score);
		break;
	}
}

#if 0
static void trans_score_succs(game g, const move *succs, score *vals, int nsucc, int depth) {
}

static void trans_free_mem() {
}

static int trans_suggest(game g, move *arr, int depth) {
	return 0;
}
#endif

static struct addon trans_addon __maybe_unused = {
	.reset		= trans_reset,
	.notify_return	= trans_notify_return,
	.notify_entry	= trans_notify_entry,
};

void addon_trans_init() {
#ifdef CFG_TRANSPOSITION
	addon_register(trans_addon);
#else
	;
#endif
}

