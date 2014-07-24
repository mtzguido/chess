#ifndef __ADDON_H__
#define __ADDON_H__

#include "succs.h"
#include "board.h"
#include "ai.h"

typedef enum {
	FLAG_NONE,
	FLAG_EXACT,
	FLAG_LOWER_BOUND,
	FLAG_UPPER_BOUND
} flag_t;

void addon_reset(void);
void addon_notify_return(game g, move m, int depth, score s, flag_t flag);
void addon_notify_entry(game g, int depth, score *alpha, score *beta);
void addon_notify_cut(game g, move m, int depth);
void addon_score_succs(game g, int depth);
void addon_free_mem(void);
int  addon_suggest(game g, move **arr, int depth);

/* Most valuable victim / least valuable attacker */
static inline int mvv_lva(piece_t a, piece_t v) {
	if (v != EMPTY)
		return 10 * (v&7) - (a&7);
	else
		return 0;
}

#endif

