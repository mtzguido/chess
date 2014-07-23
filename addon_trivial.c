#include "addon.h"
#include "addon_trivial.h"

#include <stdio.h>

static void trivial_score_succs(game g, struct MS *ss,
				int nsucc, int depth) {
	int i;

	for (i=0; i<nsucc; i++) {
		move m = ss[i].m;

		if (m.move_type != MOVE_REGULAR)
			continue;

		if (m.promote != 0)
			ss[i].s += PROMOTE_SCORE;

		if (enemy_piece(g, m.R, m.C))
			ss[i].s += KILLER_SCORE;

		ss[i].s += mvv_lva(g->board[m.r][m.c], g->board[m.R][m.C]);
	}
}

static struct addon trivial_addon __maybe_unused =
{
	.score_succs = trivial_score_succs,
};

void addon_trivial_init() {
	addon_register(trivial_addon);
}
