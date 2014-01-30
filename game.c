#include "game.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>

int isLegalMove(game g, int row1, int col1, int row2, int col2) {
	return 1;
}

#define copyWithMove(to, from, r, c, R, C) ((to)=(from), (to).board[r][c] = (from).board[R][C], (to).board[R][C] = 0, (to).turn = flipTurn((from).turn), 0)
#define addToRet(rr, cc, RR, CC) (copyWithMove(arr[alen], g, rr, cc, RR, CC), arr[alen].lastmove.r=rr, arr[alen].lastmove.R=RR, arr[alen].lastmove.c=cc,arr[alen].lastmove.C=CC, alen++, 0)

int genSuccs(game g, game **arr_ret) {
	int i, j;
	int alen, asz;
	game *arr;

	arr = malloc(32000 * sizeof g);
	alen = 0;
	asz = 32;
	asz=asz; /* !!!!!!!!!!!!!!!!! */

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g.board[i][j] == 0 || colorOf(g.board[i][j]) != g.turn)
				continue; /* can't be moved */

			if (isPawn(g.board[i][j])) {
				if (g.turn == BLACK && i < 7) {
					if (g.board[i+1][j] == 0)
						addToRet(i+1, j, i, j);
					else if (j < 7 && colorOf(g.board[i+1][j+1]) != g.turn)
						addToRet(i+1, j+1, i, j);
					else if (j > 0 && colorOf(g.board[i+1][j-1]) != g.turn)
						addToRet(i+1, j-1, i, j);
				
				} else if (g.turn == WHITE && i > 0) {
					if (g.board[i-1][j] == 0)
						addToRet(i-1, j, i, j);
					else if (j < 7 && colorOf(g.board[i-1][j+1]) != g.turn)
						addToRet(i-1, j+1, i, j);
					else if (j > 0 && colorOf(g.board[i-1][j-1]) != g.turn)
						addToRet(i-1, j-1, i, j);
				
				}
			} else if (isKnight(g.board[i][j])) {
				int di, dj;
				for (di=1; di<3; di++) {
					dj = 3-di;

					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i+di, j+dj, i, j);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i+di, j+dj, i, j);

					di=-di;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i+di, j+dj, i, j);

					dj=-dj;
					if (i+di >= 0 && j+dj>=0 && i+di < 8 && j+dj < 8 && (g.board[i+di][j+dj] == 0 || colorOf(g.board[i+di][j+dj]) != g.turn))
						addToRet(i+di, j+dj, i, j);

					di=-di;
				}
			} else if (isRook(g.board[i][j])) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
			} else if (isBishop(g.board[i][j])) {
				int k, l;

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
			} else if (isQueen(g.board[i][j])) {
				int k, l;

				k = i;
				for (l=j+1; l<8; l++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				k = i;
				for (l=j-1; l>=0; l--) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				l = j;
				for (k=i+1; k<8; k++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				l = j;
				for (k=i-1; k>=0; k++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}

				for (k=i+1, l=j+1; k<8 && l<8; k++, l++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				for (k=i-1, l=j+1; k>=0 && l<8; k--, l++) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				for (k=i+1, l=j-1; k<8 && l>=0; k++, l--) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
				for (k=i-1, l=j-1; k>=0 && l>=0; k--, l--) {
					if (g.board[k][l] == 0)
						addToRet(k, l, i, j);
					else {
						if (colorOf(g.board[k][l]) != g.turn) {
							addToRet(k, l, i, j);
						}
						break;
					}
				}
			} else if (isKing(g.board[i][j])) {
				if (i>0 && (g.board[i-1][j] == 0 || colorOf(g.board[i-1][j]) != g.turn))
					addToRet(i-1, j, i, j);
				if (i<8 && (g.board[i+1][j] == 0 || colorOf(g.board[i+1][j]) != g.turn))
					addToRet(i+1, j, i, j);
				if (j>0 && (g.board[i][j-1] == 0 || colorOf(g.board[i][j-1]) != g.turn))
					addToRet(i, j-1, i, j);                               
				if (j<8 && (g.board[i][j+1] == 0 || colorOf(g.board[i][j+1]) != g.turn))
					addToRet(i, j+1, i, j);
				if (i>0 && j>0 && (g.board[i-1][j-1] == 0 || colorOf(g.board[i-1][j-1]) != g.turn))
					addToRet(i-1, j-1, i, j);
				if (i<8 && j>0 && (g.board[i+1][j-1] == 0 || colorOf(g.board[i-1][j-1]) != g.turn))
					addToRet(i+1, j-1, i, j);
				if (i>0 && j<8 && (g.board[i-1][j+1] == 0 || colorOf(g.board[i-1][j+1]) != g.turn))
					addToRet(i-1, j+1, i, j);
				if (i<8 && j<8 && (g.board[i+1][j+1] == 0 || colorOf(g.board[i-1][j+1]) != g.turn))
					addToRet(i+1, j+1, i, j);

			}
		}
	}

	*arr_ret = arr;
	return alen;
}


