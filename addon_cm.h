#ifdef CFG_COUNTERMOVE
#include "board.h"

static move counterTable[2][8][8][8][8];

static void cm_init() {
	/* ugh... */
	int a, b, c, d, e;
	for (a=0; a<2; a++)
	for (b=0; b<8; b++)
	for (c=0; c<8; c++)
	for (d=0; d<8; d++)
	for (e=0; e<8; e++)
		counterTable[a][b][c][d][e].move_type = -1;
}

static void cm_sort(game g, game *succs, int nsucc, int depth __attribute__((unused))) {
	int i;

	/* Buscamos la counter move */
	move l = g->lastmove;
	move m = counterTable[g->turn][l.r][l.c][l.R][l.C];

	if (m.move_type == -1)
		return;

	for (i=0; i<nsucc; i++) {
		if (equalMove(succs[i]->lastmove, m)) {
			game temp;

			if (i > 0) {
				temp = succs[i];
				succs[i] = succs[0];
				succs[0] = temp;
			}

			break;
		}
	}
}

static void cm_notify_cut(game g, game next, int depth) {
	if (!next->lastmove.was_capture) {
		move m = g->lastmove;
		counterTable[g->turn][m.r][m.c][m.R][m.C] = next->lastmove;
	}
}

#else

static void cm_sort(game g, game *succs, int nsucc, int depth) { }
static void cm_init() { }
static void cm_notify_cut(game g, game next, int depth) { }

#endif

