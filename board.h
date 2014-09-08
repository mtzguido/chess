#ifndef __BOARD_H
#define __BOARD_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"

enum move_types {
	MOVE_INVAL,
	MOVE_REGULAR,
	MOVE_KINGSIDE_CASTLE,
	MOVE_QUEENSIDE_CASTLE,
	MOVE_NULL,
};

struct move {
	/* (r,c) -> (R,C) */
	u8 r:4;
	u8 c:4;
	u8 R:4;
	u8 C:4;
	u8 promote:4;

	u8 who:1;
	enum move_types move_type:3;
};

typedef struct move move;

typedef u8 piece_t;

struct game_struct {
	piece_t board[8][8];
	u64 piecemask[2];
	uint64_t zobrist;
	int pps_O;
	int pps_E;
	int pieceScore[2];
	int pawn_rank[2][10];

	move lastmove;
	u8 idlecount;
	bool castle_king[2];
	bool castle_queen[2];
	i8 en_passant_x:4;
	i8 en_passant_y:4;
	bool castled[2];
	u8 kingx[2];
	u8 kingy[2];
	i8 inCheck[2];
	u8 turn:1;

	bool was_capture;
	bool was_promote;
};

typedef struct game_struct *game;

/* Global game */
extern game G;

game prevGame(void);

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
#define WBISHOP		3
#define WROOK		4
#define WQUEEN		5
#define WKING		6
#define BPAWN		(8 | WPAWN)
#define BKNIGHT		(8 | WKNIGHT)
#define BBISHOP		(8 | WBISHOP)
#define BROOK		(8 | WROOK)
#define BQUEEN		(8 | WQUEEN)
#define BKING		(8 | WKING)

#define isEmpty(c)	((c) == EMPTY)
#define isPawn(c)	((c&7) == WPAWN)
#define isKnight(c)	((c&7) == WKNIGHT)
#define isBishop(c)	((c&7) == WBISHOP)
#define isRook(c)	((c&7) == WROOK)
#define isQueen(c)	((c&7) == WQUEEN)
#define isKing(c)	((c&7) == WKING)
#define colorOf(c)	(!((c)&8))

void startingGame(void);

bool doMove(move m); /* Actua sobre g */
bool doMove_unchecked(move m); /* Actua sobre g */
void undoMove(void);

bool inCheck(int who);
int isFinished(void);
bool isDraw(void);

void printBoard(void);

void tostr(char *s);
void fromstr(const char *s);

bool equalGame(game a, game b);

char charOf(int piece);

static inline bool own_piece(i8 r, i8 c) {
	return G->piecemask[G->turn] & ((u64)1 << (r*8 + c));
}

static inline bool enemy_piece(i8 r, i8 c) {
	return G->piecemask[flipTurn(G->turn)] & ((u64)1 << (r*8 + c));
}

static inline bool any_piece(i8 r, i8 c) {
	return (G->piecemask[BLACK] | G->piecemask[WHITE])
			& ((u64)1 << (r*8 + c));
}

static inline bool isCapture(move m) {
	bool ret;
	if (m.move_type != MOVE_REGULAR)
		return false;

	ret = enemy_piece(m.R, m.C)
		|| (m.R == G->en_passant_x
			&& m.C == G->en_passant_y
			&& isPawn(G->board[m.r][m.c]));
	return ret;
}

static inline bool isPromotion(move m) {
	if (m.move_type != MOVE_REGULAR)
		return false;

	return m.promote != EMPTY;
}

static inline bool equalMove(move a, move b) {
	if (a.who != b.who || a.move_type != b.move_type)
		return false;

	if (a.move_type != MOVE_REGULAR)
		return a.move_type == b.move_type;

	return a.r == b.r
	    && a.R == b.R
	    && a.c == b.c
	    && a.C == b.C
	    && a.promote == b.promote;
}

/* Piece scores */
#define QUEEN_SCORE		900
#define ROOK_SCORE		500
#define BISHOP_SCORE		330
#define KNIGHT_SCORE		320
#define PAWN_SCORE		100

#define SIDE_SCORE		(QUEEN_SCORE + 2 * ROOK_SCORE + \
				 2 * BISHOP_SCORE + 2 * KNIGHT_SCORE + \
				 8 * PAWN_SCORE)

static inline int interpolate(game g, int start, int finish) {
	int t = g->pieceScore[WHITE] + g->pieceScore[BLACK];
	return (t*(start - finish)/(2*SIDE_SCORE)) + finish;
}

typedef int score;
static const score minScore = -1e7;
static const score maxScore =  1e7;

score scoreOf(int piece);

struct MS {
	move m;
	score s;
};

#endif
