#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define MAX_DEPTH 30

#define ARRSIZE(a) ((sizeof (a))/(sizeof ((a)[0])))
#define __maybe_unused  __attribute__((unused))

typedef uint64_t	u64;
typedef uint32_t	u32;
typedef uint16_t	u16;
typedef uint8_t		u8;
typedef int64_t		i64;
typedef int32_t		i32;
typedef int16_t		i16;
typedef int8_t		i8;

int isPrefix(char *a, char *b);
char pieceOf(char c);

#ifdef CFG_DEPTH_EXTENSION
 #define EXTRA_CHECK	1
 #define EXTRA_CAPTURE	5
 #define EXTRA_PROMOTION	99
#else
 #define EXTRA_CHECK	0
 #define EXTRA_CAPTURE	0
 #define EXTRA_PROMOTION	0
#endif

enum play_mode {
	normal,
	self,
	moves,
	randplay,
	ai_vs_rand,
};

struct opts {
	enum play_mode mode;
	int nmoves;
	int depth;
	bool shuffle;
	bool black;

	/* Heuristics */
	bool heur_trans;
	bool heur_killer;
	bool heur_cm;
	bool heur_trivial;
};

static const struct opts defopts = {
	.mode = normal,
	.nmoves = 0,
	.depth = 6,
	.shuffle = false,
	.black = false,

	.heur_trans = true,
	.heur_killer = true,
	.heur_cm = true,
	.heur_trivial = true,
};

extern struct opts copts;

static inline int fls(u64 x)
{
	int r = 64;

	if (!x)
		return 0;

	if (!(x & 0xffffffff00000000u)) {
		x <<= 32;
		r -= 32;
	}
	if (!(x & 0xffff000000000000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff00000000000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf000000000000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc000000000000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x8000000000000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

#endif
