#include "addon.h"
#include "addon_trivial.h"

#include <stdio.h>

/* Most valuable victim / least valuable attacker */
static inline int mvv_lva(piece_t a, piece_t v) {
	return 10 * (v&7) - (a&7);
}

void trivial_score_succs(int depth) {
	int i;

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		move m = gsuccs[i].m;

		if (m.move_type != MOVE_REGULAR)
			continue;

		if (m.promote != 0) {
			assert(isPawn(G->board[m.r][m.c]));
			assert(m.R == (G->turn == WHITE ? 0 : 7));
			gsuccs[i].s += PROMOTE_SCORE;
		}

		if (enemy_piece(G, m.R, m.C)) {
			gsuccs[i].s += CAPT_SCORE;
			gsuccs[i].s += mvv_lva(G->board[m.r][m.c], G->board[m.R][m.C]);
		}
	}
}
