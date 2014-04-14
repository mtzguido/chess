#include "board.h"
#include "succs.h"

#include <stdio.h>
#include <assert.h>

static inline void addToRet (move m, move *arr, int *len);
static inline void addToRet2(move m, move *arr, int *len);
static inline move makeRegularMove(int who, int r, int c, int R, int C);

void pawnSuccs(int r, int c, game g, move *arr, int *alen) {
	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	if (g->turn == BLACK && r < 7) {
		if (r == 6) {
			m.R = r+1; m.C = c;   addToRet2(m, arr, alen);
			m.R = r+1; m.C = c+1; addToRet2(m, arr, alen);
			m.R = r+1; m.C = c-1; addToRet2(m, arr, alen);
		} else {
			if (r == 1) {
				m.R = r+2; m.C = c;
				addToRet(m, arr, alen);
			}
			m.R = r+1; m.C = c;   addToRet(m, arr, alen);
			m.R = r+1; m.C = c+1; addToRet(m, arr, alen);
			m.R = r+1; m.C = c-1; addToRet(m, arr, alen);
		}

	} else if (g->turn == WHITE && r > 0) {
		if (r == 1) {
			m.R = r-1; m.C = c;   addToRet2(m, arr, alen);
			m.R = r-1; m.C = c+1; addToRet2(m, arr, alen);
			m.R = r-1; m.C = c-1; addToRet2(m, arr, alen);
		} else {
			if (r == 6) {
				m.R = r-2; m.C = c;
				addToRet(m, arr, alen);
			}
			m.R = r-1; m.C = c;   addToRet(m, arr, alen);
			m.R = r-1; m.C = c+1; addToRet(m, arr, alen);
			m.R = r-1; m.C = c-1; addToRet(m, arr, alen);
		}
	}
}

void knightSuccs(int r, int c, game g, move *arr, int *alen) {
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

		addToRet(m, arr, alen);
	}
}

void rookSuccs(int r, int c, game g, move *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	R = r;
	for (C=c+1; C<8; C++) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}
	for (C=c-1; C>=0; C--) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}

	C = c;
	for (R=r+1; R<8; R++) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}

	for (R=r-1; R>=0; R--) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}
}

void bishopSuccs(int r, int c, game g, move *arr, int *alen) {
	int R, C;

	move m = {0};
	m.who = g->turn;
	m.move_type = MOVE_REGULAR;
	m.r = r;
	m.c = c;

	for (R=r+1, C=c+1; R<8 && C<8; R++, C++) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}

	for (R=r-1, C=c+1; R>=0 && C<8; R--, C++) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}

	for (R=r+1, C=c-1; R<8 && C>=0; R++, C--) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}

	for (R=r-1, C=c-1; R>=0 && C>=0; R--, C--) {
		if (g->board[R][C] == 0) {
			m.R = R; m.C = C;
			addToRet(m, arr, alen);
		} else {
			if (colorOf(g->board[R][C]) != g->turn) {
				m.R = R; m.C = C;
				addToRet(m, arr, alen);
			}
			break;
		}
	}
}

void kingSuccs(int r, int c, game g, move *arr, int *alen) {
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

		addToRet(m, arr, alen);
	}
}

int genSuccs(game g, move **arr_ret) {
	int i, j;
	int alen, asz;
	move m = {0};
	move *arr;

	alen = 0;
	asz = 128;
	arr = malloc(asz * sizeof arr[0]);
	assert(arr != NULL);

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			i8 piece = g->board[i][j];

			if (piece == 0 || colorOf(piece) != g->turn)
				continue; /* Can't be moved */

			switch (piece) {
			case WPAWN:
			case BPAWN:
				pawnSuccs(i, j, g, arr, &alen);
				break;
			case WKNIGHT:
			case BKNIGHT:
				knightSuccs(i, j, g, arr, &alen);
				break;
			case WROOK:
			case BROOK:
				rookSuccs(i, j, g, arr, &alen);
				break;
			case WBISHOP:
			case BBISHOP:
				bishopSuccs(i, j, g, arr, &alen);
				break;
			case WQUEEN:
			case BQUEEN:
				rookSuccs(i, j, g, arr, &alen);
				bishopSuccs(i, j, g, arr, &alen);
				break;
			case WKING:
			case BKING:
				kingSuccs(i, j, g, arr, &alen);
				break;
			}
		}
	}

	m.who = g->turn;
	if (g->castle_king[g->turn]) {
		m.move_type = MOVE_KINGSIDE_CASTLE;
		addToRet(m, arr, &alen);
	}

	if (g->castle_queen[g->turn]) {
		m.move_type = MOVE_QUEENSIDE_CASTLE;
		addToRet(m, arr, &alen);
	}

	assert(alen <= asz);

	*arr_ret = arr;
	return alen;
}

static inline void addToRet(move m, move *arr, int *len) {
	arr[*len] = m;
	(*len)++;
}

static inline void addToRet2(move m, move *arr, int *len) {
	/*
	 * Aca agregamos ambos casos si es un peón que promueve.
	 * es una chanchada, si.
	 */
	int i;

	for (i=0; i<2; i++) {
		m.promote = i == 0 ? WQUEEN : WKNIGHT;

		arr[*len] = m;
		(*len)++;
	}
}

static inline move makeRegularMove(int who, int r, int c, int R, int C) {
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

void freeSuccs(move *arr, int len __attribute__((unused))) {
	free(arr);
}

