#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

#include <stdint.h>

extern const uint64_t zobrist_keys[];

static inline uint64_t ZOBR_PIECE(int piece, int r, int c) {
	return zobrist_keys[r*8*13 + c*13 + piece + 6];
}

static inline uint64_t ZOBR_BLACK() {
	return zobrist_keys[832];
}

static inline uint64_t ZOBR_CASTLE_K(int who) {
	return zobrist_keys[833 + who];
}

static inline uint64_t ZOBR_CASTLE_Q(int who) {
	return zobrist_keys[835 + who];
}

static inline uint64_t ZOBR_EP(int c) {
	if (c == -1)
		return 0;

	return zobrist_keys[837 + c];
}

#endif
