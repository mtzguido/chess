#include <stdio.h>
#include <assert.h>
#include "board.h"

static int sign(int a) {
	if (a > 0) return 1;
	if (a < 0) return -1;
	return 0;
}

static bool pawnMove(int r, int c, int R, int C);
static bool rookMove(int r, int c, int R, int C);
static bool kingMove(int r, int c, int R, int C);
static bool knightMove(int r, int c, int R, int C);
static bool bishopMove(int r, int c, int R, int C);

static bool pawnMove(int r, int c, int R, int C) {
	if (colorOf(G->board[r][c]) == WHITE) {
		if (c == C) {
			if (R == r-1 && G->board[R][C] == 0)
				return 1;
			else if (R == r-2 && r == 6 && G->board[r-1][c] == 0 && G->board[r-2][c] == 0)
				return 1;
			else
				return 0;
		} else if ((R == r-1) && (c == C+1 || c == C-1)) {
			if (G->board[R][C] != 0 && colorOf(G->board[R][C]) != colorOf(G->board[r][c]))
				return 1;
			else if (R == G->en_passant_x && C == G->en_passant_y)
				return 1;
			else
				return 0;
		}
	} else { /* colorOf(G->board[r][c]) == BLACK */
		if (c == C) {
			if (R == r+1 && G->board[R][C] == 0)
				return 1;
			else if (R == r+2 && r == 1 && G->board[r+1][c] == 0 && G->board[r+2][c] == 0)
				return 1;
			else
				return 0;
		} else if ((R == r+1) && (c == C+1 || c == C-1)) {
			if (G->board[R][C] != 0 && colorOf(G->board[R][C]) != colorOf(G->board[r][c]))
				return 1;
			else if (R == G->en_passant_x && C == G->en_passant_y)
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

static bool knightMove(int r, int c, int R, int C) {
	if (abs(r-R) + abs(c-C) != 3 || abs(r-R) == 0 || abs(c-C) == 0)
		return 0;

	if (G->board[R][C] == 0 || colorOf(G->board[R][C]) != colorOf(G->board[r][c]))
		return 1;
	else
		return 0;
}

static bool bishopMove(int r, int c, int R, int C) {
	int dr, dc;
	int i, j;
	if (abs(r-R) != abs(c-C))
		return 0;

	dr = sign(R-r);
	dc = sign(C-c);

	for (j=c+dc, i=r+dr; j != C && i != R; i+=dr, j+=dc)
		if (G->board[i][j] != 0)
			return 0;

	if (G->board[R][C] != 0 && colorOf(G->board[R][C]) == colorOf(G->board[r][c]))
		return 0;

	return 1;
}

static bool rookMove(int r, int c, int R, int C) {
	if (r == R) {
		int i;
		int dc = sign(C-c);
		for (i=c+dc; i != C; i += dc)
			if (G->board[r][i] != 0)
				return 0;

		if (G->board[R][C] != 0 && colorOf(G->board[R][C]) == colorOf(G->board[r][c]))
			return 0;

		return 1;
	} else if (c == C) {
		int i;
		int dr = sign(R-r);
		for (i=r+dr; i != R; i += dr)
			if (G->board[i][C] != 0)
				return 0;

		if (G->board[R][C] != 0 && colorOf(G->board[R][C]) == colorOf(G->board[r][c]))
			return 0;

		return 1;
	} else
		return 0;
}

static bool kingMove(int r, int c, int R, int C) {
	if (abs(C-c) > 1) return 0;
	if (abs(R-r) > 1) return 0;
	if (G->board[R][C] != 0 && colorOf(G->board[R][C]) == colorOf(G->board[r][c]))
		return 0;

	return 1;
}

bool canMove(int r, int c, int R, int C) {
	const int piece = G->board[r][c];
	bool ret;

	switch(piece) {
	case EMPTY:
		ret = false;
		break;
	case WPAWN:
	case BPAWN:
		ret = pawnMove(r, c, R, C);
		break;
	case WKNIGHT:
	case BKNIGHT:
		ret = knightMove(r, c, R, C);
		break;
	case WBISHOP:
	case BBISHOP:
		ret = bishopMove(r, c, R, C);
		break;
	case WROOK:
	case BROOK:
		ret = rookMove(r, c, R, C);
		break;
	case WQUEEN:
	case BQUEEN:
		ret = rookMove(r, c, R, C)
		   || bishopMove(r, c, R, C);
		break;
	case WKING:
	case BKING:
		ret = kingMove(r, c, R, C);
		break;
	default:
		fprintf(stderr, "!!!!!!!! (%i)\n", G->board[r][c]);
		abort();
	}

	return ret;
}
