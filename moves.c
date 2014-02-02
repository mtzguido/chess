#include "moves.h"
#include "board.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static int threatens(game g, int r, int c, int R, int C);
static int inCheck(game g, int who);

static int sign(int a) {
	if (a > 0) return 1;
	if (a < 0) return -1;
	return 0;
}

game doMove(game g, int r, int c, int R, int C) {
	game ret = g;

	ret.board[R][C] = g.board[r][c];
	ret.board[r][c] = 0;

	ret.turn = flipTurn(g.turn);

	return ret;
}

int isFinished(game g) {
	int i, j;
	int wk = 0, bk = 0;
	game *arr;
	int n;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g.board[i][j] == WKING)
				wk = 1;
			if (g.board[i][j] == BKING)
				bk = 1;
		}
	}

	if (!wk || !bk) {
		printBoard(g);
		assert("LOLWHAT"==NULL);
	}

	n = genSuccs(g, &arr);

	for (i=0; i<n; i++)
		if (!inCheck(arr[i], g.turn)) {
			free(arr);
			return -1;
		}

	free(arr);
	return flipTurn(g.turn);
}

static int inCheck(game g, int who) {
	int i, j;
	int found = 0;
	int kr, kc;

	for (i=0; !found && i<8; i++)
		for (j=0; !found && j<8; j++)
			if (isKing(g.board[i][j]) && colorOf(g.board[i][j]) == who) {
				kr = i;
				kc = j;
				found = 1;
			}

	assert(found);

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			if(g.board[i][j] != 0 && colorOf(g.board[i][j]) != who)
				if (threatens(g, i, j, kr, kc))
					return 1;

	return 0;
}

int isLegalMove(game g, int r, int c, int R, int C) {
	game ng;

	/* siempre se mueve una pieza propia */
	if (g.board[r][c] == 0 || colorOf(g.board[r][c]) != g.turn) {
		return 0;
	}

	/* la pieza debe poder moverse al destino */
	if (!threatens(g, r, c, R, C))
		return 0;

	/* no podemos quedar en jaque */
	ng = doMove(g, r, c, R, C);
	if (inCheck(ng, g.turn)) {
		printf("IS IN CHEKCKKK!\n");
		return 0;
	}

	return 1;
}

static int threatens(game g, int r, int c, int R, int C) {
	if (r < 0 || r >= 8) return 0;
	if (R < 0 || R >= 8) return 0;
	if (c < 0 || c >= 8) return 0;
	if (C < 0 || C >= 8) return 0;
	if (r == R && c == C) return 0;

	if (isPawn(g.board[r][c])) {
		if (g.turn == WHITE) {
			if (c == C) {
				if (R == r-1 && g.board[R][C] == 0)
					return 1;
				else if (R == r-2 && r == 6 && g.board[r-1][c] == 0 && g.board[r-2][c] == 0)
					return 1;
				else
					return 0;
			} else if ((R == r-1) && (c == C+1 || c == C-1)) {
				if (g.board[R][C] != 0 && colorOf(g.board[R][C]) != g.turn) /* o en passant! */
					return 1;
				else
					return 0;
			}
		} else { /* g.turn == BLACK */
			if (c == C) {
				if (R == r+1 && g.board[R][C] == 0)
					return 1;
				else if (R == r+2 && r == 1 && g.board[r+1][c] == 0 && g.board[r+2][c] == 0)
					return 1;
				else
					return 0;
			} else if ((R == r+1) && (c == C+1 || c == C-1)) {
				if (g.board[R][C] != 0 && colorOf(g.board[R][C]) != g.turn) /* o en passant! */
					return 1;
				else
					return 0;
			}
		}
	} else if (isKnight(g.board[r][c])) {
		if (abs(r-R) + abs(c-C) != 3 || abs(r-R) == 0 || abs(c-C) == 0)
			return 0;

		if (g.board[R][C] == 0 || colorOf(g.board[R][C]) != g.turn)
			return 1;
		else
			return 0;
	} else if (isBishop(g.board[r][c])) {
		int dr, dc;
		int i, j;
		if (abs(r-R) != abs(c-C))
			return 0;

		dr = sign(R-r);
		dc = sign(C-c);

		for (j=c+dc, i=r+dr; j != C && i != R; i+=dr, j+=dc)
			if (g.board[i][j] != 0)
				return 0;

		if (g.board[R][C] != 0 && colorOf(g.board[R][C]) == g.turn)
			return 0;

		return 1;
	} else if (isRook(g.board[r][c])) {
		if (r == R) {
			int i;
			int dc = sign(C-c);
			for (i=c+dc; i != C; i += dc)
				if (g.board[r][i] != 0)
					return 0;

			if (g.board[R][C] != 0 && colorOf(g.board[R][C] == g.turn))
				return 0;
			
			return 1;
		} else if (c == C) {
			int i;
			int dr = sign(R-r);
			for (i=r+dr; i != R; i += dr)
				if (g.board[i][C] != 0)
					return 0;

			if (g.board[R][C] != 0 && colorOf(g.board[R][C] == g.turn))
				return 0;
			
			return 1;

		} else
			return 0;
	} else if (isQueen(g.board[r][c])) {
		if (abs(R-r) == abs(C-c)) {
			int dr, dc;
			int i, j;
			if (abs(r-R) != abs(c-C))
				return 0;

			dr = sign(R-r);
			dc = sign(C-c);

			for (j=c+dc, i=r+dr; j != C && i != R; i+=dr, j+=dc)
				if (g.board[i][j] != 0)
					return 0;

			if (g.board[R][C] != 0 && colorOf(g.board[R][C]) == g.turn)
				return 0;

			return 1;
		} else if (r == R) {
			int i;
			int dc = sign(C-c);
			for (i=c+dc; i != C; i += dc)
				if (g.board[r][i] != 0)
					return 0;

			if (g.board[R][C] != 0 && colorOf(g.board[R][C] == g.turn))
				return 0;
			
			return 1;
		} else if (c == C) {
			int i;
			int dr = sign(R-r);
			for (i=r+dr; i != R; i += dr)
				if (g.board[i][C] != 0)
					return 0;

			if (g.board[R][C] != 0 && colorOf(g.board[R][C] == g.turn))
				return 0;
			
			return 1;

		} else
			return 0;
	} else if (isKing(g.board[r][c])) {
		if (abs(C-c) > 1) return 0;
		if (abs(R-r) > 1) return 0;
		if (g.board[R][C] != 0 && colorOf(g.board[R][C]) == g.turn)
			return 0;

		return 1;
	} else {
		fprintf(stderr, "!!!!!!!! (%i)\n", g.board[r][c]);
	}

	return 0;
}

#define copyWithMove(to, from, r, c, R, C) ((to)=(from), (to).board[R][C] = (from).board[r][c], (to).board[r][c] = 0, (to).turn = flipTurn((from).turn), 0)
#define addToRet(rr, cc, RR, CC) (copyWithMove(arr[alen], g, rr, cc, RR, CC), (!inCheck(arr[alen], g.turn))?(arr[alen].lastmove.r=rr, arr[alen].lastmove.R=RR, arr[alen].lastmove.c=cc,arr[alen].lastmove.C=CC, alen++, (alen == asz)?(asz *= 2,arr = realloc(arr, asz * sizeof g), 0):0, 0):0)

int genSuccs(game g, game **arr_ret) {
	int i, j;
	int alen, asz;
	game *arr;

	alen = 0;
	asz = 32;
	arr = malloc(asz * sizeof g);

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g.board[i][j] == 0 || colorOf(g.board[i][j]) != g.turn)
				continue; /* can't be moved */

			if (isPawn(g.board[i][j])) {
				if (g.turn == BLACK && i < 7) {
					if (g.board[i+1][j] == 0)
						addToRet(i, j, i+1, j);
					if (j < 7 && g.board[i+1][j+1] != 0&& colorOf(g.board[i+1][j+1]) != g.turn)
						addToRet(i, j, i+1, j+1);
					if (j > 0 && g.board[i+1][j-1] != 0&& colorOf(g.board[i+1][j-1]) != g.turn)
						addToRet(i, j, i+1, j-1);
				} else if (g.turn == WHITE && i > 0) {
					if (g.board[i-1][j] == 0)
						addToRet(i, j, i-1, j);
					if (j < 7 && g.board[i-1][j+1] != 0 && colorOf(g.board[i-1][j+1]) != g.turn)
						addToRet(i, j, i-1, j+1);
					if (j > 0 && g.board[i-1][j-1] != 0 && colorOf(g.board[i-1][j-1]) != g.turn)
						addToRet(i, j, i-1, j-1);
				}
			} else if (isKnight(g.board[i][j])) {
				int di, dj;
				for (di=1; di<3; di++) {
					dj = 3-di;

					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i, j, i+di, j+dj);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i, j, i+di, j+dj);

					di=-di;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i, j, i+di, j+dj);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i, j, i+di, j+dj);

					di=-di;
				}
			} else if (isRook(g.board[i][j])) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
			} else if (isBishop(g.board[i][j])) {
				int k, l;

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
			} else if (isQueen(g.board[i][j])) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g.board[k][l] == 0)
						addToRet(i, j, k, l);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(i, j, k, l);
						}
						break;
					}
				}
			} else if (isKing(g.board[i][j])) {
				if (i>0 && (g.board[i-1][j] == 0 || colorOf(g.board[i-1][j]) != g.turn))
					addToRet(i, j, i-1, j);
				if (i<7 && (g.board[i+1][j] == 0 || colorOf(g.board[i+1][j]) != g.turn))
					addToRet(i, j, i+1, j);
				if (j>0 && (g.board[i][j-1] == 0 || colorOf(g.board[i][j-1]) != g.turn))
					addToRet(i, j, i, j-1);                               
				if (j<7 && (g.board[i][j+1] == 0 || colorOf(g.board[i][j+1]) != g.turn))
					addToRet(i, j, i, j+1);
				if (i>0 && j>0 && (g.board[i-1][j-1] == 0 || colorOf(g.board[i-1][j-1]) != g.turn))
					addToRet(i, j, i-1, j-1);
				if (i<7 && j>0 && (g.board[i+1][j-1] == 0 || colorOf(g.board[i+1][j-1]) != g.turn))
					addToRet(i, j, i+1, j-1);
				if (i>0 && j<7 && (g.board[i-1][j+1] == 0 || colorOf(g.board[i-1][j+1]) != g.turn))
					addToRet(i, j, i-1, j+1);
				if (i<7 && j<7 && (g.board[i+1][j+1] == 0 || colorOf(g.board[i+1][j+1]) != g.turn))
					addToRet(i, j, i+1, j+1);
			}
		}
	}

	*arr_ret = arr;

//	if (alen == 0) {
//		printBoard(g);
//		printf("DEVUELVO 0 COMO UN CAGON\n");
//	}

	return alen;
}


