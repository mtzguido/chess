#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_DEPTH 30

#define ARRSIZE(a) ((sizeof (a))/(sizeof ((a)[0])))
#define __unused  __attribute__((unused))

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

char pieceOf(char c);

enum play_mode {
	xboard,
	self,
	moves,
	randplay,
	ai_vs_rand,
	board_eval,
	version,
	help,
};

struct opts {
	int mode;			/* Program mode */

	int nmoves;			/* Amount of moves to calculate */
	int log;			/* Write gamelog to file*/
	int depth;			/* Maximum search depth */
	int timed;			/* Do not stop by time */
	unsigned long timelimit;	/* Max time for each move, in ms */
	unsigned long lbound;		/* Min time for each move, in ms */
	int smart_stop;			/* Smart stopping */
	int shuffle;			/* Shuffle the succ moves */
	int book;			/* Use opening book */
	int sort;			/* Use move sorting */

	bool custom_start;		/* Use a custom starting board */
	char custom_start_str[100];	/* Custom board spec */

	/* Misc options */
	int verbosity;			/* Verbosity level */
	int seed;			/* Random seed */

	/* Search */
	int asp;			/* Use aspiration windows */
	int ab;				/* Alpha-beta */
	int quiesce;			/* Use quiescence search on leaf nodes */
	int null;			/* Use Null Move Heuristic */
	int lmr;			/* Late move reduction */
	int forced_extend;		/* Forced move extension */
	int delta_prune;		/* Quiescence delta pruning */

	/* Heuristics (mostly move ordering) */
	int heur_trans;			/* Transposition table */
	int heur_killer;		/* Killer Heuristic */
	int heur_cm;			/* Countermove Heuristic */
	int heur_trivial;		/* Trivial and cheap move ordering */

	/* Board evaluation */
	int h11n;			/* 1-1-n heuristic */
};

static const struct opts defopts = {
	.mode =			xboard,
	.nmoves =		0,
	.log =			1,
	.depth =		6,
	.timed = 		1,
	.timelimit =		1000,
	.lbound =		0,
	.smart_stop =		1,
	.shuffle =		1,
	.book =			1,
	.sort =			1,

	.verbosity =		0,

	.asp =			1,
	.ab =			1,
	.quiesce =		1,
	.null =			1,
	.lmr =			1,
	.forced_extend =	1,
	.delta_prune =		1,

	.heur_trans =		1,
	.heur_killer =		1,
	.heur_cm =		1,
	.heur_trivial =		1,

	.h11n =			1,
};

#ifdef FIXOPTS
#define copts defopts
#else
extern struct opts copts;
#endif

void dbg(char *s, ...);

#define mask_for_each(mask, temp, i)				\
	for ((temp) = (mask);					\
	     (temp) != 0 && ((i) = __builtin_ffsll(temp), 1);	\
	     (temp) = (temp) & ~((u64)1 << ((i)-1)))

#endif
