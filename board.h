#ifndef __BOARD_H__
#define __BOARD_H__

typedef struct {
	unsigned char board[8][8];
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
#define WHITE	0
#define BLACK	1

/* pieces */
#define EMPTY	0
#define WPAWN	2
#define BPAWN	3
#define WTOWER	4
#define BTOWER	5
#define WKNIGHT	6
#define BKNIGHT	7
#define	WBISHOP	8
#define	BBISHOP	9
#define WQUEEN	10
#define BQUEEN	11
#define	WKING	12
#define	BKING	13

#define isEmpty(c)	((c)==0)
#define isPawn(c)	((c)/2 == 1)
#define isTower(c)	((c)/2 == 2)
#define isKnight(c)	((c)/2 == 3)
#define isBishop(c)	((c)/2 == 4)
#define isQueen(c)	((c)/2 == 5)
#define isKing(c)	((c)/2 == 6)

#define colorOf(c)	((c)&1)

char charOf(int piece);

#endif

