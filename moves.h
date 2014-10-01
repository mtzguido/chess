#ifndef __MOVES_H
#define __MOVES_H

#include "board.h"
#include "common.h"

bool doMove(const move * const m);
bool doMove_unchecked(const move * const m);
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
	return ((u64)1 << (r * 8 + c));
}

static inline int bitrow(int i) {
	return i >> 3;
}

static inline int bitcol(int i) {
	return i & 7;
}

extern struct undo_info * const hstack;
#endif
