#include "board.h"
#include "succs.h"

#include <assert.h>

static void addToRet (game g, move m, game **arr, int *len);
static void addToRet2(game g, move m, game **arr, int *len);
static move makeRegularMove(int who, int r, int c, int R, int C);

int pawnSuccs(int r, int c, game g, game **arr, int *alen) {
	int n = *alen;
	if (g->turn == BLACK && r < 7) {
		if (g->board[r+1][c] == 0)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c), arr, alen);
		if (c < 7 && g->board[r+1][c+1] != 0 && colorOf(g->board[r+1][c+1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c+1), arr, alen);
		if (c > 0 && g->board[r+1][c-1] != 0 && colorOf(g->board[r+1][c-1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c-1), arr, alen);
		if (r < 7 && c > 0 && g->en_passant_x == r+1 && g->en_passant_y == c-1)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c-1), arr, alen);
		if (r < 7 && c < 7 && g->en_passant_x == r+1 && g->en_passant_y == c+1)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c+1), arr, alen);
	} else if (g->turn == WHITE && r > 0) {
		if (g->board[r-1][c] == 0)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c), arr, alen);
		if (c < 7 && g->board[r-1][c+1] != 0 && colorOf(g->board[r-1][c+1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c+1), arr, alen);
		if (c > 0 && g->board[r-1][c-1] != 0 && colorOf(g->board[r-1][c-1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c-1), arr, alen);
		if (r > 0 && c > 0 && g->en_passant_x == r-1 && g->en_passant_y == c-1)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c-1), arr, alen);
		if (r > 0 && c < 8 && g->en_passant_x == r-1 && g->en_passant_y == c+1)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c+1), arr, alen);
	}

	return *alen > n;
}

int genSuccs(game g, game **arr_ret) {
	int i, j;
	int alen, asz;
	game *arr;

	alen = 0;
	asz = 128;
	arr = malloc(asz * sizeof g);
	assert(arr != NULL);

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			char piece = g->board[i][j];

			if (piece == 0 || colorOf(piece) != g->turn)
				continue; /* Can't be moved */

			if (isPawn(piece)) {
				pawnSuccs(i, j, g, &arr, &alen);
			} else if (isKnight(piece)) {
				int di, dj;
				for (di=1; di<3; di++) {
					dj = 3-di;

					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen);

					di=-di;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen);

					di=-di;
				}
			} else if (isRook(piece) || isQueen(piece)) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
			} else if (isBishop(piece) || isQueen(piece)) {
				int k, l;

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen);
						}
						break;
					}
				}
			} else if (isKing(piece)) {
				if (i>0 && (g->board[i-1][j] == 0 || colorOf(g->board[i-1][j]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i-1, j), &arr, &alen);
				if (i<7 && (g->board[i+1][j] == 0 || colorOf(g->board[i+1][j]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i+1, j), &arr, &alen);
				if (j>0 && (g->board[i][j-1] == 0 || colorOf(g->board[i][j-1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i, j-1), &arr, &alen);                               
				if (j<7 && (g->board[i][j+1] == 0 || colorOf(g->board[i][j+1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i, j+1), &arr, &alen);
				if (i>0 && j>0 && (g->board[i-1][j-1] == 0 || colorOf(g->board[i-1][j-1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i-1, j-1), &arr, &alen);
				if (i<7 && j>0 && (g->board[i+1][j-1] == 0 || colorOf(g->board[i+1][j-1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i+1, j-1), &arr, &alen);
				if (i>0 && j<7 && (g->board[i-1][j+1] == 0 || colorOf(g->board[i-1][j+1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i-1, j+1), &arr, &alen);
				if (i<7 && j<7 && (g->board[i+1][j+1] == 0 || colorOf(g->board[i+1][j+1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i+1, j+1), &arr, &alen);
			}
			assert(arr != NULL);
		}
	}

	move m;

	m.who = g->turn;
	m.move_type = MOVE_KINGSIDE_CASTLE;
	if (isLegalMove(g, m))
		addToRet(g, m, &arr, &alen);

	m.move_type = MOVE_QUEENSIDE_CASTLE;
	if (isLegalMove(g, m))
		addToRet(g, m, &arr, &alen);

	*arr_ret = arr;

	assert(alen <= asz);

	return alen;
}


static void addToRet(game g, move m, game **arr, int *len) {
	if (! isLegalMove(g, m)) {
		return;
	}

	game t = copyGame(g);

	doMove(t, m);
	assert (!inCheck(t, g->turn));

	t->turn = flipTurn(g->turn);
	t->lastmove = m;

	(*arr)[*len] = t;
	(*len)++;
}

static void addToRet2(game g, move m, game **arr, int *len) {
	/* Aca agregamos ambos casos si es un peón que promueve.
	 * es una chanchada, si. */
	assert(isPawn(g->board[m.r][m.c]));

	if (m.move_type == MOVE_REGULAR
		&& m.R == (m.who == WHITE ? 0 : 7)) {
		/* Caballo */
		do {
			game t = copyGame(g);

			m.promote = WKNIGHT;
			if (! isLegalMove(t, m)) {
				freeGame(g);
				break;
			}

			doMove(t, m);
	assert (!inCheck(t, g->turn));

			t->turn = flipTurn(g->turn);
			t->lastmove = m;

			(*arr)[*len] = t;
			(*len)++;

		} while (0);

		/* Reina */
		do {
			game t = copyGame(g);

			m.promote = WQUEEN;
			if (! isLegalMove(t, m)) {
				freeGame(g);
				break;
			}

			doMove(t, m);
	assert (!inCheck(t, g->turn));

			t->turn = flipTurn(g->turn);
			t->lastmove = m;

			(*arr)[*len] = t;
			(*len)++;

		} while (0);
	} else {
		addToRet(g, m, arr, len);
	}
}

static move makeRegularMove(int who, int r, int c, int R, int C) {
	move ret;

	ret.who = who;
	ret.move_type = MOVE_REGULAR;
	ret.r = r;
	ret.c = c;
	ret.R = R;
	ret.C = C;
	ret.promote = 0;

	return ret;
}

int hasNextGame(game g) {
	int n;
	game *arr;

	/* medio choto esto */
	n = genSuccs(g, &arr);
	freeSuccs(arr, n);

	return n != 0;
}

