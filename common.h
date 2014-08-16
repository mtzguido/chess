#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_DEPTH 30

#define ARRSIZE(a) ((sizeof (a))/(sizeof ((a)[0])))
#define __maybe_unused  __attribute__((unused))

#define unlikely(c) __builtin_expect(c, 0)
#define   likely(c) __builtin_expect(c, 1)

#define clamp(v, l, h)			\
({					\
	typeof(v) __v = v;		\
	typeof(l) __l = l;		\
	typeof(h) __h = h;		\
	(void) (&__v == &__l);		\
	(void) (&__v == &__h);		\
	__v = __v < __l ? __l : __v;	\
	__v > __h ? __h : __v;		\
})

typedef uint64_t	u64;
typedef uint32_t	u32;
typedef uint16_t	u16;
typedef uint8_t		u8;
typedef int64_t		i64;
typedef int32_t		i32;
typedef int16_t		i16;
typedef int8_t		i8;

extern unsigned seed;

int isPrefix(char *a, char *b);
char pieceOf(char c);

enum play_mode {
	normal,
	self,
	moves,
	randplay,
	ai_vs_rand,
	board_eval,
};

struct opts {
	enum play_mode mode;
	int nmoves;			/* Amount of moves to calculate */
	int depth;			/* Maximum search depth */
	bool fixed_depth;		/* Fix depth, do not cut by time */
	unsigned long timelimit;	/* Time limit for each move, in ms */
	bool shuffle;			/* Shuffle the succ moves */
	bool usebook;			/* Use opening book */
	bool black;			/* Play as black */
	bool sort;			/* Use move sorting */

	bool custom_start;		/* Use a custom starting board */
	char custom_start_str[100];	/* Custom board spec */

	bool reverse;			/* Reverse the order of succ generation (debug) */

	/* Search */
	bool iter;			/* Iterative deepening */
	bool ab;			/* Alpha-beta */
	bool quiesce;			/* Use quiescence search on leaf nodes */
	bool nullmove;			/* Use Null Move Heuristic */
	bool lmr;			/* Late move reduction */
	bool forced_extend;		/* Forced move extension */
	bool delta_prune;		/* Quiescence delta pruning */

	/* Heuristics (mostly move ordering) */
	bool heur_trans;		/* Transposition table */
	bool heur_killer;		/* Killer Heuristic */
	bool heur_cm;			/* Countermove Heuristic */
	bool heur_trivial;		/* Trivial and cheap move ordering */

	/* Board evaluation */
	bool h11n;			/* 1-1-n heuristic */
};

static const struct opts defopts = {
	.mode = normal,
	.nmoves = 0,
	.depth = 6,
	.fixed_depth = false,
	.timelimit = 1000,
	.shuffle = true,
	.usebook = true,
	.black = false,
	.sort = true,

	.reverse = false,

	.iter = true,
	.ab = true,
	.quiesce = true,
	.nullmove = true,
	.lmr = true,
	.forced_extend = true,
	.delta_prune = true,

	.heur_trans = true,
	.heur_killer = true,
	.heur_cm = true,
	.heur_trivial = true,

	.h11n = true,
};

extern struct opts copts;

int on_bits(u64 x, u8 *rows, u8 *cols);

#endif
