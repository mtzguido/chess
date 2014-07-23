#include "addon_killer.h"
#include "addon.h"

#define KILLER_WIDTH 2
#define KTABLE_SIZE (MAX_DEPTH + 10)

static move killerTable[KTABLE_SIZE][KILLER_WIDTH];

static void killer_reset() {
	int i, j;
	for (i=0; i<KTABLE_SIZE; i++)
		for (j=0; j<KILLER_WIDTH; j++)
			killerTable[i][j].move_type = MOVE_INVAL;
}

static void killer_score_succs(game g __maybe_unused, int depth) {
	int i, k;

	/*
	 * Usamos también las killers de 2
	 * plies atrás.
	 * */
	for (k=0; k<KILLER_WIDTH; k++) {
		for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
			if (equalMove(gsuccs[i].m, killerTable[depth][k])) {
				gsuccs[i].s += KILLER_SCORE;
				break;
			}
		}
	}
}

static void killer_notify_cut(game g __maybe_unused,
			      move m, int depth) {
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

static int killer_suggest(game g, move *arr, int depth) {
	int i, c;

	if (depth > KTABLE_SIZE)
		return 0;

	c = 0;
	for (i=0; i<KILLER_WIDTH; i++) {
		if (killerTable[depth][i].move_type == MOVE_INVAL)
			continue;

		if (killerTable[depth][i].who != g->turn)
			continue;

		arr[c++] = killerTable[depth][i];
	}

	return c;
}

static struct addon killer_addon __maybe_unused =
{
	.reset = killer_reset,
	.score_succs = killer_score_succs,
	.notify_cut = killer_notify_cut,
	.suggest = killer_suggest,
};

void addon_killer_init() {
	addon_register(killer_addon);
}
