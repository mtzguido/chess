#include "board.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static char charOf(int piece);
static int threatens(game g, int r, int c, int R, int C);
static int inCheck(game g, int who);

static struct game_struct
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
	.turn = WHITE,
	.lastmove = {0},
	.idlecount = 0,
	.w_castle_king = 1,
	.w_castle_queen = 1,
	.b_castle_king = 1,
	.b_castle_queen = 1,
	.en_passant_x = -1,
	.en_passant_y = -1
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

static int sign(int a) {
	if (a > 0) return 1;
	if (a < 0) return -1;
	return 0;
}

int doMove(game g, move m) {
	switch (m.move_type) {
	case MOVE_REGULAR:
		g->board[m.R][m.C] = g->board[m.r][m.c];
		g->board[m.r][m.c] = 0;
		break;

	default:
		assert("UNIMPLEMENTED" == NULL);
	}

	g->turn = flipTurn(g->turn);
	return 0;
}

int isFinished(game g) {
	game *arr;
	int i, n;

	n = genSuccs(g, &arr);

	for (i=0; i<n; i++)
		if (!inCheck(arr[i], g->turn)) {
			freeSuccs(arr, n);
			return -1;
		}

	freeSuccs(arr, n);
	assert(n == 0);
	if (inCheck(g, g->turn))
		return DRAW; /* Stalemate */
	else
		return WIN(flipTurn(g->turn)); /* Current player is checkmated */
}

static int inCheck(game g, int who) {
	int i, j;
	int found = 0;
	int kr, kc;

	for (i=0; !found && i<8; i++)
		for (j=0; !found && j<8; j++)
			if (isKing(g->board[i][j]) && colorOf(g->board[i][j]) == who) {
				kr = i;
				kc = j;
				found = 1;
			}

	assert(found);

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			if(g->board[i][j] != 0 && colorOf(g->board[i][j]) != who)
				if (threatens(g, i, j, kr, kc))
					return 1;

	return 0;
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
		if (!threatens(g, m.r, m.c, m.R, m.C)) {
			ret = 0;
			goto out;
		}

		break;
	default:
		assert("UNIMPLEMENTED" == NULL);
	}

	/* No podemos quedar en jaque */
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

static int threatens(game g, int r, int c, int R, int C) {
	if (r < 0 || r >= 8) return 0;
	if (R < 0 || R >= 8) return 0;
	if (c < 0 || c >= 8) return 0;
	if (C < 0 || C >= 8) return 0;
	if (r == R && c == C) return 0;

	if (isPawn(g->board[r][c])) {
		if (g->turn == WHITE) {
			if (c == C) {
				if (R == r-1 && g->board[R][C] == 0)
					return 1;
				else if (R == r-2 && r == 6 && g->board[r-1][c] == 0 && g->board[r-2][c] == 0)
					return 1;
				else
					return 0;
			} else if ((R == r-1) && (c == C+1 || c == C-1)) {
				if (g->board[R][C] != 0 && colorOf(g->board[R][C]) != g->turn) /* o en passant! */
					return 1;
				else
					return 0;
			}
		} else { /* g->turn == BLACK */
			if (c == C) {
				if (R == r+1 && g->board[R][C] == 0)
					return 1;
				else if (R == r+2 && r == 1 && g->board[r+1][c] == 0 && g->board[r+2][c] == 0)
					return 1;
				else
					return 0;
			} else if ((R == r+1) && (c == C+1 || c == C-1)) {
				if (g->board[R][C] != 0 && colorOf(g->board[R][C]) != g->turn) /* o en passant! */
					return 1;
				else
					return 0;
			}
		}
	} else if (isKnight(g->board[r][c])) {
		if (abs(r-R) + abs(c-C) != 3 || abs(r-R) == 0 || abs(c-C) == 0)
			return 0;

		if (g->board[R][C] == 0 || colorOf(g->board[R][C]) != g->turn)
			return 1;
		else
			return 0;
	} else if (isBishop(g->board[r][c])) {
		int dr, dc;
		int i, j;
		if (abs(r-R) != abs(c-C))
			return 0;

		dr = sign(R-r);
		dc = sign(C-c);

		for (j=c+dc, i=r+dr; j != C && i != R; i+=dr, j+=dc)
			if (g->board[i][j] != 0)
				return 0;

		if (g->board[R][C] != 0 && colorOf(g->board[R][C]) == g->turn)
			return 0;

		return 1;
	} else if (isRook(g->board[r][c])) {
		if (r == R) {
			int i;
			int dc = sign(C-c);
			for (i=c+dc; i != C; i += dc)
				if (g->board[r][i] != 0)
					return 0;

			if (g->board[R][C] != 0 && colorOf(g->board[R][C] == g->turn))
				return 0;
			
			return 1;
		} else if (c == C) {
			int i;
			int dr = sign(R-r);
			for (i=r+dr; i != R; i += dr)
				if (g->board[i][C] != 0)
					return 0;

			if (g->board[R][C] != 0 && colorOf(g->board[R][C] == g->turn))
				return 0;
			
			return 1;

		} else
			return 0;
	} else if (isQueen(g->board[r][c])) {
		if (abs(R-r) == abs(C-c)) {
			int dr, dc;
			int i, j;
			if (abs(r-R) != abs(c-C))
				return 0;

			dr = sign(R-r);
			dc = sign(C-c);

			for (j=c+dc, i=r+dr; j != C && i != R; i+=dr, j+=dc)
				if (g->board[i][j] != 0)
					return 0;

			if (g->board[R][C] != 0 && colorOf(g->board[R][C]) == g->turn)
				return 0;

			return 1;
		} else if (r == R) {
			int i;
			int dc = sign(C-c);
			for (i=c+dc; i != C; i += dc)
				if (g->board[r][i] != 0)
					return 0;

			if (g->board[R][C] != 0 && colorOf(g->board[R][C] == g->turn))
				return 0;
			
			return 1;
		} else if (c == C) {
			int i;
			int dr = sign(R-r);
			for (i=r+dr; i != R; i += dr)
				if (g->board[i][C] != 0)
					return 0;

			if (g->board[R][C] != 0 && colorOf(g->board[R][C] == g->turn))
				return 0;
			
			return 1;

		} else
			return 0;
	} else if (isKing(g->board[r][c])) {
		if (abs(C-c) > 1) return 0;
		if (abs(R-r) > 1) return 0;
		if (g->board[R][C] != 0 && colorOf(g->board[R][C]) == g->turn)
			return 0;

		return 1;
	} else {
		fprintf(stderr, "!!!!!!!! (%i)\n", g->board[r][c]);
	}

	return 0;
}

static void addToRet(game g, move m, game **arr, int *len, int *size) {
	game t = copyGame(g);
	doMove(t, m);
	t->turn = flipTurn(g->turn);
	t->lastmove = m;

	if (inCheck(t, g->turn)) {
		freeGame(t);
		return;
	}

	(*arr)[*len] = t;

	(*len)++;
	if (*len == *size) {
		(*size)*=2;
		(*arr)=realloc((*arr), (*size) * sizeof (game));
		assert(*arr != NULL);
	}
}

static move makeRegularMove(int r, int c, int R, int C) {
	move ret;
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
	asz = 32;
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
						addToRet(g, makeRegularMove(i, j, i+1, j), &arr, &alen, &asz);
					if (j < 7 && g->board[i+1][j+1] != 0&& colorOf(g->board[i+1][j+1]) != g->turn)
						addToRet(g, makeRegularMove(i, j, i+1, j+1), &arr, &alen, &asz);
					if (j > 0 && g->board[i+1][j-1] != 0&& colorOf(g->board[i+1][j-1]) != g->turn)
						addToRet(g, makeRegularMove(i, j, i+1, j-1), &arr, &alen, &asz);
				} else if (g->turn == WHITE && i > 0) {
					if (g->board[i-1][j] == 0)
						addToRet(g, makeRegularMove(i, j, i-1, j), &arr, &alen, &asz);
					if (j < 7 && g->board[i-1][j+1] != 0 && colorOf(g->board[i-1][j+1]) != g->turn)
						addToRet(g, makeRegularMove(i, j, i-1, j+1), &arr, &alen, &asz);
					if (j > 0 && g->board[i-1][j-1] != 0 && colorOf(g->board[i-1][j-1]) != g->turn)
						addToRet(g, makeRegularMove(i, j, i-1, j-1), &arr, &alen, &asz);
				}
			} else if (isKnight(piece)) {
				int di, dj;
				for (di=1; di<3; di++) {
					dj = 3-di;

					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(i, j, i+di, j+dj), &arr, &alen, &asz);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(i, j, i+di, j+dj), &arr, &alen, &asz);

					di=-di;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(i, j, i+di, j+dj), &arr, &alen, &asz);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g->board[i+di][j+dj] == 0 || colorOf(g->board[i+di][j+dj]) != g->turn))
						addToRet(g, makeRegularMove(i, j, i+di, j+dj), &arr, &alen, &asz);

					di=-di;
				}
			} else if (isRook(piece)) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
			} else if (isBishop(piece)) {
				int k, l;

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
			} else if (isQueen(piece)) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g->board[k][l] == 0)
						addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
					else {
						if (colorOf(g->board[k][l]) != g->turn) {
							addToRet(g, makeRegularMove(i, j, k, l), &arr, &alen, &asz);
						}
						break;
					}
				}
			} else if (isKing(piece)) {
				if (i>0 && (g->board[i-1][j] == 0 || colorOf(g->board[i-1][j]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i-1, j), &arr, &alen, &asz);
				if (i<7 && (g->board[i+1][j] == 0 || colorOf(g->board[i+1][j]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i+1, j), &arr, &alen, &asz);
				if (j>0 && (g->board[i][j-1] == 0 || colorOf(g->board[i][j-1]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i, j-1), &arr, &alen, &asz);                               
				if (j<7 && (g->board[i][j+1] == 0 || colorOf(g->board[i][j+1]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i, j+1), &arr, &alen, &asz);
				if (i>0 && j>0 && (g->board[i-1][j-1] == 0 || colorOf(g->board[i-1][j-1]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i-1, j-1), &arr, &alen, &asz);
				if (i<7 && j>0 && (g->board[i+1][j-1] == 0 || colorOf(g->board[i+1][j-1]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i+1, j-1), &arr, &alen, &asz);
				if (i>0 && j<7 && (g->board[i-1][j+1] == 0 || colorOf(g->board[i-1][j+1]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i-1, j+1), &arr, &alen, &asz);
				if (i<7 && j<7 && (g->board[i+1][j+1] == 0 || colorOf(g->board[i+1][j+1]) != g->turn))
					addToRet(g, makeRegularMove(i, j, i+1, j+1), &arr, &alen, &asz);
			}
			assert(arr != NULL);
		}
	}

	*arr_ret = arr;

	return alen;
}

void freeSuccs(game *arr, int len) {
	int i;

	for (i=0; i<len; i++)
		freeGame(arr[i]);

	free(arr);
}

