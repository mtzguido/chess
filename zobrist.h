#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include "common.h"

extern const u64 zobrist_keys[];

static inline u64 ZOBR_PIECE(int piece, int r, int c) {
	return zobrist_keys[r*8*13 + c*13 + piece + 6];
}

static inline u64 ZOBR_BLACK() {
	return zobrist_keys[832];
}

static inline u64 ZOBR_CASTLE_K(int who) {
	return zobrist_keys[833 + who];
}

static inline u64 ZOBR_CASTLE_Q(int who) {
	return zobrist_keys[835 + who];
}

static inline u64 ZOBR_EP(int c) {
	if (c == -1)
		return 0;

	return zobrist_keys[837 + c];
}

#endif
