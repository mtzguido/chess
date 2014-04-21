#include "addon_trans.h"
#include "addon.h"

#include <stdio.h>
#include <assert.h>

#define TRANS_SCORE 10000

static inline score max(score a, score b) { return a > b ? a : b; }
static inline score min(score a, score b) { return a < b ? a : b; }

static int seq = 0;

struct tt_entry {
	u64	key;
	int	depth;
	score	score;
	u8	flag;
	int	seq;
	i8	r, c, R, C;
};

struct tt_entry tt[CFG_TTABLE_SIZE] __attribute__((aligned(4096)));

static void trans_reset() {
	seq++;
}

static void notify_coll()
{
}

static void trans_notify_return(game g, move move, int depth, score score, flag_t flag) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	if (tt[idx].seq == seq)
		notify_coll();

	tt[idx].key   = key;
	tt[idx].score = score;
	tt[idx].depth = depth;
	tt[idx].flag  = flag;
	tt[idx].seq   = seq;

	if (move.move_type == MOVE_REGULAR && move.promote == 0) {
		tt[idx].r = move.r;
		tt[idx].R = move.R;
		tt[idx].c = move.c;
		tt[idx].C = move.C;
	} else {
		tt[idx].r = -1;
	}
}

static void asd ()
{
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

	asd();
}

static int trans_suggest(game g, move *arr, int depth) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	if (tt[idx].key != key)
		return 0;

	if (tt[idx].r < 0)
		return 0;

	arr[0].move_type = MOVE_REGULAR;
	arr[0].who = g->turn;
	arr[0].r = tt[idx].r;
	arr[0].R = tt[idx].R;
	arr[0].c = tt[idx].c;
	arr[0].C = tt[idx].C;

	return 1;
}

static struct addon trans_addon __maybe_unused = {
	.reset		= trans_reset,
	.notify_return	= trans_notify_return,
	.notify_entry	= trans_notify_entry,
	.suggest	= trans_suggest,
};

void addon_trans_init() {
#ifdef CFG_TRANSPOSITION
	addon_register(trans_addon);
#else
	;
#endif
}

