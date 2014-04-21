#include "board.h"
#include "succs.h"

#include <stdio.h>
#include <assert.h>

static inline void addToRet(move m, move *arr, int *len);
static inline void addToRet_promote(move m, move *arr, int *len);

void pawnSuccs_w(int r, int c, game g, move *arr, int *alen) {
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

		if (enemy_of(g->board[m.R][c-1], g->turn)) {
			m.C = c-1;
			addToRet_promote(m, arr, alen);
		}

		if (enemy_of(g->board[m.R][c+1], g->turn)) {
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

		if (enemy_of(g->board[m.R][c-1], g->turn)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (enemy_of(g->board[m.R][c+1], g->turn)) {
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

		if (enemy_of(g->board[m.R][c-1], g->turn)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (enemy_of(g->board[m.R][c+1], g->turn)) {
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

void pawnSuccs_b(int r, int c, game g, move *arr, int *alen) {
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

		if (enemy_of(g->board[m.R][c-1], g->turn)) {
			m.C = c-1;
			addToRet_promote(m, arr, alen);
		}

		if (enemy_of(g->board[m.R][c+1], g->turn)) {
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

		if (enemy_of(g->board[m.R][c-1], g->turn)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (enemy_of(g->board[m.R][c+1], g->turn)) {
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

		if (enemy_of(g->board[m.R][c-1], g->turn)) {
			m.C = c-1;
			addToRet(m, arr, alen);
		}

		if (enemy_of(g->board[m.R][c+1], g->turn)) {
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

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		if (g->board[m.R][m.C] != 0 && colorOf(g->board[m.R][m.C]) == g->turn)
			continue;

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

		if (m.R < 0 || m.R > 7)
			continue;

		if (m.C < 0 || m.C > 7)
			continue;

		addToRet(m, arr, alen);
	}
}

int genSuccs(game g, move **arr_ret) {
	int i, j;
	int alen, asz;
	move *arr;
	static int amax = 0;

	alen = 0;
	asz = 128;
	arr = malloc(asz * sizeof arr[0]);
	assert(arr != NULL);

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			i8 piece = g->board[i][j];

			if (piece == EMPTY)
				continue; /* Can't be moved */

			switch (piece) {
			case WPAWN:
				pawnSuccs_w(i, j, g, arr, &alen);
				break;
			case BPAWN:
				pawnSuccs_b(i, j, g, arr, &alen);
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

	{
		move m = {0};
		const int kr = g->turn == WHITE ? 7 : 0;

		m.who = g->turn;
		if (g->castle_king[g->turn]
		 && g->board[kr][5] == EMPTY
		 && g->board[kr][6] == EMPTY) {
			m.move_type = MOVE_KINGSIDE_CASTLE;
			addToRet(m, arr, &alen);
		}
	
		if (g->castle_queen[g->turn]
		 && g->board[kr][1] == EMPTY
		 && g->board[kr][2] == EMPTY
		 && g->board[kr][3] == EMPTY) {
			m.move_type = MOVE_QUEENSIDE_CASTLE;
			addToRet(m, arr, &alen);
		}
	}

	assert(alen <= asz);

	if (alen > amax) {
		amax = alen;
		fprintf(stderr, "amax = %i\n", amax);
	}

	*arr_ret = arr;
	return alen;
}

static inline void addToRet(move m, move *arr, int *len) {
	arr[*len] = m;
	(*len)++;
}

static inline void addToRet_promote(move m, move *arr, int *len) {
	/*
	 * Aca agregamos ambos casos si es un peón que promueve.
	 * es una chanchada, si.
	 */
	m.promote = WQUEEN;
	addToRet(m, arr, len);

	m.promote = WKNIGHT;
	addToRet(m, arr, len);
}

void freeSuccs(move *arr, int len __maybe_unused) {
	free(arr);
}
