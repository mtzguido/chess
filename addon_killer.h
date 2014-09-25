#ifndef __ADDON_KILLER_H
#define __ADDON_KILLER_H

#include "addon.h"

void killer_reset(void);
void killer_score_succs(int depth);
void killer_notify_cut(const move * const m, int depth);

static inline void killer_notify_return(const move * const m, int depth, score score,
					flag_t flag)
{
}

static inline void killer_notify_entry(int depth, score *alpha, score *beta)
{
}

#endif
