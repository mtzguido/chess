#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#define ARRSIZE(a) ((int)((sizeof (a))/(sizeof ((a)[0]))))
#define __unused  __attribute__((unused))
#define __noinline __attribute__ ((noinline))

#define unlikely(c) __builtin_expect(c, 0)
#define   likely(c) __builtin_expect(c, 1)

#define min(a, b)			\
({					\
	typeof(a) __a = a;		\
	typeof(a) __b = b;		\
	(void) (&__a == &__b);		\
	__a < __b ? __a : __b;		\
})

#define max(a, b)			\
({					\
	typeof(a) __a = a;		\
	typeof(a) __b = b;		\
	(void) (&__a == &__b);		\
	__a > __b ? __a : __b;		\
})

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

unsigned long getms(void);
unsigned long getms_clock(void);

char pieceOf(char c);

enum play_mode {
	xboard,
	moves,
	board_eval,
	bench_eval,
	bench_search,
	bench_bits,
	h11n_table,
	print_sizes,
	version,
	help,
};

struct opts {
	int mode;			/* Program mode */

	int nmoves;			/* Amount of moves to calculate */
	int log;			/* Write gamelog to file*/
	int depth;			/* Maximum search depth */
	unsigned long timelimit;	/* Max time for each move, in ms (0 = inf)*/
	unsigned long lbound;		/* Min time for each move, in ms */
	int smart_stop;			/* Smart stopping */
	int shuffle;			/* Shuffle the succ moves */
	int book;			/* Use opening book */
	u64 bit_seed;			/* Seed for bit benching */

	bool custom_start;		/* Use a custom starting board */
	char custom_start_str[100];	/* Custom board spec */

	/* Misc options */
	int verbosity;			/* Verbosity level */
	int seed;			/* Random seed */
	int syslog;			/* Log to syslog too */

	/* Search */
	int asp;			/* Use aspiration windows */
	int ab;				/* Alpha-beta */
	int quiesce;			/* Use quiescence search on leaf nodes */
	int null;			/* Use Null Move Heuristic */
	int lmr;			/* Late move reduction */
	int forced_extend;		/* Forced move extension */
	int delta_prune;		/* Quiescence delta pruning */

	/* Board evaluation */
	int lazy;			/* Lazy evaluation */
	int h11n;			/* 1-1-n heuristic */
	int pps;			/* pps divider */
};

static const struct opts defopts = {
	.mode =			xboard,
	.nmoves =		0,
	.log =			1,
	.depth =		30,
	.timelimit =		0,
	.lbound =		0,
	.smart_stop =		1,
	.shuffle =		0,
	.book =			1,
	.bit_seed = 		(u64) 0xf0f0f0f0f0f0f0f0,

	.verbosity =		0,
	.syslog =		0,

	.asp =			1,
	.ab =			1,
	.quiesce =		1,
	.null =			1,
	.lmr =			1,
	.forced_extend =	1,
	.delta_prune =		1,

	.lazy =			1,
	.h11n =			1,
	.pps =			1,
};

#ifdef FIXOPTS
#define copts defopts
#else
extern struct opts copts;
#endif

void dbg(char *s, ...);

#include "bits.h"

#endif
