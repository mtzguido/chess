#include "addon.h"
#include "addon_trans.h"

struct tt_entry {
	u64 key;
	score val;
	move m;
	u8 depth:5;
	flag_t flag:3;
};

struct tt_entry tt[CFG_TTABLE_SIZE] __attribute__((aligned(4096)));

void trans_reset() {
}

void trans_notify_return(move move, int depth, score score, flag_t flag) {
	u64 key = G->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	tt[idx].key = key;
	tt[idx].m = move;
	tt[idx].val = score;
	tt[idx].flag = flag;
	tt[idx].depth = depth;
}

void trans_notify_entry(int depth, score *alpha, score *beta) {
	u64 key = G->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;
	const struct tt_entry entry = tt[idx];

	if (entry.flag == FLAG_NONE)
		return;

	if (entry.key != key)
		return;

	if (entry.depth < depth)
		return;

	switch (entry.flag) {
	case FLAG_EXACT:
		*alpha = *beta = entry.val;
		break;
	case FLAG_LOWER_BOUND:
		*alpha = max(*alpha, entry.val);
		break;
	case FLAG_UPPER_BOUND:
		*beta = min(*beta, entry.val);
		break;
	default:
		assert(0);
	}
}

void trans_score_succs(int depth) {
	u64 key = G->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;
	int i;

	if (tt[idx].key != key)
		return;

	for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
		if (equalMove(gsuccs[i].m, tt[idx].m)) {
			gsuccs[i].s += TRANS_SCORE;
			stats.tt_hits++;
			break;
		}
	}
}
