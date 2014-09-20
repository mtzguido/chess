#include "addon_killer.h"
#include "addon.h"

#define KILLER_WIDTH 2
#define KTABLE_SIZE (MAX_DEPTH + 10)

static move killerTable[KTABLE_SIZE][KILLER_WIDTH];

void killer_reset() {
	int i, j;
	for (i = 0; i < KTABLE_SIZE; i++)
		for (j = 0; j < KILLER_WIDTH; j++)
			killerTable[i][j].move_type = MOVE_INVAL;
}

void killer_score_succs(int depth) {
	int i, k;

	if (depth > MAX_DEPTH)
		return;

	/*
	 * Usamos también las killers de 2
	 * plies atrás.
	 * */
	for (k = 0; k < KILLER_WIDTH; k++) {
		for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
			if (equalMove(gsuccs[i].m, killerTable[depth][k])) {
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

	for (i=0; i<KILLER_WIDTH; i++)
		if (equalMove(killerTable[depth][i], m))
			return;

	for (i=KILLER_WIDTH-1; i >= 1; i--)
		killerTable[depth][i] = killerTable[depth][i-1];

	killerTable[depth][0] = m;
}
