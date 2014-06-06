#ifndef __ADDON_H__
#define __ADDON_H__

#include "board.h"
#include "ai.h"

typedef enum {
	FLAG_NONE,
	FLAG_EXACT,
	FLAG_LOWER_BOUND,
	FLAG_UPPER_BOUND
} flag_t;

struct addon {
	void (*reset)(void);
	void (*notify_return)(game g, move m, int depth, score s, flag_t flag);
	void (*notify_entry)(game g, int depth, score *alpha, score *beta);
	void (*notify_cut)(game g, move m, int depth);
	void (*score_succs)(game g, struct MS *ss,
			    int nsucc, int depth);
	void (*free_mem)(void);
	int  (*suggest)(game g, move *arr, int depth);
};

void addon_register(struct addon sa);

void addon_reset(void);
void addon_notify_return(game g, move m, int depth, score s, flag_t flag);
void addon_notify_entry(game g, int depth, score *alpha, score *beta);
void addon_notify_cut(game g, move m, int depth);
void addon_score_succs(game g, struct MS *ss,
		       int nsucc, int depth);
void addon_free_mem(void);
int  addon_suggest(game g, move **arr, int depth);

#endif

