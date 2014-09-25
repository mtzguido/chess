#ifndef __ADDON_CM_H
#define __ADDON_CM_H

#include "addon.h"

void cm_reset(void);
void cm_score_succs(int depth);
void cm_notify_cut(const move * const m, int depth);

static inline void cm_notify_return(const move * const m, int depth, score score,
				    flag_t flag)
{
}

static inline void cm_notify_entry(int depth, score *alpha, score *beta)
{
}

#endif
