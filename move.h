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

static inline bool canMove(game g, int r, int c, int R, int C) {
	const int piece = g->board[r][c];
	G = g;
	switch(piece) {
	case EMPTY:
		return false;
	case WPAWN:
	case BPAWN:
		return pawnMove(r, c, R, C);
	case WKNIGHT:
	case BKNIGHT:
		return knightMove(r, c, R, C);
	case WBISHOP:
	case BBISHOP:
		return bishopMove(r, c, R, C);
	case WROOK:
	case BROOK:
		return rookMove(r, c, R, C);
	case WQUEEN:
	case BQUEEN:
		return rookMove(r, c, R, C)
		    || bishopMove(r, c, R, C);
	case WKING:
	case BKING:
		return kingMove(r, c, R, C);
	default:
		fprintf(stderr, "!!!!!!!! (%i)\n", G->board[r][c]);
		abort();
	}
}

#endif

