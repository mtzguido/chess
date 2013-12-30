#include "board.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

game init = {
	.board= {
		{ WTOWER, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WTOWER },
		{ WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN },
		{ BTOWER, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BTOWER }
	},
	.turn = WHITE,
	.idlecount = 0,
	.wk_cancastle = 1,
	.wq_cancastle = 1,
	.bk_cancastle = 1,
	.bq_cancastle = 1,
	.en_passant_x = 0,
	.en_passant_y = 0
};

game startingStatus() {
	return init;
}

void printBoard(game g) {
	int i, j;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			putchar(charOf(g.board[i][j]));
		}
		putchar('\n');
	}
}

char charOf(int piece) {

	switch (piece) {
		case EMPTY:   return ' ';
		case WPAWN:   return 'P';
		case BPAWN:   return 'p';
		case WTOWER:  return 'T';
		case BTOWER:  return 't';
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

