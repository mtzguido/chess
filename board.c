#include "board.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "move.h"

static char charOf(int piece);
static int threatens(game g, int r, int c, int R, int C);
static int inCheck(game g, int who);

static const struct game_struct
init = {
	.board= {
		{ BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK },
		{ BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN },
		{ WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK }
	},
	.kingx = { 0, 7 },
	.kingy = { 4, 4 },
	.turn = WHITE,
	.lastmove = { 0 },
	.idlecount = 0,
	.castle_king = { 1, 1 },
	.castle_queen = { 1, 1 },
	.en_passant_x = -1,
	.en_passant_y = -1,
	.inCheck = { 0, 0 }
};

game startingGame() {
	game g = malloc(sizeof(*g));
	*g = init;
	return g;
}

game copyGame(game g) {
	game ret = malloc(sizeof (*ret));
	*ret = *g;
	return ret;
}

void freeGame(game g) {
	free(g);
}

void printBoard(game g) {
	int i, j;

	printf("(turn: %s)\n", g->turn == WHITE ? "WHITE" : "BLACK");
	for (i=0; i<8; i++) {
		printf("   ");
		for (j=0; j<8; j++) {
			putchar(charOf(g->board[i][j]));
			putchar(charOf(g->board[i][j]));
			putchar(charOf(g->board[i][j]));
			putchar(' ');
			putchar(' ');
			putchar(' ');
		}
		putchar('\n');
		printf("%i  ", 8-i);
		for (j=0; j<8; j++) {
			putchar(charOf(g->board[i][j]));
			putchar(charOf(g->board[i][j]));
			putchar(charOf(g->board[i][j]));
			putchar(' ');
			putchar(' ');
			putchar(' ');
		}
		putchar('\n');
		putchar('\n');
	}

	printf("\n    a     b     c     d     e     f     g     h\n");
	printf("[ castle_king = %i %i \n", g->castle_king[0], g->castle_king[1]);
	printf("  castle_queen = %i %i \n", g->castle_queen[0], g->castle_queen[1]);
	printf("  kingx = %i %i \n", g->kingx[0], g->kingx[1]);
	printf("  kingy = %i %i \n", g->kingy[0], g->kingy[1]);
	printf("  inCheck = %i %i \n", g->inCheck[0], g->inCheck[1]);
	printf("]\n");


	fflush(stdout);
}

static char charOf(int piece) {
	switch (piece) {
		case EMPTY:   return '.';
		case WPAWN:   return 'P';
		case BPAWN:   return 'p';
		case WROOK:   return 'R';
		case BROOK:   return 'r';
		case WKNIGHT: return 'N';
		case BKNIGHT: return 'n';
		case WBISHOP: return 'B';
		case BBISHOP: return 'b';
		case WQUEEN:  return 'Q';
		case BQUEEN:  return 'q';
		case WKING:	  return 'K';
		case BKING:	  return 'k';
		default:      assert(0);
	}
}

/* True if a change in (r,c) can never cause
 * a threat to (kr, kc). Having a piece type is useless
 * because something like
 * B . . .
 * . N . .
 * . . k .
 * . . . .
 * may occur. Where moving the knight could cause a check
 * even when it's not the piece who threatens the king */
static int safe (int r, int c, int kr, int kc) {
	if (r == kr || c == kc) return 1; /* misma fila o columna */
	if (abs(r-c) == abs(kr-kc)) return 1; /* misma diagonal */
	if (abs(r-kr) - abs(c-kc) == 3) return 1; /* caballos */
	return 0;
}

int doMove(game g, move m) {

	switch (m.move_type) {
	case MOVE_REGULAR: {
		int8_t piece = g->board[m.r][m.c];
		if (isKing(piece)) {
			g->kingx[colorOf(piece)] = m.R;
			g->kingy[colorOf(piece)] = m.C;
			g->castle_king[m.who] = 0;
			g->castle_queen[m.who] = 0;
		}

		if (m.r == m.who*7) {
			if (m.c == 7)
				g->castle_king[m.who] = 0;
			else if (m.c == 0)
				g->castle_queen[m.who] = 0;
		}

		g->board[m.R][m.C] = g->board[m.r][m.c];
		g->board[m.r][m.c] = 0;

		if (!safe(m.r, m.c, g->kingx[WHITE], g->kingy[WHITE]) ||
			!safe(m.R, m.C, g->kingx[WHITE], g->kingy[WHITE]))
			g->inCheck[WHITE] = 0;

		if (!safe(m.r, m.c, g->kingx[BLACK], g->kingy[BLACK]) ||
			!safe(m.R, m.C, g->kingx[BLACK], g->kingy[BLACK]))
			g->inCheck[BLACK] = 0;

		break;
	}
	case MOVE_KINGSIDE_CASTLE:
		g->castle_king[m.who] = 0;
		g->inCheck[flipTurn(m.who)] = 0;

		if (m.who == WHITE) {
			g->board[7][4] = EMPTY;
			g->board[7][5] = WROOK;
			g->board[7][6] = WKING;
			g->board[7][7] = EMPTY;
			g->kingx[WHITE] = 7;
			g->kingy[WHITE] = 6;
		} else {
			g->board[0][4] = EMPTY;
			g->board[0][5] = BROOK;
			g->board[0][6] = BKING;
			g->board[0][7] = EMPTY;
			g->kingx[BLACK] = 0;
			g->kingy[BLACK] = 6;
		}
		break;
	case MOVE_QUEENSIDE_CASTLE:
		g->castle_queen[m.who] = 0;
		g->inCheck[flipTurn(m.who)] = 0;

		if (m.who == WHITE) {
			g->board[7][0] = EMPTY;
			g->board[7][1] = EMPTY;
			g->board[7][2] = WKING;
			g->board[7][3] = WROOK;
			g->board[7][4] = EMPTY;
			g->kingx[WHITE] = 7;
			g->kingy[WHITE] = 2;
		} else {
			g->board[0][0] = EMPTY;
			g->board[0][1] = EMPTY;
			g->board[0][2] = BKING;
			g->board[0][3] = BROOK;
			g->board[0][4] = EMPTY;
			g->kingx[BLACK] = 0;
			g->kingy[BLACK] = 2;
		}
		break;


	default:
		assert("UNIMPLEMENTED" == NULL);
	}

	/* Sabemos con certeza que no podemos estar en jaque,
	 * ya que la movida es válida (isLegalMove debe haber
	 * retornado true para poder llamar a doMove). */
	g->inCheck[m.who] = 1;
	g->turn = flipTurn(g->turn);

	return 0;
}

int isFinished(game g) {
	game *arr;
	int n;

	/* medio choto esto */
	n = genSuccs(g, &arr);
	freeSuccs(arr, n);

	if (n != 0)
		return -1;
	else if (inCheck(g, g->turn))
		return DRAW; /* Stalemate */
	else
		return WIN(flipTurn(g->turn)); /* Current player is checkmated */
}

static int inCheck(game g, int who) {
	int kr, kc;
	int i, j;

	if (g->inCheck[who] != 0)
		return g->inCheck[who] - 1;

	kr = g->kingx[who];
	kc = g->kingy[who];

	if (threatens(g, kr-2, kc-1, kr, kc)) goto ret_true;
	if (threatens(g, kr+2, kc-1, kr, kc)) goto ret_true;
	if (threatens(g, kr-2, kc+1, kr, kc)) goto ret_true;
	if (threatens(g, kr+2, kc+1, kr, kc)) goto ret_true;
	if (threatens(g, kr-1, kc-2, kr, kc)) goto ret_true;
	if (threatens(g, kr+1, kc-2, kr, kc)) goto ret_true;
	if (threatens(g, kr-1, kc+2, kr, kc)) goto ret_true;
	if (threatens(g, kr+1, kc+2, kr, kc)) goto ret_true;
	for (i=0; i<8; i++)
		if (threatens(g, i, kc, kr, kc)) goto ret_true;
		else if (g->board[i][kc] != 0) break;
	for (i=0; i<8; i++)
		if (threatens(g, kr, i, kr, kc)) goto ret_true;
		else if (g->board[kr][i] != 0) break;

	for (i=kr-1, j=kc-1; i>=0 && j>=0; i--, j--)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;
	for (i=kr+1, j=kc+1; i<8 && j<8; i++, j++)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;
	for (i=kr+1, j=kc-1; i<8 && j>=0; i++, j--)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;
	for (i=kr-1, j=kc+1; i>=0 && j<8; i--, j++)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;

	g->inCheck[who] = 1;
	return 0;

ret_true:
	g->inCheck[who] = 2;
	return 1;
}

static inline int threatens(game g, int r, int c, int R, int C) {
	return r >= 0 && r < 8 
	    && R >= 0 && R < 8 
	    && c >= 0 && c < 8 
	    && C >= 0 && C < 8 
	    && g->board[r][c]*g->board[R][C] < 0 
	    && canMove(g, r, c, R, C);
}

int isLegalMove(game g, move m) {
	game ng = copyGame(g);
	int ret;

	switch (m.move_type) {
	case MOVE_REGULAR:
		/* Siempre se mueve una pieza propia */
		if (g->board[m.r][m.c] == 0 || colorOf(g->board[m.r][m.c]) != g->turn) {
			ret = 0;
			goto out;
		}

		/* La pieza debe poder moverse al destino */
		if (!canMove(g, m.r, m.c, m.R, m.C)) {
			ret = 0;
			goto out;
		}

		break;
	case MOVE_KINGSIDE_CASTLE:
		if (m.who == WHITE) {
			if (!(g->castle_king[WHITE] && ! inCheck(g, WHITE)
			 && g->board[7][7] == WROOK && g->board[7][6] == EMPTY
			 && g->board[7][5] == EMPTY && g->board[7][4] == WKING)) {
				ret = 0;
				goto out;
			}
		} else {
			if (!(g->castle_king[BLACK] && ! inCheck(g, BLACK)
			 && g->board[0][7] == BROOK && g->board[0][6] == EMPTY
			 && g->board[0][5] == EMPTY && g->board[0][4] == BKING)) {
				ret = 0;
				goto out;
			}
		}
		break;
	case MOVE_QUEENSIDE_CASTLE:
		if (m.who == WHITE) {
			if (!(g->castle_queen[WHITE] && ! inCheck(g, WHITE)
			 && g->board[7][0] == WROOK && g->board[7][1] == EMPTY
			 && g->board[7][2] == EMPTY && g->board[7][3] == EMPTY
			 && g->board[7][4] == WKING)) {
				ret = 0;
				goto out;
			}
		} else {
			if (!(g->castle_queen[BLACK] && ! inCheck(g, BLACK)
			 && g->board[0][0] == BROOK && g->board[0][1] == EMPTY
			 && g->board[0][2] == EMPTY && g->board[0][3] == EMPTY
			 && g->board[0][4] == BKING)) {
				ret = 0;
				goto out;
			}
		}
		break;


	default:
		assert("UNIMPLEMENTED" == NULL);
	}

	/* Nunca podemos quedar en jaque */
	doMove(ng, m);
	if (inCheck(ng, g->turn)) {
		ret = 0;
		goto out;
	}

	ret = 1;

out:
	freeGame(ng);
	return ret;
}

static void addToRet(game g, move m, game **arr, int *len, int *size) {
	game t = copyGame(g);
	doMove(t, m);

	if (inCheck(t, g->turn)) {
		freeGame(t);
		return;
	}

	t->turn = flipTurn(g->turn);
	t->lastmove = m;

	(*arr)[*len] = t;

	(*len)++;
	if (*len == *size) {
		(*size)*=2;
		(*arr)=realloc((*arr), (*size) * sizeof (game));
		assert(*arr != NULL);
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

	return ret;
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
				if (g->turn == BLACK && i < 7) {
					if (g->board[i+1][j] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, i+1, j), &arr, &alen, &asz);
					if (j < 7 && g->board[i+1][j+1] != 0&& colorOf(g->board[i+1][j+1]) != g->turn)
						addToRet(g, makeRegularMove(g->turn, i, j, i+1, j+1), &arr, &alen, &asz);
					if (j > 0 && g->board[i+1][j-1] != 0&& colorOf(g->board[i+1][j-1]) != g->turn)
						addToRet(g, makeRegularMove(g->turn, i, j, i+1, j-1), &arr, &alen, &asz);
				} else if (g->turn == WHITE && i > 0) {
					if (g->board[i-1][j] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, i-1, j), &arr, &alen, &asz);
					if (j < 7 && g->board[i-1][j+1] != 0 && colorOf(g->board[i-1][j+1]) != g->turn)
						addToRet(g, makeRegularMove(g->turn, i, j, i-1, j+1), &arr, &alen, &asz);
					if (j > 0 && g->board[i-1][j-1] != 0 && colorOf(g->board[i-1][j-1]) != g->turn)
						addToRet(g, makeRegularMove(g->turn, i, j, i-1, j-1), &arr, &alen, &asz);
				}
			} else if (isKnight(piece)) {
				int di, dj;
				for (di=1; di<3; di++) {
					dj = 3-di;

					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen, &asz);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen, &asz);

					di=-di;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen, &asz);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(g->turn, i, j, i+di, j+dj), &arr, &alen, &asz);

					di=-di;
				}
			} else if (isRook(piece)) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
			} else if (isBishop(piece)) {
				int k, l;

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
			} else if (isQueen(piece)) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(g->turn, i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
			} else if (isKing(piece)) {
				if (i>0 && (g->board[i-1][j] == 0 || colorOf(g->board[i-1][j]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i-1, j), &arr, &alen, &asz);
				if (i<7 && (g->board[i+1][j] == 0 || colorOf(g->board[i+1][j]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i+1, j), &arr, &alen, &asz);
				if (j>0 && (g->board[i][j-1] == 0 || colorOf(g->board[i][j-1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i, j-1), &arr, &alen, &asz);                               
				if (j<7 && (g->board[i][j+1] == 0 || colorOf(g->board[i][j+1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i, j+1), &arr, &alen, &asz);
				if (i>0 && j>0 && (g->board[i-1][j-1] == 0 || colorOf(g->board[i-1][j-1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i-1, j-1), &arr, &alen, &asz);
				if (i<7 && j>0 && (g->board[i+1][j-1] == 0 || colorOf(g->board[i+1][j-1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i+1, j-1), &arr, &alen, &asz);
				if (i>0 && j<7 && (g->board[i-1][j+1] == 0 || colorOf(g->board[i-1][j+1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i-1, j+1), &arr, &alen, &asz);
				if (i<7 && j<7 && (g->board[i+1][j+1] == 0 || colorOf(g->board[i+1][j+1]) != g->turn))
					addToRet(g, makeRegularMove(g->turn, i, j, i+1, j+1), &arr, &alen, &asz);
			}
			assert(arr != NULL);
		}
	}

	move m;

	m.who = g->turn;
	m.move_type = MOVE_KINGSIDE_CASTLE;
	if (isLegalMove(g, m))
		addToRet(g, m, &arr, &alen, &asz);

	m.move_type = MOVE_QUEENSIDE_CASTLE;
	if (isLegalMove(g, m))
		addToRet(g, m, &arr, &alen, &asz);

	*arr_ret = arr;

	return alen;
}

void freeSuccs(game *arr, int len) {
	int i;

	for (i=0; i<len; i++)
		freeGame(arr[i]);

	free(arr);
}

