#include "board.h"
#include "succs.h"
#include "masks.h"

#include <stdio.h>
#include <assert.h>

static inline void addToRet(move m, struct MS *arr, int *len);
static inline void addToRet_promote(move m, struct MS *arr, int *len);

static void pawnSuccs_w(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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
			addToRet_promote(m, arr, alen);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m, arr, alen);
		}
		return;
	case 3:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 2 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m, arr, alen);
			m.R = r-1;
	 	}
		/* Fall through */
	case 2:
	case 4:
	case 5:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m, arr, alen);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;
	case 6:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m, arr, alen);

			if (g->board[r-2][c] == EMPTY) {
				m.R = r-2;
				addToRet(m, arr, alen);
				m.R = r-1;
			}
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void pawnSuccs_b(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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
			addToRet_promote(m, arr, alen);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m, arr, alen);
		}
		return;
	case 4:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 5 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m, arr, alen);
			m.R = r+1;
		}
		/* Fall through */
	case 2:
	case 3:
	case 5:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m, arr, alen);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;
	case 1:
		if (g->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m, arr, alen);

			if (g->board[r+2][c] == EMPTY) {
				m.R = r+2;
				addToRet(m, arr, alen);
				m.R = r+1;
			}
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void pawnCaps_w(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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
			addToRet_promote(m, arr, alen);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m, arr, alen);
		}
		return;
	case 3:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 2 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m, arr, alen);
			m.R = r-1;
	 	}
		/* Fall through */
	case 2:
	case 4:
	case 5:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;
	case 6:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void pawnCaps_b(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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
			addToRet_promote(m, arr, alen);
		}

		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m, arr, alen);
		}
		return;
	case 4:
		/* Sólo acá puede haber e.p. */
		if (g->en_passant_x == 5 && abs(g->en_passant_y - c) == 1) {
			m.R = g->en_passant_x;
			m.C = g->en_passant_y;
			addToRet(m, arr, alen);
			m.R = r+1;
		}
		/* Fall through */
	case 2:
	case 3:
	case 5:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;
	case 1:
		if (c > 0 && enemy_piece(g, m.R, c-1)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (c < 7 && enemy_piece(g, m.R, c+1)) {
			m.C = c+1;
			addToRet(m, arr, alen);
		}

		return;

	case 0:
	case 7:
	default:
		assert(0);
	}
}

static void knightSuccs(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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

		addToRet(m, arr, alen);
	}
}

static void rookSuccs_col(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (C=c-1; C>=0; C--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void rookSuccs_row(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (R=r-1; R>=0; R--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void rookSuccs(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	rookSuccs_row(r, c, g, arr, alen);
	rookSuccs_col(r, c, g, arr, alen);
}

static void bishopSuccs_diag1(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R=r+1, C=c-1; R<8 && C>=0; R++, C--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (R=r-1, C=c+1; R>=0 && C<8; R--, C++) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void bishopSuccs_diag2(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R=r+1, C=c+1; R<8 && C<8; R++, C++) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}

	for (R=r-1, C=c-1; R>=0 && C>=0; R--, C--) {
		if (!own_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else break;
		if (enemy_piece(g, R, C))
			break;
	}
}

static void bishopSuccs(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	bishopSuccs_diag1(r, c, g, arr, alen);
	bishopSuccs_diag2(r, c, g, arr, alen);
}

void kingSuccs(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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

		addToRet(m, arr, alen);
	}
}

static void queenSuccs(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	rookSuccs(r, c, g, arr, alen);
	bishopSuccs(r, c, g, arr, alen);
}

static void knightCaps(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	const int dr[] = { 2,  2, -2, -2, 1,  1, -1, -1 };
	const int dc[] = { 1, -1,  1, -1, 2, -2,  2, -2 };
	unsigned i;
	int other = flipTurn(g->turn);

	if (!(g->piecemask[other] & knightmask[r*8+c]))
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

		addToRet(m, arr, alen);
	}
}

static void rookCaps_row(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & rowmask[r]))
		return;

	R = r;
	for (C=c+1; C<8; C++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}

	for (C=c-1; C>=0; C--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void rookCaps_col(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & colmask[c]))
		return;

	C = c;
	for (R=r+1; R<8; R++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}

	for (R=r-1; R>=0; R--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void rookCaps(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	rookCaps_row(r, c, g, arr, alen);
	rookCaps_col(r, c, g, arr, alen);
}

static void bishopCaps_diag1(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & diag1mask[r+c]))
		return;

	for (R=r+1, C=c-1; R<8 && C>=0; R++, C--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}

	for (R=r-1, C=c+1; R>=0 && C<8; R--, C++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag2(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(g->piecemask[flipTurn(g->turn)] & diag2mask[c-r+7]))
		return;

	for (R=r+1, C=c+1; R<8 && C<8; R++, C++) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}

	for (R=r-1, C=c-1; R>=0 && C>=0; R--, C--) {
		if (enemy_piece(g, R, C)) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
			break;
		} else if (own_piece(g, R, C)) {
			break;
		}
	}
}

static void bishopCaps(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	bishopCaps_diag1(r, c, g, arr, alen);
	bishopCaps_diag2(r, c, g, arr, alen);
}

static void kingCaps(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
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

		if (!enemy_piece(g, m.R, m.C))
			continue;

		addToRet(m, arr, alen);
	}
}

static void queenCaps(i8 r, i8 c, const game g, struct MS *arr, int *alen) {
	rookCaps(r, c, g, arr, alen);
	bishopCaps(r, c, g, arr, alen);
}

static void castleSuccs(const game g, struct MS *arr, int *alen) {
	const piece_t kr = g->turn == WHITE ? 7 : 0;
	const piece_t rpiece = g->turn == WHITE ? WROOK : BROOK;
	move m = {0};

	m.who = g->turn;

	if (g->castle_king[g->turn]
	 && g->board[kr][5] == EMPTY
	 && g->board[kr][6] == EMPTY
	 && g->board[kr][7] == rpiece) {
		m.move_type = MOVE_KINGSIDE_CASTLE;
		addToRet(m, arr, alen);
	}

	if (g->castle_queen[g->turn]
	 && g->board[kr][0] == rpiece
	 && g->board[kr][1] == EMPTY
	 && g->board[kr][2] == EMPTY
	 && g->board[kr][3] == EMPTY) {
		m.move_type = MOVE_QUEENSIDE_CASTLE;
		addToRet(m, arr, alen);
	}
}

__always_inline
static void pieceSuccs(i8 i, i8 j, const game g, struct MS *arr, int *alen) {
	const piece_t piece = g->board[i][j];

	switch (piece) {
	case WPAWN:
		pawnSuccs_w(i, j, g, arr, alen);
		break;
	case BPAWN:
		pawnSuccs_b(i, j, g, arr, alen);
		break;
	case WKNIGHT:
	case BKNIGHT:
		knightSuccs(i, j, g, arr, alen);
		break;
	case WROOK:
	case BROOK:
		rookSuccs(i, j, g, arr, alen);
		break;
	case WBISHOP:
	case BBISHOP:
		bishopSuccs(i, j, g, arr, alen);
		break;
	case WQUEEN:
	case BQUEEN:
		queenSuccs(i, j, g, arr, alen);
		break;
	case WKING:
	case BKING:
		kingSuccs(i, j, g, arr, alen);
		break;
	default:
		assert(0);
	}
}

__always_inline
static void pieceCaps(i8 i, i8 j, const game g, struct MS *arr, int *alen) {
	const piece_t piece = g->board[i][j];

	switch (piece) {
	case WPAWN:
		pawnCaps_w(i, j, g, arr, alen);
		break;
	case BPAWN:
		pawnCaps_b(i, j, g, arr, alen);
		break;
	case WKNIGHT:
	case BKNIGHT:
		knightCaps(i, j, g, arr, alen);
		break;
	case WROOK:
	case BROOK:
		rookCaps(i, j, g, arr, alen);
		break;
	case WBISHOP:
	case BBISHOP:
		bishopCaps(i, j, g, arr, alen);
		break;
	case WQUEEN:
	case BQUEEN:
		queenCaps(i, j, g, arr, alen);
		break;
	case WKING:
	case BKING:
		kingCaps(i, j, g, arr, alen);
		break;
	default:
		assert(0);
	}
}

typedef void (*movegen_t)(i8 i, i8 j, const game g, struct MS *arr, int *alen);

static int __genSuccs(const game g, struct MS **arr_ret, movegen_t fun) {
	int i;
	int alen, asz;
	struct MS *arr;
	u8 rows[32], cols[32];
	int npcs;

	alen = 0;
	asz = 100;
	arr = calloc(asz, sizeof arr[0]);
	assert(arr != NULL);

	npcs = on_bits(g->piecemask[g->turn], rows, cols);

	if (copts.reverse && fun == pieceSuccs) {
		for (i=npcs-1; i>=0; i--) {
			const u8 r = rows[i];
			const u8 c = cols[i];

			fun(r, c, g, arr, &alen);
		}
	} else {
		for (i=0; i<npcs; i++) {
			const u8 r = rows[i];
			const u8 c = cols[i];

			fun(r, c, g, arr, &alen);
		}
	}

	castleSuccs(g, arr, &alen);

	if (alen > asz) {
		fprintf(stderr, " !!!!!! TODO MAL !!!!! \n");
		fprintf(stderr, " !!!!!! TODO MAL !!!!! \n");
		printBoard(g);
		fprintf(stderr, " !!!!!! TODO MAL !!!!! \n");
		fprintf(stderr, " !!!!!! TODO MAL !!!!! \n");
		abort();
	}

	*arr_ret = arr;
	return alen;
}

int genSuccs(const game g, struct MS **arr_ret) {
	return __genSuccs(g, arr_ret, pieceSuccs);
}

int genCaps(const game g, struct MS **arr_ret) {
	return __genSuccs(g, arr_ret, pieceCaps);
}

static inline void _addToRet(move m, struct MS *arr, int *len) {
	arr[*len].m = m;
	arr[*len].s = 0;

	(*len)++;
}

static inline void addToRet(move m, struct MS *arr, int *len) {
	assert(m.promote == EMPTY);
	_addToRet(m, arr, len);
}

static inline void addToRet_promote(move m, struct MS *arr, int *len) {
	m.promote = WQUEEN;
	_addToRet(m, arr, len);

	m.promote = WKNIGHT;
	_addToRet(m, arr, len);
}

void freeSuccs(struct MS *arr, int len __maybe_unused) {
	free(arr);
}
