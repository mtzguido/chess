#ifndef __ADDON_TRANS_H__
#define __ADDON_TRANS_H__

#include "addon.h"

void trans_reset(void);

void trans_notify_return(game g, move move, int depth, score score,
			 flag_t flag);

#define trans_notify_entry(...)	do { } while (0)
#define trans_notify_cut(...)	do { } while (0)
#define trans_suggest(...)	do { } while (0)

void trans_score_succs(game g, int depth);

#endif
