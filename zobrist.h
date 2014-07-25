#ifndef __ZOBRIST_H
#define __ZOBRIST_H

#include "common.h"
#include "board.h"

#include <assert.h>

extern const u64 zobrist_keys[794];

static inline u64 ZOBR_PIECE(piece_t piece, int r, int c) {
	assert(piece != 0);

	const int p = piece > 6 ? piece - 2 : piece;
	return zobrist_keys[r*8*12 + c*12 + p];
}

static inline u64 ZOBR_BLACK() {
	return zobrist_keys[781];
}

static inline u64 ZOBR_CASTLE_K(int who) {
	return zobrist_keys[782 + who];
}

static inline u64 ZOBR_CASTLE_Q(int who) {
	return zobrist_keys[784 + who];
}

static inline u64 ZOBR_EP(int c) {
	if (c == -1)
		return 0;

	return zobrist_keys[786 + c];
}

#endif
