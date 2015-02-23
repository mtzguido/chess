#include "addon_killer.h"
#include "addon.h"

#define KILLER_WIDTH 2
#define KTABLE_SIZE MAX_PLY

static move killerTable[KTABLE_SIZE][KILLER_WIDTH];

void killer_reset() {
	int i;

	/*
	 * Only mark the first entry as invalid,
	 * the scoring function will find it and
	 * stop without checking the others
	 */
	for (i = 0; i < KTABLE_SIZE; i++)
		killerTable[i][0].move_type = MOVE_INVAL;
}

void killer_score_succs(int depth) {
	int i, k;

	if (depth > KTABLE_SIZE)
		return;

	for (k = 0; k < KILLER_WIDTH; k++) {
		const move m = killerTable[depth][k];

		/*
		 * The table is filled left to right, so if we
		 * have an empty entry there are no more valid
		 * ones
		 */
		if (m.move_type == MOVE_INVAL)
			return;

		for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
			if (equalMove(gsuccs[i].m, m)) {
				gsuccs[i].s += KILLER_SCORE;
				break;
			}
		}
	}
}

void killer_notify_cut(move m, int depth) {
	int i;

	if (depth > KTABLE_SIZE)
		return;

	for (i = 0; i < KILLER_WIDTH; i++) {
		move *tm = &killerTable[depth][i];

		if (equalMove(*tm, m)) {
			if (i != 0) {
				move swap;

				swap = *tm;
				*tm = killerTable[depth][0];
				killerTable[depth][0] = swap;
			}

			return;
		}
	}

	for (i = KILLER_WIDTH-1; i >= 1; i--)
		killerTable[depth][i] = killerTable[depth][i-1];

	killerTable[depth][0] = m;
}
