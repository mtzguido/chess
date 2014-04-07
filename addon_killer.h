#ifdef CFG_KILLER
#include "board.h"

#define NKILLER	2

#ifdef CFG_DEPTH_EXTENSION
 #define KTABLE_SIZE (SEARCH_DEPTH + EXTRA_CHECK + EXTRA_CAPTURE + EXTRA_PROMOTION)
#else
 #define KTABLE_SIZE (SEARCH_DEPTH)
#endif

static move killerTable[KTABLE_SIZE][NKILLER];

static void killer_init() {
	int i, j;
	for (i=0; i<KTABLE_SIZE; i++)
		for (j=0; j<NKILLER; j++)
			killerTable[i][j].move_type = -1;
}

static void killer_sort(game g, game *succs, int nsucc, int depth) {
	int kindex = 0;
	int i, k;

	/* Ordenamos las killer move al
	 * principio
	 *
	 * usamos también las killers de 2
	 * plies atrás. */

	for (i=0; i<nsucc; i++) {
		for (k=0; k<NKILLER; k++) {
			if (equalMove(succs[i]->lastmove, killerTable[depth][k])) {
				game temp;

				if (i > kindex) {
					temp = succs[i];
					succs[i] = succs[kindex];
					succs[kindex] = temp;
				}
				kindex++;

				break;
			}
		}
	}
}

static void killer_notify_cut(game g, game next, int depth) {
	int i;

	if (depth > KTABLE_SIZE)
		return;

	for (i=0; i<NKILLER; i++)
		if (equalMove(killerTable[depth][i], next->lastmove))
			return;

	for (i=NKILLER-1; i >= 1; i--)
		killerTable[depth][i] = killerTable[depth][i-1];

	killerTable[depth][0] = next->lastmove;
}

#else

static void killer_init() { }
static void killer_sort(game g, game *succs, int nsucc, int depth) { }
static void killer_notify_cut(game g, game next, int depth) { }

#endif

