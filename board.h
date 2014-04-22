#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"

#define MOVE_REGULAR 0
#define MOVE_KINGSIDE_CASTLE 1
#define MOVE_QUEENSIDE_CASTLE 2

struct move {
	u8 who;
	i8 move_type;
	/* (r,c) -> (R,C) */
	i8 r;
	i8 c;
	i8 R;
	i8 C;
	i8 promote;
} __attribute__((packed));

typedef struct move move;

struct game_struct {
	/* Tablero */
	/* board [1][2] == C2 */
	i8 board[8][8];
	u8 turn;

	/* Última jugada */
	move lastmove;

	/* Estado no visible */
	u8 idlecount;
	bool castle_king[2];
	bool castle_queen[2];
	i8 en_passant_x;
	i8 en_passant_y;

	/* Si se hizo enroque */
	bool castled[2];

	/* Optimizaciones */
	/*   Zobrist hash */
	uint64_t zobrist;

	/*   Posicion de los reyes */
	u8 kingx[2];
	u8 kingy[2];

	/*   Caches de jaque */
	i8 inCheck[2];
	/*     inCheck[who] = -1 -> no conocido
	 *     inCheck[who] = 0 -> libre
	 *     inCheck[who] = 1 -> en jaque */

	/*   Cache de score */
	int pieceScore;
	int totalScore;
	/*   Piece scores de opening y
	 *   endgame, se interpolan luego */
	int pps_O;
	int pps_E;
} __attribute__((packed));

typedef struct game_struct *game;

/* Players */
#define BLACK	0
#define WHITE	1
#define flipTurn(t) (1^(t))

/* Results */
#define DRAW_STALE 10
#define DRAW_3FOLD 11
#define DRAW_50MOVE 12
#define WIN(p) (2+(p))

/* Pieces */
#define EMPTY		0
#define WPAWN		1
#define WKNIGHT		2
#define	WBISHOP		3
#define WROOK		4
#define WQUEEN		5
#define	WKING		6
#define BPAWN		(-1)
#define BKNIGHT		(-2)
#define	BBISHOP		(-3)
#define BROOK		(-4)
#define BQUEEN		(-5)
#define	BKING		(-6)

#define isEmpty(c)	((c) == 0)
#define isPawn(c)	(abs(c) == 1)
#define isKnight(c)	(abs(c) == 2)
#define isBishop(c)	(abs(c) == 3)
#define isRook(c)	(abs(c) == 4)
#define isQueen(c)	(abs(c) == 5)
#define isKing(c)	(abs(c) == 6)
#define colorOf(c)	((c)>0)

game startingGame(void);

bool doMove(game g, move m); /* Actua sobre g */
game copyGame(game g);
void freeGame(game g);

bool inCheck(game g, int who);
int isFinished(game g);
bool isDraw(game g);

void printBoard(game b);

bool equalGame(game a, game b);

char charOf(int piece);

static inline bool enemy_of(i8 piece, u8 who) {
	return piece != EMPTY && colorOf(piece) != who;
}

__maybe_unused static bool equalMove(move a, move b) {
	if (a.who != b.who)
		return false;

	if (a.move_type != MOVE_REGULAR)
		return a.move_type == b.move_type;

	return memcmp(&a, &b, sizeof (move)) == 0;
}

#endif

