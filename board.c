#include "board.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

game init = {
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
		case WROOK:  return 'R';
		case BROOK:  return 'r';
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

