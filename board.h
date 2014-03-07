#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdlib.h>
#include <stdint.h>

#define MOVE_REGULAR 0
#define MOVE_KINGSIDE_CASTLE 1
#define MOVE_QUEENSIDE_CASTLE 2

typedef struct move {
	int who;
	int move_type;
	int r, c, R, C; /* (r,c) -> (R,C) */
	int promote;
/*	int capture:1;
	int epcapture:1;
	*/
} move;

struct game_struct {
	/* Tablero */
	/* board [1][2] == C2 */
	int8_t board[8][8];
	unsigned char turn;

	/* Última jugada */
	move lastmove;

	/* Estado no visible */
	uint8_t idlecount;
	uint8_t castle_king[2];
	uint8_t castle_queen[2];
	uint8_t en_passant_x;
	uint8_t en_passant_y;

	/* Optimizaciones */
	/*   Posicion de los reyes */
	uint8_t kingx[2];
	uint8_t kingy[2];

	/*   Caches de jaque */
	int8_t inCheck[2];
	/*     inCheck[who] = -1 -> no conocido
	 *     inCheck[who] = 0 -> libre
	 *     inCheck[who] = 1 -> en jaque */
};

typedef struct game_struct *game;

/* Players */
#define BLACK	0
#define WHITE	1
#define flipTurn(t) ((t)==BLACK?WHITE:BLACK)

/* Results */
#define DRAW 1
#define WIN(p) (2+(p))

/* Pieces */
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
#define isRook(c)	(abs(c) == 2)
#define isKnight(c)	(abs(c) == 3)
#define isBishop(c)	(abs(c) == 4)
#define isQueen(c)	(abs(c) == 5)
#define isKing(c)	(abs(c) == 6)
#define colorOf(c)	((c)>0)

game startingGame(void);

int isLegalMove(game g, move m);
void doMove(game g, move m); /* Actua sobre g */
game copyGame(game g);
void freeGame(game g);

#include "succs.h"

int inCheck(game g, int who);

void freeSuccs(game *arr, int len);
int isFinished(game g);

void printBoard(game b);

#endif

