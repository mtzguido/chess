#include "addon.h"
#include "addon_cm.h"
#include "board.h"
#include "moves.h"

static move counterTable[2][8][8][8][8];

void cm_reset() {
	/* ugh... */
	int a, b, c, d, e;
	for (a=0; a<2; a++)
	for (b=0; b<8; b++)
	for (c=0; c<8; c++)
	for (d=0; d<8; d++)
	for (e=0; e<8; e++)
		counterTable[a][b][c][d][e].move_type = MOVE_INVAL;
}

void cm_score_succs(int depth) {
	int i;

	/* Buscamos la counter move */
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
