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
		counterTable[a][b][c][d][e].move_type = MOVE_INVAL;
}

static void cm_score_succs(game g, struct MS *ss,
			   int nsucc, int depth) {
	int i;

	/* Buscamos la counter move */
	move l = g->lastmove;
	move m = counterTable[g->turn][l.r][l.c][l.R][l.C];

	if (m.move_type == MOVE_INVAL)
		return;

	for (i=0; i<nsucc; i++) {
		if (equalMove(ss[i].m, m)) {
			ss[i].s += CM_SCORE;
			break;
		}
	}
}

static void cm_notify_cut(game g, move m, int depth) {
	move om = g->lastmove;
	counterTable[g->turn][om.r][om.c][om.R][om.C] = m;
}

static int cm_suggest(game g, move *arr, int depth) {
	move m = g->lastmove;
	move cm = counterTable[g->turn][m.r][m.c][m.R][m.C];

	if (cm.move_type == MOVE_INVAL)
		return 0;

	if (cm.who != g->turn)
		return 0;

	*arr = cm;
	return 1;
}

static struct addon cm_addon __maybe_unused = {
	.reset = cm_reset,
	.score_succs = cm_score_succs,
	.notify_cut = cm_notify_cut,
	.suggest = cm_suggest,
};

void addon_cm_init() {
	addon_register(cm_addon);
}
