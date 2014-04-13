#include <stdio.h>
#include <assert.h>
#include "board.h"

static int sign(int a) {
	if (a > 0) return 1;
	if (a < 0) return -1;
	return 0;
}

bool pawnMove(game g, int r, int c, int R, int C);
bool rookMove(game g, int r, int c, int R, int C);
bool kingMove(game g, int r, int c, int R, int C);
bool knightMove(game g, int r, int c, int R, int C);
bool bishopMove(game g, int r, int c, int R, int C);

bool pawnMove(game g, int r, int c, int R, int C) {
	if (colorOf(g->board[r][c]) == WHITE) {
		if (c == C) {
			if (R == r-1 && g->board[R][C] == 0)
				return 1;
			else if (R == r-2 && r == 6 && g->board[r-1][c] == 0 && g->board[r-2][c] == 0)
				return 1;
			else
				return 0;
		} else if ((R == r-1) && (c == C+1 || c == C-1)) {
			if (g->board[R][C] != 0 && colorOf(g->board[R][C]) != colorOf(g->board[r][c]))
				return 1;
			else if (R == g->en_passant_x && C == g->en_passant_y)
				return 1;
			else
				return 0;
		}
	} else { /* colorOf(g->board[r][c]) == BLACK */
		if (c == C) {
			if (R == r+1 && g->board[R][C] == 0)
				return 1;
			else if (R == r+2 && r == 1 && g->board[r+1][c] == 0 && g->board[r+2][c] == 0)
				return 1;
			else
				return 0;
		} else if ((R == r+1) && (c == C+1 || c == C-1)) {
			if (g->board[R][C] != 0 && colorOf(g->board[R][C]) != colorOf(g->board[r][c]))
				return 1;
			else if (R == g->en_passant_x && C == g->en_passant_y)
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

bool knightMove(game g, int r, int c, int R, int C) {
	if (abs(r-R) + abs(c-C) != 3 || abs(r-R) == 0 || abs(c-C) == 0)
		return 0;

	if (g->board[R][C] == 0 || colorOf(g->board[R][C]) != colorOf(g->board[r][c]))
		return 1;
	else
		return 0;
}

bool bishopMove(game g, int r, int c, int R, int C) {
	int dr, dc;
	int i, j;
	if (abs(r-R) != abs(c-C))
		return 0;

	dr = sign(R-r);
	dc = sign(C-c);

	for (j=c+dc, i=r+dr; j != C && i != R; i+=dr, j+=dc)
		if (g->board[i][j] != 0)
			return 0;

	if (g->board[R][C] != 0 && colorOf(g->board[R][C]) == colorOf(g->board[r][c]))
		return 0;

	return 1;
}

bool rookMove(game g, int r, int c, int R, int C) {
	if (r == R) {
		int i;
		int dc = sign(C-c);
		for (i=c+dc; i != C; i += dc)
			if (g->board[r][i] != 0)
				return 0;

		if (g->board[R][C] != 0 && colorOf(g->board[R][C] == colorOf(g->board[r][c])))
			return 0;

		return 1;
	} else if (c == C) {
		int i;
		int dr = sign(R-r);
		for (i=r+dr; i != R; i += dr)
			if (g->board[i][C] != 0)
				return 0;

		if (g->board[R][C] != 0 && colorOf(g->board[R][C] == colorOf(g->board[r][c])))
			return 0;

		return 1;
	} else
		return 0;
}

bool kingMove(game g, int r, int c, int R, int C) {
	if (abs(C-c) > 1) return 0;
	if (abs(R-r) > 1) return 0;
	if (g->board[R][C] != 0 && colorOf(g->board[R][C]) == colorOf(g->board[r][c]))
		return 0;

	return 1;
}

