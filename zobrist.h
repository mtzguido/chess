#ifndef __ZOBRIST_H
#define __ZOBRIST_H

#include "common.h"
#include "board.h"

#include <assert.h>

extern const u64 zobrist_keys[794];

static inline u64 ZOBR_PIECE(piece_t piece, int r, int c) {
	const int p = piece > 6 ? piece - 2 : piece;
	const unsigned idx = r * 8 * 12 + c * 12 + p;
	assert(piece != 0);

	assert(idx < ARRSIZE(zobrist_keys));

	return zobrist_keys[idx];
}

static inline u64 ZOBR_BLACK() {
	const unsigned idx = 781;

	assert(idx < ARRSIZE(zobrist_keys));

	return zobrist_keys[idx];
}

static inline u64 ZOBR_CASTLE_K(int who) {
	const unsigned idx = 782 + who;

	assert(idx < ARRSIZE(zobrist_keys));

	return zobrist_keys[idx];
}

static inline u64 ZOBR_CASTLE_Q(int who) {
	const unsigned idx = 784 + who;

	assert(idx < ARRSIZE(zobrist_keys));

	return zobrist_keys[idx];
}

static inline u64 ZOBR_EP(int c) {
	const unsigned idx = 786 + c;

	if (c == -1)
		return 0;

	assert(idx < ARRSIZE(zobrist_keys));

	return zobrist_keys[idx];
}

#endif
