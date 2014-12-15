#ifndef __MOVES_H
#define __MOVES_H

#include "board.h"
#include "common.h"

bool doMove(move m);
bool doMove_unchecked(move m);
void undoMove(void);

#define MAX_HPLY 2000
extern int hply;

struct undo_info {
	move m;
	piece_t capt;
	bool was_promote;
	bool was_ep;

	u8 idlecount;
	bool castle_king[2];
	bool castle_queen[2];
	i8 en_passant_x:4;
	i8 en_passant_y:4;
	bool castled[2];
	i8 inCheck[2];
};

static inline u64 posbit(int r, int c) {
#ifdef FLIPBIT
	return ((u64)1 << (63 - r * 8 - c));
#else
	return ((u64)1 << (r * 8 + c));
#endif
}

static inline int bitrow(int i) {
#ifdef FLIPBIT
	return (63 - i) >> 3;
#else
	return i >> 3;
#endif
}

static inline int bitcol(int i) {
#ifdef FLIPBIT
	return (63 - i) & 7;
#else
	return i & 7;
#endif
}

extern struct undo_info * const hstack;
#endif
