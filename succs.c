#include "board.h"
#include "succs.h"
#include "masks.h"

#include <stdio.h>
#include <assert.h>

#ifndef NDEBUG
static game dbg_caps = NULL;
#endif

static inline void addToRet_f(move m);
static inline void addToRet_promote_f(move m);

#define addToRet_promote(m)			\
	do {					\
		assert(m.who == g->turn);	\
		addToRet_promote_f(m);		\
	} while(0)

#define addToRet(m)							\
	do {								\
		assert(dbg_caps == NULL || isCapture(dbg_caps,m));	\
		assert(m.who == g->turn);				\
		addToRet_f(m);						\
	} while(0)

struct MS gsuccs[MAX_PLY*200];
int first_succ[MAX_PLY];
int ply;

static void pawnSuccs_w(i8 r, i8 c, const game g) {
	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	/* Por defecto, nos movemos 1 casilla */
	m.R = r-1;

	switch (r) {
	case 1:
		/* Última fila */
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 3:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 2 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m);
			m.R = r-1;
	 	}
		/* Fall through */
	case 2:
	case 4:
	case 5:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 6:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);

			if (g->board[r-2][c] == EMPTY) {
				m.R = r-2;
				addToRet(m);
				m.R = r-1;
			}
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void pawnSuccs_b(i8 r, i8 c, const game g) {
	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	/* Por defecto, nos movemos 1 casilla */
	m.R = r+1;

	switch (r) {
	case 6:
		/* Última fila */
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 4:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 5 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m);
			m.R = r+1;
		}
		/* Fall through */
	case 2:
	case 3:
	case 5:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 1:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);

			if (g->board[r+2][c] == EMPTY) {
				m.R = r+2;
				addToRet(m);
				m.R = r+1;
			}
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void pawnCaps_w(i8 r, i8 c, const game g) {
	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	/* Por defecto, nos movemos 1 casilla */
	m.R = r-1;

	switch (r) {
	case 1:
		/* Última fila */
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 3:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 2 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m);
			m.R = r-1;
	 	}
		/* Fall through */
	case 2:
	case 4:
	case 5:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 6:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void pawnCaps_b(i8 r, i8 c, const game g) {
	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	/* Por defecto, nos movemos 1 casilla */
	m.R = r+1;

	switch (r) {
	case 6:
		/* Última fila */
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 4:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 5 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m);
			m.R = r+1;
		}
		/* Fall through */
	case 2:
	case 3:
	case 5:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 1:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void knightSuccs(i8 r, i8 c, const game g) {
	const int dr[] = { 2,  2, -2, -2, 1,  1, -1, -1 };
	const int dc[] = { 1, -1,  1, -1, 2, -2,  2, -2 };
	unsigned i;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (i=0; i< sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (own_piece(g, m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void rookSuccs_col(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	R = r;
	for (C=c+1; C<8; C++) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (C=c-1; C>=0; C--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void rookSuccs_row(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	C = c;
	for (R=r+1; R<8; R++) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (R=r-1; R>=0; R--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void rookSuccs(i8 r, i8 c, const game g) {
	rookSuccs_row(r, c, g);
	rookSuccs_col(r, c, g);
}

static void bishopSuccs_diag1(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R=r+1, C=c-1; R<8 && C>=0; R++, C--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (R=r-1, C=c+1; R>=0 && C<8; R--, C++) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void bishopSuccs_diag2(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R=r+1, C=c+1; R<8 && C<8; R++, C++) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (R=r-1, C=c-1; R>=0 && C>=0; R--, C--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void bishopSuccs(i8 r, i8 c, const game g) {
	bishopSuccs_diag1(r, c, g);
	bishopSuccs_diag2(r, c, g);
}

void kingSuccs(i8 r, i8 c, const game g) {
	const int dr[] = {  1, 1,  1, 0, -1, -1, -1, 0  };
	const int dc[] = { -1, 0,  1, 1,  1,  0, -1, -1 };
	unsigned i;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (i=0; i < sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (own_piece(g, m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void queenSuccs(i8 r, i8 c, const game g) {
	rookSuccs(r, c, g);
	bishopSuccs(r, c, g);
}

static void knightCaps(i8 r, i8 c, const game g) {
	const int dr[] = { 2,  2, -2, -2, 1,  1, -1, -1 };
	const int dc[] = { 1, -1,  1, -1, 2, -2,  2, -2 };
	unsigned i;
	int other = flipTurn(g->turn);

	if (!(g->piecemask[other] & knight_mask[r*8+c]))
		return;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (i=0; i< sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (!enemy_piece(g, m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void rookCaps_row_w(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & row_w_mask[r*8+c]))
		return;

	R = r;
	for (C=c-1; C>=0; C--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void rookCaps_row_e(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & row_e_mask[r*8+c]))
		return;

	R = r;
	for (C=c+1; C<8; C++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void rookCaps_col_n(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & col_n_mask[r*8+c]))
		return;

	C = c;
	for (R=r-1; R>=0; R--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void rookCaps_col_s(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & col_s_mask[r*8+c]))
		return;

	C = c;
	for (R=r+1; R<8; R++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void rookCaps(i8 r, i8 c, const game g) {
	rookCaps_row_w(r, c, g);
	rookCaps_row_e(r, c, g);
	rookCaps_col_s(r, c, g);
	rookCaps_col_n(r, c, g);
}

static void bishopCaps_diag_nw(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & diag_nw_mask[r*8+c]))
		return;

	for (R=r-1, C=c-1; R>=0 && C>=0; R--, C--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag_ne(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & diag_ne_mask[r*8+c]))
		return;

	for (R=r-1, C=c+1; R>=0 && C<8; R--, C++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag_sw(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & diag_sw_mask[r*8+c]))
		return;

	for (R=r+1, C=c-1; R<8 && C>=0; R++, C--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag_se(i8 r, i8 c, const game g) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & diag_se_mask[r*8+c]))
		return;

	for (R=r+1, C=c+1; R<8 && C<8; R++, C++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void bishopCaps(i8 r, i8 c, const game g) {
	bishopCaps_diag_nw(r, c, g);
	bishopCaps_diag_sw(r, c, g);
	bishopCaps_diag_ne(r, c, g);
	bishopCaps_diag_se(r, c, g);
}

static void kingCaps(i8 r, i8 c, const game g) {
	const int dr[] = {  1, 1,  1, 0, -1, -1, -1, 0  };
	const int dc[] = { -1, 0,  1, 1,  1,  0, -1, -1 };
	const int other = flipTurn(g->turn);
	unsigned i;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[other] & king_mask[r*8+c]))
		return;

	for (i=0; i < sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (!enemy_piece(g, m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void queenCaps(i8 r, i8 c, const game g) {
	rookCaps(r, c, g);
	bishopCaps(r, c, g);
}

static void castleSuccs(const game g) {
	const piece_t kr = g->turn == WHITE ? 7 : 0;
	const piece_t rpiece = g->turn == WHITE ? WROOK : BROOK;
	move m = {0};

	m.who = g->turn;

	if (g->castle_king[g->turn]
	 && g->board[kr][5] == EMPTY
	 && g->board[kr][6] == EMPTY
	 && g->board[kr][7] == rpiece) {
		m.move_type = MOVE_KINGSIDE_CASTLE;
		addToRet(m);
	}

	if (g->castle_queen[g->turn]
	 && g->board[kr][0] == rpiece
	 && g->board[kr][1] == EMPTY
	 && g->board[kr][2] == EMPTY
	 && g->board[kr][3] == EMPTY) {
		m.move_type = MOVE_QUEENSIDE_CASTLE;
		addToRet(m);
	}
}

__always_inline
static void pieceSuccs(i8 i, i8 j, const game g) {
	const piece_t piece = g->board[i][j];

	switch (piece) {
	case WPAWN:
		pawnSuccs_w(i, j, g);
		break;
	case BPAWN:
		pawnSuccs_b(i, j, g);
		break;
	case WKNIGHT:
	case BKNIGHT:
		knightSuccs(i, j, g);
		break;
	case WROOK:
	case BROOK:
		rookSuccs(i, j, g);
		break;
	case WBISHOP:
	case BBISHOP:
		bishopSuccs(i, j, g);
		break;
	case WQUEEN:
	case BQUEEN:
		queenSuccs(i, j, g);
		break;
	case WKING:
	case BKING:
		kingSuccs(i, j, g);
		break;
	default:
		assert(0);
	}
}

__always_inline
static void pieceCaps(i8 i, i8 j, const game g) {
	const piece_t piece = g->board[i][j];

	switch (piece) {
	case WPAWN:
		pawnCaps_w(i, j, g);
		break;
	case BPAWN:
		pawnCaps_b(i, j, g);
		break;
	case WKNIGHT:
	case BKNIGHT:
		knightCaps(i, j, g);
		break;
	case WROOK:
	case BROOK:
		rookCaps(i, j, g);
		break;
	case WBISHOP:
	case BBISHOP:
		bishopCaps(i, j, g);
		break;
	case WQUEEN:
	case BQUEEN:
		queenCaps(i, j, g);
		break;
	case WKING:
	case BKING:
		kingCaps(i, j, g);
		break;
	default:
		assert(0);
	}
}

typedef void (*movegen_t)(i8 i, i8 j, const game g);

static void  __genSuccs(const game g, movegen_t fun) {
	u64 temp;
	int i;

	first_succ[ply+1] = first_succ[ply];

	mask_for_each(g->piecemask[g->turn], temp, i) {
		const u8 r = (i-1) / 8;
		const u8 c = (i-1) % 8;

		fun(r, c, g);
	}

}

void genSuccs(const game g) {
	__genSuccs(g, pieceSuccs);
	assert(first_succ[ply+1] > first_succ[ply]);
	castleSuccs(g);
}

void genCaps(const game g) {
#ifndef NDEBUG
	dbg_caps = g;
#endif

	__genSuccs(g, pieceCaps);

#ifndef NDEBUG
	dbg_caps = NULL;
#endif
}

static inline void _addToRet(move m) {
	assert(first_succ[ply+1] < (int)(sizeof gsuccs / sizeof gsuccs[0]));
	gsuccs[first_succ[ply+1]].m = m;
	gsuccs[first_succ[ply+1]].s = 0;
	first_succ[ply+1]++;
}

static inline void addToRet_f(move m) {
	assert(m.promote == EMPTY);
	_addToRet(m);
}

static inline void addToRet_promote_f(move m) {
	m.promote = WQUEEN;
	_addToRet(m);

	m.promote = WKNIGHT;
	_addToRet(m);
}
