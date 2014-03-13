#include "board.h"
#include "succs.h"

#include <stdio.h>
#include <assert.h>

static void addToRet (game g, move m, game **arr, int *len);
static void addToRet2(game g, move m, game **arr, int *len);
static move makeRegularMove(int who, int r, int c, int R, int C);

int pawnSuccs(int r, int c, game g, game **arr, int *alen) {
	int n = *alen;

	if (g->turn == BLACK && r < 7) {
		if (g->board[r+1][c] == 0)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c), arr, alen);
		if (r == 1 && g->board[r+1][c] == 0 && g->board[r+2][c] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, r+2, c), arr, alen);
		if (c < 7 && g->board[r+1][c+1] != 0 && colorOf(g->board[r+1][c+1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c+1), arr, alen);
		if (c > 0 && g->board[r+1][c-1] != 0 && colorOf(g->board[r+1][c-1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r+1, c-1), arr, alen);
		if (r < 7 && c > 0 && g->en_passant_x == r+1 && g->en_passant_y == c-1)
			addToRet(g, makeRegularMove(g->turn, r, c, r+1, c-1), arr, alen);
		if (r < 7 && c < 7 && g->en_passant_x == r+1 && g->en_passant_y == c+1)
			addToRet(g, makeRegularMove(g->turn, r, c, r+1, c+1), arr, alen);
	} else if (g->turn == WHITE && r > 0) {
		if (g->board[r-1][c] == 0)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c), arr, alen);
		if (r == 6 && g->board[r-1][c] == 0 && g->board[r-2][c] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, r-2, c), arr, alen);
		if (c < 7 && g->board[r-1][c+1] != 0 && colorOf(g->board[r-1][c+1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c+1), arr, alen);
		if (c > 0 && g->board[r-1][c-1] != 0 && colorOf(g->board[r-1][c-1]) != g->turn)
			addToRet2(g, makeRegularMove(g->turn, r, c, r-1, c-1), arr, alen);
		if (r > 0 && c > 0 && g->en_passant_x == r-1 && g->en_passant_y == c-1)
			addToRet(g, makeRegularMove(g->turn, r, c, r-1, c-1), arr, alen);
		if (r > 0 && c < 8 && g->en_passant_x == r-1 && g->en_passant_y == c+1)
			addToRet(g, makeRegularMove(g->turn, r, c, r-1, c+1), arr, alen);
	}

	return *alen > n;
}

int knightSuccs(int r, int c, game g, game **arr, int *alen) {
	int R, C;
	int i;
	int n = *alen;
	const int dr[] = { 2,  2, -2, -2, 1,  1, -1, -1 };
	const int dc[] = { 1, -1,  1, -1, 2, -2,  2, -2 };

	for (i=0; i< sizeof dr / sizeof dr[0]; i++) {
		R = r + dr[i];
		C = c + dc[i];

		if (R >= 0 && R < 8 && C >= 0 && C < 8)
			if (g->board[R][C] == 0 || colorOf(g->board[R][C] != g->turn))
					addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
	}

	return *alen > n;
}

int rookSuccs(int r, int c, game g, game **arr, int *alen) {
	int R, C;
	int n = *alen;

	R = r;
	for (C=c+1; C<8; C++) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}
	for (C=c-1; C>=0; C--) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}

	C = c;
	for (R=r+1; R<8; R++) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}
	for (R=r-1; R>=0; R--) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}

	return *alen > n;
}

int bishopSuccs(int r, int c, game g, game **arr, int *alen) {
	int R, C;
	int n = *alen;

	for (R=r+1, C=c+1; R<8 && C<8; R++, C++) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}
	for (R=r-1, C=c+1; R>=0 && C<8; R--, C++) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}
	for (R=r+1, C=c-1; R<8 && C>=0; R++, C--) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}
	for (R=r-1, C=c-1; R>=0 && C>=0; R--, C--) {
		if (g->board[R][C] == 0)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
		else {
			if (colorOf(g->board[R][C]) != g->turn) {
				addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
			}
			break;
		}
	}

	return *alen > n;
}

int kingSuccs(int r, int c, game g, game **arr, int *alen) {
	int n = *alen;

	int i;
	const int dr[] = {  1, 1,  1, 0, -1, -1, -1, 0  };
	const int dc[] = { -1, 0,  1, 1,  1,  0, -1, -1 };

	for (i=0; i < sizeof dr / sizeof dr[0]; i++) {
		register int R = r + dr[i];
		register int C = c + dc[i];

		if (R >= 0 && R < 8 && C >= 0 && C < 8)
			addToRet(g, makeRegularMove(g->turn, r, c, R, C), arr, alen);
	}

	return *alen > n;
}

int genSuccs(game g, game **arr_ret) {
	int i, j;
	int alen, asz;
	game *arr;
	move m = {0};

	alen = 0;
	asz = 128;
	arr = malloc(asz * sizeof g);
	assert(arr != NULL);

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			char piece = g->board[i][j];

			if (piece == 0 || colorOf(piece) != g->turn)
				continue; /* Can't be moved */
			else if (isPawn(piece))
				pawnSuccs(i, j, g, &arr, &alen);
			else if (isKnight(piece))
				knightSuccs(i, j, g, &arr, &alen);
			else if (isRook(piece))
				rookSuccs(i, j, g, &arr, &alen);
			else if (isBishop(piece))
				bishopSuccs(i, j, g, &arr, &alen);
			else if (isQueen(piece)) {
				rookSuccs(i, j, g, &arr, &alen);
				bishopSuccs(i, j, g, &arr, &alen);
			} else if (isKing(piece)) 
				kingSuccs(i, j, g, &arr, &alen);

			assert(arr != NULL);
		}
	}

	m.who = g->turn;
	m.move_type = MOVE_KINGSIDE_CASTLE;
	if (g->castle_king[g->turn])
		addToRet(g, m, &arr, &alen);

	m.move_type = MOVE_QUEENSIDE_CASTLE;
	if (g->castle_queen[g->turn])
		addToRet(g, m, &arr, &alen);

	*arr_ret = arr;

	assert(alen <= asz);

	return alen;
}

static void addToRet(game g, move m, game **arr, int *len) {
	game t = copyGame(g);

	if (!doMove(t, m)) {
		freeGame(t);
		return;
	}

	(*arr)[*len] = t;
	(*len)++;
}

static void addToRet2(game g, move m, game **arr, int *len) {
	/* Aca agregamos ambos casos si es un peón que promueve.
	 * es una chanchada, si. */
	assert(isPawn(g->board[m.r][m.c]));

	if (m.R == (m.who == WHITE ? 0 : 7)) {
		/* Caballo */
		do {
			game t = copyGame(g);

			m.promote = WKNIGHT;
			if (!doMove(t, m)) {
				freeGame(t);
				break;
			}

			(*arr)[*len] = t;
			(*len)++;
		} while (0);

		/* Reina */
		do {
			game t = copyGame(g);

			m.promote = WQUEEN;
			if (!doMove(t, m)) {
				freeGame(t);
				break;
			}

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
	int i, j;
	int alen, asz;
	game *arr;

	alen = 0;
	asz = 128;
	arr = malloc(asz * sizeof g);
	assert(arr != NULL);

	int ret = 1;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			char piece = g->board[i][j];

			if (piece == 0 || colorOf(piece) != g->turn) {
				continue; /* Can't be moved */
			} else if (isPawn(piece)) {
				if (pawnSuccs(i, j, g, &arr, &alen))
					goto out;
			} else if (isKnight(piece)) {
				if (knightSuccs(i, j, g, &arr, &alen))
					goto out;
			} else if (isRook(piece)) {
				if (rookSuccs(i, j, g, &arr, &alen))
					goto out;
			} else if (isBishop(piece)) {
				if (bishopSuccs(i, j, g, &arr, &alen))
					goto out;
			} else if (isQueen(piece)) {
				if (rookSuccs(i, j, g, &arr, &alen))
					goto out;

				if (bishopSuccs(i, j, g, &arr, &alen))
					goto out;
			} else if (isKing(piece)) {
				if (kingSuccs(i, j, g, &arr, &alen))
					goto out;
			}

			assert(arr != NULL);
		}
	}

	move m;

	m.who = g->turn;

	m.move_type = MOVE_KINGSIDE_CASTLE;
	addToRet(g, m, &arr, &alen);

	m.move_type = MOVE_QUEENSIDE_CASTLE;
	addToRet(g, m, &arr, &alen);

	ret = 0;

out:
	assert(alen <= asz);
	freeSuccs(arr, alen);
	return ret;
}

