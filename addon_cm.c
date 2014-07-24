#include "addon.h"
#include "addon_cm.h"
#include "board.h"

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

void cm_score_succs(game g, int depth) {
	int i;

	/* Buscamos la counter move */
	move l = g->lastmove;
	move m = counterTable[g->turn][l.r][l.c][l.R][l.C];

	if (m.move_type == MOVE_INVAL)
		return;

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		if (equalMove(gsuccs[i].m, m)) {
			gsuccs[i].s += CM_SCORE;
			break;
		}
	}
}

void cm_notify_cut(game g, move m, int depth) {
	move om = g->lastmove;
	counterTable[g->turn][om.r][om.c][om.R][om.C] = m;
}

void cm_suggest(game g, move *arr, int *n, int depth) {
	move m = g->lastmove;
	move cm = counterTable[g->turn][m.r][m.c][m.R][m.C];

	if (cm.move_type == MOVE_INVAL)
		return;

	if (cm.who != g->turn)
		return;

	arr[*n++] = cm;
}
