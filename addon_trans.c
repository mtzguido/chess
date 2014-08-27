#include "addon.h"
#include "addon_trans.h"

/*
static inline score max(score a, score b) { return a > b ? a : b; }
static inline score min(score a, score b) { return a < b ? a : b; }
*/

static int trans_seq = 0;

struct tt_entry {
	u64 key;
	score val;
	move m;
	u8 depth, seq;
};

struct tt_entry tt[CFG_TTABLE_SIZE] __attribute__((aligned(4096)));

void trans_reset() {
	trans_seq++;
}

void trans_notify_return(game g, move move, int depth, score score,
				flag_t flag) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	tt[idx].key = key;
	tt[idx].seq = trans_seq;
	tt[idx].m = move;
}

#define trans_notify_entry(...)	do { } while (0)
#define trans_notify_cut(...)	do { } while (0)
#define trans_suggest(...)	do { } while (0)

#if 0
void trans_notify_entry(game g, int depth, score *alpha, score *beta) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	if (tt[idx].seq != trans_seq)
		return;

	if (tt[idx].flag == FLAG_NONE)
		return;

	if (tt[idx].key != key)
		return;

	if (tt[idx].depth < depth)
		return;

	switch (tt[idx].flag) {
	case FLAG_EXACT:
		*alpha = *beta = tt[idx].val;
		break;
	case FLAG_LOWER_BOUND:
		*alpha = max(*alpha, tt[idx].val);
		break;
	case FLAG_UPPER_BOUND:
		*beta = min(*beta, tt[idx].val);
		break;
	}
}

int trans_suggest(game g, move *arr, int depth) {
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
#endif

void trans_score_succs(game g, int depth) {
	u64 key = g->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;
	int i;

	if (tt[idx].key != key)
		return;

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		if (equalMove(gsuccs[i].m, tt[idx].m)) {
			gsuccs[i].s += TRANS_SCORE;
			stats.tt_hits++;
			break;
		}
	}
}
