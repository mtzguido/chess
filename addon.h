#ifndef __ADDON_H__
#define __ADDON_H__

#include "board.h"
#include "ai.h"

struct addon {
	void (*reset)(void);
	void (*notify_return)(game g, move m, score s, int depth);
	bool (*notify_entry)(game g, int depth, score *ret);
	void (*notify_cut)(game g, move m, int depth);
	void (*score_succs)(game g, const game *succs,
			    score *vals, int nsucc, int depth);
	void (*free_mem)(void);
};

void addon_register(struct addon sa);

void addon_reset(void);
void addon_notify_return(game g, move m, score s, int depth);
bool addon_notify_entry(game g, int depth, score *ret);
void addon_notify_cut(game g, move m, int depth);
void addon_score_succs(game g, const game *succs,
		      score *vals, int nsucc, int depth);
void addon_free_mem(void);

#endif

