#ifndef __MOVE_H__
#define __MOVE_H__

#include "board.h"
#include <assert.h>
#include <stdio.h>

int pawnMove(game g, int r, int c, int R, int C);
int rookMove(game g, int r, int c, int R, int C);
int kingMove(game g, int r, int c, int R, int C);
int knightMove(game g, int r, int c, int R, int C);
int bishopMove(game g, int r, int c, int R, int C);

static inline int canMove(game g, int r, int c, int R, int C) {
	const int piece = g->board[r][c];
	switch(piece) {
	case 0:
		return 0;
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
	  assert(0);
	}

	return -1;
}


#endif

