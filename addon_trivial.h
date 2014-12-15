#ifndef __ADDON_TRIVIAL_H
#define __ADDON_TRIVIAL_H

#include "addon.h"

void trivial_score_succs(int depth);

static inline void trivial_reset()
{
}

static inline void trivial_notify_cut(move m, int depth)
{
}

static inline void trivial_notify_return(move m, int depth, score score,
					 flag_t flag)
{
}

static inline void trivial_notify_entry(int depth, score *alpha, score *beta)
{
}

#endif
