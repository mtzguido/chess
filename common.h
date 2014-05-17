#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define MAX_DEPTH 30

#define ARRSIZE(a) ((sizeof (a))/(sizeof ((a)[0])))
#define __maybe_unused  __attribute__((unused))

typedef	uint64_t	u64;
typedef	uint32_t	u32;
typedef	uint16_t	u16;
typedef	uint8_t		u8;
typedef	int64_t		i64;
typedef	int32_t		i32;
typedef	int16_t		i16;
typedef	int8_t		i8;

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
	none,
	self,
	fairy,
	moves,
	randplay,
};

struct opts {
	enum play_mode mode;
	int nmoves;
	int depth;
	bool shuffle;
	bool alphabeta;

	/* Heuristics */
	bool heur_trans;
	bool heur_killer;
	bool heur_cm;
	bool heur_trivial;
};

static const struct opts defopts = {
	.mode = none,
	.nmoves = 0,
	.depth = 6,
	.shuffle = true,
	.alphabeta = true,

	.heur_trans = true,
	.heur_killer = true,
	.heur_cm = true,
	.heur_trivial = true,
};

extern struct opts copts;

#endif
