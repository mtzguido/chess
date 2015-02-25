#include "board.h"
#include "succs.h"
#include "masks.h"
#include "moves.h"

#include <stdio.h>
#include <assert.h>

#ifndef NDEBUG
static game dbg_caps = NULL;
#endif

static inline void addToRet_f(move m);
static inline void addToRet_promote_f(move m);

#define addToRet_promote(m)			\
	do {					\
		assert(m.who == G->turn);	\
		addToRet_promote_f(m);		\
	} while(0)

#define addToRet(m)							\
	do {								\
		assert(dbg_caps == NULL || isCapture(m));		\
		assert(m.who == G->turn);				\
		addToRet_f(m);						\
	} while(0)

struct MS gsuccs[MAX_PLY*200];
int first_succ[MAX_PLY];
int ply;

static void pawnSuccs_w(i8 r, i8 c) {
	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	m.R = r - 1;

	switch (r) {
	case 1:
		/* Final row */
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 3:
		/* Only here can we have e.p. */
		if (G->en_passant_x == 2 && abs(G->en_passant_y - c) == 1) {
			m.R = G->en_passant_x;
			m.C = G->en_passant_y;
			addToRet(m);
			m.R = r-1;
	 	}
		/* Fall through */
	case 2:
	case 4:
	case 5:
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 6:
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);

			if (G->board[r-2][c] == EMPTY) {
				m.R = r-2;
				addToRet(m);
				m.R = r-1;
			}
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
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

static void pawnSuccs_b(i8 r, i8 c) {
	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	m.R = r+1;

	switch (r) {
	case 6:
		/* Final row */
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 4:
		/* Only here can we have e.p. */
		if (G->en_passant_x == 5 && abs(G->en_passant_y - c) == 1) {
			m.R = G->en_passant_x;
			m.C = G->en_passant_y;
			addToRet(m);
			m.R = r+1;
		}
		/* Fall through */
	case 2:
	case 3:
	case 5:
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 1:
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet(m);

			if (G->board[r+2][c] == EMPTY) {
				m.R = r+2;
				addToRet(m);
				m.R = r+1;
			}
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
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

static void pawnCaps_w(i8 r, i8 c) {
	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	m.R = r-1;

	switch (r) {
	case 1:
		/* Final row */
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 3:
		/* Only here can we have e.p. */
		if (G->en_passant_x == 2 && abs(G->en_passant_y - c) == 1) {
			m.R = G->en_passant_x;
			m.C = G->en_passant_y;
			addToRet(m);
			m.R = r-1;
	 	}
		/* Fall through */
	case 2:
	case 4:
	case 5:
		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 6:
		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
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

static void pawnCaps_b(i8 r, i8 c) {
	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	m.R = r+1;

	switch (r) {
	case 6:
		/* Final row */
		if (G->board[m.R][c] == EMPTY) {
			m.C = c;
			addToRet_promote(m);
		}

		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet_promote(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet_promote(m);
		}
		return;
	case 4:
		/* Only here can we have e.p. */
		if (G->en_passant_x == 5 && abs(G->en_passant_y - c) == 1) {
			m.R = G->en_passant_x;
			m.C = G->en_passant_y;
			addToRet(m);
			m.R = r+1;
		}
		/* Fall through */
	case 2:
	case 3:
	case 5:
		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
			m.C = c+1;
			addToRet(m);
		}

		return;
	case 1:
		if (c > 0 && enemy_piece(m.R, c-1)) {
			m.C = c-1;
			addToRet(m);
		}

		if (c < 7 && enemy_piece(m.R, c+1)) {
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

static void knightSuccs(i8 r, i8 c) {
	const int dr[] = { 2,  2, -2, -2, 1,  1, -1, -1 };
	const int dc[] = { 1, -1,  1, -1, 2, -2,  2, -2 };
	unsigned i;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (i = 0; i < sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (own_piece(m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void rookSuccs_col(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	R = r;
	for (C = c + 1; C < 8; C++) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}

	for (C = c - 1; C >= 0; C--) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}
}

static void rookSuccs_row(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	C = c;
	for (R = r + 1; R < 8; R++) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}

	for (R = r - 1; R >= 0; R--) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}
}

static void rookSuccs(i8 r, i8 c) {
	rookSuccs_row(r, c);
	rookSuccs_col(r, c);
}

static void bishopSuccs_diag1(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R = r + 1, C = c - 1; R < 8 && C >= 0; R++, C--) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}

	for (R = r - 1, C = c + 1; R >= 0 && C < 8; R--, C++) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}
}

static void bishopSuccs_diag2(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R = r + 1, C = c + 1; R < 8 && C < 8; R++, C++) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}

	for (R = r - 1, C = c - 1; R >= 0 && C >= 0; R--, C--) {
		if (!own_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
		} else break;
		if (enemy_piece(R, C))
			break;
	}
}

static void bishopSuccs(i8 r, i8 c) {
	bishopSuccs_diag1(r, c);
	bishopSuccs_diag2(r, c);
}

void kingSuccs(i8 r, i8 c) {
	const int dr[] = {  1, 1,  1, 0, -1, -1, -1, 0  };
	const int dc[] = { -1, 0,  1, 1,  1,  0, -1, -1 };
	unsigned i;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (i = 0; i < sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (own_piece(m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void queenSuccs(i8 r, i8 c) {
	rookSuccs(r, c);
	bishopSuccs(r, c);
}

static void knightCaps(i8 r, i8 c) {
	const int dr[] = { 2,  2, -2, -2, 1,  1, -1, -1 };
	const int dc[] = { 1, -1,  1, -1, 2, -2,  2, -2 };
	unsigned i;
	int other = flipTurn(G->turn);

	if (!(G->piecemask[other] & knight_mask[r*8+c]))
		return;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (i = 0; i < sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (!enemy_piece(m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void rookCaps_row_w(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & row_w_mask[r*8+c]))
		return;

	R = r;
	for (C = c - 1; C >= 0; C--) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void rookCaps_row_e(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & row_e_mask[r*8+c]))
		return;

	R = r;
	for (C = c + 1; C < 8; C++) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void rookCaps_col_n(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & col_n_mask[r*8+c]))
		return;

	C = c;
	for (R = r - 1; R >= 0; R--) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void rookCaps_col_s(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & col_s_mask[r*8+c]))
		return;

	C = c;
	for (R = r + 1; R < 8; R++) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void rookCaps(i8 r, i8 c) {
	rookCaps_row_w(r, c);
	rookCaps_row_e(r, c);
	rookCaps_col_s(r, c);
	rookCaps_col_n(r, c);
}

static void bishopCaps_diag_nw(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & diag_nw_mask[r*8+c]))
		return;

	for (R = r - 1, C = c - 1; R >= 0 && C >= 0; R--, C--) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag_ne(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & diag_ne_mask[r*8+c]))
		return;

	for (R = r - 1, C = c + 1; R >= 0 && C < 8; R--, C++) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag_sw(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & diag_sw_mask[r*8+c]))
		return;

	for (R = r + 1, C = c - 1; R < 8 && C >= 0; R++, C--) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void bishopCaps_diag_se(i8 r, i8 c) {
	int R, C;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[flipTurn(G->turn)] & diag_se_mask[r*8+c]))
		return;

	for (R = r + 1, C = c + 1; R < 8 && C < 8; R++, C++) {
		if (enemy_piece(R, C)) {
			m.R = R; m.C = C;
			addToRet(m);
			break;
		} else if (own_piece(R, C)) {
			break;
		}
	}
}

static void bishopCaps(i8 r, i8 c) {
	bishopCaps_diag_nw(r, c);
	bishopCaps_diag_sw(r, c);
	bishopCaps_diag_ne(r, c);
	bishopCaps_diag_se(r, c);
}

static void kingCaps(i8 r, i8 c) {
	const int dr[] = {  1, 1,  1, 0, -1, -1, -1, 0  };
	const int dc[] = { -1, 0,  1, 1,  1,  0, -1, -1 };
	const int other = flipTurn(G->turn);
	unsigned i;

	move m = {0};
	m.who = G->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (!(G->piecemask[other] & king_mask[r*8+c]))
		return;

	for (i = 0; i < sizeof dr / sizeof dr[0]; i++) {
		m.R = r + dr[i];
		m.C = c + dc[i];

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (!enemy_piece(m.R, m.C))
			continue;

		addToRet(m);
	}
}

static void queenCaps(i8 r, i8 c) {
	rookCaps(r, c);
	bishopCaps(r, c);
}

static void castleSuccs() {
	const piece_t kr = G->turn == WHITE ? 7 : 0;
	const piece_t rpiece = mkPiece(WROOK, G->turn);
	move m = {0};

	m.who = G->turn;

	if (G->castle_king[G->turn]
	 && G->board[kr][5] == EMPTY
	 && G->board[kr][6] == EMPTY
	 && G->board[kr][7] == rpiece) {
		m.move_type = MOVE_KINGSIDE_CASTLE;
		addToRet(m);
	}

	if (G->castle_queen[G->turn]
	 && G->board[kr][0] == rpiece
	 && G->board[kr][1] == EMPTY
	 && G->board[kr][2] == EMPTY
	 && G->board[kr][3] == EMPTY) {
		m.move_type = MOVE_QUEENSIDE_CASTLE;
		addToRet(m);
	}
}

__always_inline
static void pieceSuccs(i8 i, i8 j) {
	const piece_t piece = G->board[i][j];

	switch (piece) {
	case WPAWN:
		pawnSuccs_w(i, j);
		break;
	case BPAWN:
		pawnSuccs_b(i, j);
		break;
	case WKNIGHT:
	case BKNIGHT:
		knightSuccs(i, j);
		break;
	case WROOK:
	case BROOK:
		rookSuccs(i, j);
		break;
	case WBISHOP:
	case BBISHOP:
		bishopSuccs(i, j);
		break;
	case WQUEEN:
	case BQUEEN:
		queenSuccs(i, j);
		break;
	case WKING:
	case BKING:
		kingSuccs(i, j);
		break;
	default:
		assert(0);
	}
}

__always_inline
static void pieceCaps(i8 i, i8 j) {
	const piece_t piece = G->board[i][j];

	switch (piece) {
	case WPAWN:
		pawnCaps_w(i, j);
		break;
	case BPAWN:
		pawnCaps_b(i, j);
		break;
	case WKNIGHT:
	case BKNIGHT:
		knightCaps(i, j);
		break;
	case WROOK:
	case BROOK:
		rookCaps(i, j);
		break;
	case WBISHOP:
	case BBISHOP:
		bishopCaps(i, j);
		break;
	case WQUEEN:
	case BQUEEN:
		queenCaps(i, j);
		break;
	case WKING:
	case BKING:
		kingCaps(i, j);
		break;
	default:
		assert(0);
	}
}

typedef void (*movegen_t)(i8 i, i8 j);

static void  __genSuccs(movegen_t fun) {
	u64 temp;
	int i;

	first_succ[ply+1] = first_succ[ply];

	mask_for_each(G->piecemask[G->turn], temp, i) {
		const u8 r = bitrow(i);
		const u8 c = bitcol(i);

		fun(r, c);
	}
}

void genSuccs() {
	__genSuccs(pieceSuccs);
	assert(first_succ[ply+1] > first_succ[ply]);
	castleSuccs();
}

void genCaps() {
#ifndef NDEBUG
	dbg_caps = G;
#endif

	__genSuccs(pieceCaps);

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
