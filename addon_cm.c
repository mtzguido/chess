#include "addon.h"
#include "addon_cm.h"
#include "board.h"
#include "moves.h"

static move counterTable[2][8][8][8][8];

void cm_reset() {
	move *arr = &counterTable[0][0][0][0][0];
	unsigned i;

	for (i = 0; i < sizeof counterTable / sizeof counterTable[0]; i++)
		arr[i].move_type = MOVE_INVAL;
}

void cm_score_succs(int depth) {
	int i;

	/* Find the counter move */
	const move * const l = &hstack[hply - 1].m;
	const move * const m = &counterTable[G->turn][l->r][l->c][l->R][l->C];

	if (m->move_type == MOVE_INVAL)
		return;

	for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
		if (equalMove(&gsuccs[i].m, m)) {
			gsuccs[i].s += CM_SCORE;
			break;
		}
	}
}

void cm_notify_cut(const move * const m, int depth) {
	const move * const om = &hstack[hply - 1].m;
	counterTable[G->turn][om->r][om->c][om->R][om->C] = *m;
}
