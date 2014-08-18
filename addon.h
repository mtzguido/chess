#ifndef __ADDON_H
#define __ADDON_H

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
void addon_suggest(game g, move *arr, int *n, int depth);

#endif

