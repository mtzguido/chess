#include "addon_cm.h"
#include "addon.h"

#define CM_SCORE 500

static move counterTable[2][8][8][8][8];

static void cm_reset() {
	/* ugh... */
	int a, b, c, d, e;
	for (a=0; a<2; a++)
	for (b=0; b<8; b++)
	for (c=0; c<8; c++)
	for (d=0; d<8; d++)
	for (e=0; e<8; e++)
		counterTable[a][b][c][d][e].move_type = -1;
}

static void cm_score_succs(game g, const game *succs, score *vals,
			   int nsucc, int depth) {
	int i;

	/* Buscamos la counter move */
	move l = g->lastmove;
	move m = counterTable[g->turn][l.r][l.c][l.R][l.C];

	if (m.move_type == -1)
		return;

	for (i=0; i<nsucc; i++) {
		if (equalMove(succs[i]->lastmove, m)) {
			vals[i] += CM_SCORE;
			break;
		}
	}
}

static void cm_notify_cut(game g, move m, int depth) {
	if (!m.was_capture) {
		move om = g->lastmove;
		counterTable[g->turn][om.r][om.c][om.R][om.C] = m;
	}
}

static struct addon cm_addon __attribute__((unused)) = {
	.reset = cm_reset,
	.score_succs = cm_score_succs,
	.notify_cut = cm_notify_cut
};

void addon_cm_init() {
#ifdef CFG_COUNTERMOVE
	addon_register(cm_addon);
#else
	;
#endif
}
