#include "check.h"
#include "board.h"
#include "masks.h"
#include "common.h"

static bool inCheck_row_e(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_r = mkEnemyPiece(WROOK, who);

	if (!(G->piecemask[flipTurn(who)] & row_e_mask[kr*8+kc]))
		return false;

	i = kr;
	for (j = kc+1; j < 8; j++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_row_w(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_r = mkEnemyPiece(WROOK, who);

	if (!(G->piecemask[flipTurn(who)] & row_w_mask[kr*8+kc]))
		return false;

	i = kr;
	for (j = kc-1; j >= 0; j--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_col_s(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_r = mkEnemyPiece(WROOK, who);

	if (!(G->piecemask[flipTurn(who)] & col_s_mask[kr*8+kc]))
		return 0;

	j = kc;
	for (i = kr+1; i < 8; i++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_col_n(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_r = mkEnemyPiece(WROOK, who);

	if (!(G->piecemask[flipTurn(who)] & col_n_mask[kr*8+kc]))
		return 0;

	j = kc;
	for (i = kr-1; i >= 0; i--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_sw(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_b = mkEnemyPiece(WBISHOP, who);

	if (!(G->piecemask[flipTurn(who)] & diag_sw_mask[kr*8+kc]))
		return false;

	j = kc;
	for (i = kr + 1, j = kc - 1; i < 8 && j >= 0; i++, j--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_nw(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_b = mkEnemyPiece(WBISHOP, who);

	if (!(G->piecemask[flipTurn(who)] & diag_nw_mask[kr*8+kc]))
		return false;

	for (i = kr - 1, j = kc - 1; i >= 0 && j >= 0; i--, j--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_se(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_b = mkEnemyPiece(WBISHOP, who);

	if (!(G->piecemask[flipTurn(who)] & diag_se_mask[kr*8+kc]))
		return false;

	for (i = kr + 1, j = kc + 1; i < 8 && j < 8; i++, j++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_ne(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = mkEnemyPiece(WQUEEN, who);
	const piece_t enemy_b = mkEnemyPiece(WBISHOP, who);

	if (!(G->piecemask[flipTurn(who)] & diag_ne_mask[kr*8+kc]))
		return false;

	for (i = kr - 1, j = kc + 1; i >= 0 && j < 8; i--, j++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_knig(int kr, int kc, int who) {
	const piece_t enemy_kn = mkEnemyPiece(WKNIGHT, who);

	if (!(G->piecemask[flipTurn(who)] & knight_mask[kr*8+kc]))
		return false;

	/* Caballos */
	if (kr >= 2 && kc >= 1 && G->board[kr-2][kc-1] == enemy_kn) return true;
	if (kr <= 5 && kc >= 1 && G->board[kr+2][kc-1] == enemy_kn) return true;
	if (kr >= 2 && kc <= 6 && G->board[kr-2][kc+1] == enemy_kn) return true;
	if (kr <= 5 && kc <= 6 && G->board[kr+2][kc+1] == enemy_kn) return true;
	if (kr >= 1 && kc >= 2 && G->board[kr-1][kc-2] == enemy_kn) return true;
	if (kr <= 6 && kc >= 2 && G->board[kr+1][kc-2] == enemy_kn) return true;
	if (kr >= 1 && kc <= 5 && G->board[kr-1][kc+2] == enemy_kn) return true;
	if (kr <= 6 && kc <= 5 && G->board[kr+1][kc+2] == enemy_kn) return true;

	return false;
}

static bool inCheck_pawn(int kr, int kc, int who) {
	if (who == WHITE) {
		if (kr > 0) {
			if (kc > 0 && G->board[kr - 1][kc - 1] == BPAWN)
				return true;
			if (kc < 7 && G->board[kr - 1][kc + 1] == BPAWN)
				return true;
		}

		return 0;
	} else {
		if (kr < 7) {
			if (kc > 0 && G->board[kr + 1][kc - 1] == WPAWN)
				return true;
			if (kc < 7 && G->board[kr + 1][kc + 1] == WPAWN)
				return true;
		}

		return false;
	}
}

static bool inCheck_king() {
	/* Just by looking at the distance between kings */

	return     abs(G->kingx[0] - G->kingx[1]) <= 1
		&& abs(G->kingy[0] - G->kingy[1]) <= 1;
}

bool inCheck() {
	const int who = G->turn;
	u8 kr, kc;
	int idx;

	if (G->inCheck[who] != -1)
		return G->inCheck[who];

	idx = 8 * G->kingx[who] + G->kingy[who];
	if (!(G->piecemask[flipTurn(who)] & all_mask[idx])) {
		G->inCheck[who] = 0;
		return 0;
	}

	kr = G->kingx[who];
	kc = G->kingy[who];

	G->inCheck[who] =  inCheck_diag_sw(kr, kc, who)
			|| inCheck_diag_se(kr, kc, who)
			|| inCheck_diag_nw(kr, kc, who)
			|| inCheck_diag_ne(kr, kc, who)
			|| inCheck_row_w(kr, kc, who)
			|| inCheck_row_e(kr, kc, who)
			|| inCheck_col_n(kr, kc, who)
			|| inCheck_col_s(kr, kc, who)
			|| inCheck_knig(kr, kc, who)
			|| inCheck_pawn(kr, kc, who)
			|| inCheck_king();

	return G->inCheck[who];
}
