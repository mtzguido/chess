#ifndef __MOVE_H
#define __MOVE_H

#include "board.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool pawnMove(int r, int c, int R, int C);
bool rookMove(int r, int c, int R, int C);
bool kingMove(int r, int c, int R, int C);
bool knightMove(int r, int c, int R, int C);
bool bishopMove(int r, int c, int R, int C);

static inline bool canMove(int r, int c, int R, int C) {
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

#endif

