#include "addon_killer.h"
#include "addon.h"

#define KILLER_WIDTH 2
#define KTABLE_SIZE (CFG_DEPTH + 10)
#define KILLER_SCORE 1000

static move killerTable[KTABLE_SIZE][KILLER_WIDTH];

static void killer_reset() {
	int i, j;
	for (i=0; i<KTABLE_SIZE; i++)
		for (j=0; j<KILLER_WIDTH; j++)
			killerTable[i][j].move_type = -1;
}

static void killer_sort(game g __attribute__((unused)),
			const move *succs, score *vals,
			int nsucc, int depth) {
	int i, k;

	/*
	 * Usamos también las killers de 2
	 * plies atrás.
	 * */
	for (k=0; k<KILLER_WIDTH; k++) {
		for (i=0; i<nsucc; i++) {
			if (equalMove(succs[i], killerTable[depth][k])) {
				vals[i] += KILLER_SCORE;
				break;
			}
		}
	}
}

static void killer_notify_cut(game g __attribute__((unused)),
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
		if (killerTable[depth][i].move_type == -1)
			continue;

		if (killerTable[depth][i].who != g->turn)
			continue;

		arr[c++] = killerTable[depth][i];
	}

	return c;
}

static struct addon killer_addon __attribute__((unused)) =
{
	.reset = killer_reset,
	.score_succs = killer_sort,
	.notify_cut = killer_notify_cut,
	.suggest = killer_suggest,
};

void addon_killer_init() {
#ifdef CFG_KILLER
	addon_register(killer_addon);
#else
	;
#endif
}
