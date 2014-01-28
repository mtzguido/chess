#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdlib.h>

typedef struct {
	signed char board[8][8];
	unsigned char turn;
	unsigned char idlecount;
	unsigned char wk_cancastle;
	unsigned char wq_cancastle;
	unsigned char bk_cancastle;
	unsigned char bq_cancastle;
	unsigned char en_passant_x;
	unsigned char en_passant_y;
} game;

game startingStatus(void);
void printBoard(game b);

/* players */
#define BLACK	0
#define WHITE	1

extern int machineColor;

/* pieces */
#define EMPTY	0
#define WPAWN	1
#define WROOK	2
#define WKNIGHT	3
#define	WBISHOP	4
#define WQUEEN	5
#define	WKING	6
#define BPAWN	(-1)
#define BROOK	(-2)
#define BKNIGHT	(-3)
#define	BBISHOP	(-4)
#define BQUEEN	(-5)
#define	BKING	(-6)

#define isEmpty(c)	((c)==0)
#define isPawn(c)	(abs(c) == 1)
#define isTower(c)	(abs(c) == 2)
#define isKnight(c)	(abs(c) == 3)
#define isBishop(c)	(abs(c) == 4)
#define isQueen(c)	(abs(c) == 5)
#define isKing(c)	(abs(c) == 6)

#define colorOf(c)	((c)>0)

char charOf(int piece);

#endif

