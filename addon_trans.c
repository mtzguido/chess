#include "addon.h"
#include "addon_trans.h"

static inline score max(score a, score b) { return a > b ? a : b; }
static inline score min(score a, score b) { return a < b ? a : b; }

static int trans_seq = 0;

struct tt_entry {
	u64 key;
	score val;
	flag_t flag;
	move m;
	u8 depth, seq;
};

struct tt_entry tt[CFG_TTABLE_SIZE] __attribute__((aligned(4096)));

void trans_reset() {
	trans_seq++;
}

void trans_notify_return(move move, int depth, score score, flag_t flag) {
	u64 key = G->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	if (tt[idx].key && tt[idx].key != key)
		stats.tt_collision++;

	tt[idx].key = key;
	tt[idx].seq = trans_seq;
	tt[idx].m = move;
	tt[idx].val = score;
	tt[idx].flag = flag;
	tt[idx].depth = depth;
}

#define trans_notify_cut(...)	do { } while (0)
#define trans_suggest(...)	do { } while (0)

void trans_notify_entry(int depth, score *alpha, score *beta) {
	u64 key = G->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;
	const struct tt_entry entry = tt[idx];

	if (entry.seq != trans_seq)
		return;

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

#if 0
int trans_suggest(move *arr, int depth) {
	u64 key = G->zobrist;
	u64 idx = key % CFG_TTABLE_SIZE;

	if (tt[idx].key != key)
		return 0;

	if (tt[idx].r < 0)
		return 0;

	arr[0].move_type = MOVE_REGULAR;
	arr[0].who = G->turn;
	arr[0].r = tt[idx].r;
	arr[0].R = tt[idx].R;
	arr[0].c = tt[idx].c;
	arr[0].C = tt[idx].C;

	return 1;
}
#endif

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
