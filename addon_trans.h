#ifndef __ADDON_TRANS_H
#define __ADDON_TRANS_H

#include "addon.h"

void trans_reset(void);
void trans_notify_return(move move, int depth, score score, flag_t flag);
void trans_score_succs(int depth);
void trans_notify_entry(int depth, score *alpha, score *beta);

static inline void trans_notify_cut(move m, int depth)
{
}

#endif
