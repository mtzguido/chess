#ifndef __MOVE_H
#define __MOVE_H

#include "board.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool pawnMove(game g, int r, int c, int R, int C);
bool rookMove(game g, int r, int c, int R, int C);
bool kingMove(game g, int r, int c, int R, int C);
bool knightMove(game g, int r, int c, int R, int C);
bool bishopMove(game g, int r, int c, int R, int C);

static inline bool canMove(game g, int r, int c, int R, int C) {
	const int piece = g->board[r][c];
	switch(piece) {
	case EMPTY:
		return false;
	case WPAWN:
	case BPAWN:
		return pawnMove(g, r, c, R, C);
	case WKNIGHT:
	case BKNIGHT:
		return knightMove(g, r, c, R, C);
	case WBISHOP:
	case BBISHOP:
		return bishopMove(g, r, c, R, C);
	case WROOK:
	case BROOK:
		return rookMove(g, r, c, R, C);
	case WQUEEN:
	case BQUEEN:
		return rookMove(g, r, c, R, C)
		    || bishopMove(g, r, c, R, C);
	case WKING:
	case BKING:
		return kingMove(g, r, c, R, C);
	default:
		fprintf(stderr, "!!!!!!!! (%i)\n", g->board[r][c]);
		abort();
	}
}

#endif

