#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_DEPTH 30

#define ARRSIZE(a) ((sizeof (a))/(sizeof ((a)[0])))
#define __maybe_unused  __attribute__((unused))

#define unlikely(c) __builtin_expect(c, 0)
#define   likely(c) __builtin_expect(c, 1)

#define clamp(v, l, h)	({		\
	typeof(v) __v = v;		\
	typeof(l) __l = l;		\
	typeof(h) __h = h;		\
	(void) (&__v == &__l);		\
	(void) (&__v == &__h);		\
	__v = __v < __l ? __l : __v;	\
	__v > __h ? __h : __v; })

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
	bool usebook;
	bool black;

	bool reverse;

	bool ab; /* Alpha-beta */
	bool quiesce;
	bool nullmove;

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
	.usebook = true,
	.black = false,

	.reverse = false,

	.ab = true,
	.quiesce = true,
	.nullmove = true,

	.heur_trans = false,
	.heur_killer = true,
	.heur_cm = true,
	.heur_trivial = true,
};

extern struct opts copts;

int on_bits(u64 x, u8 *rows, u8 *cols);

#endif
