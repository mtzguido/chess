#ifndef __ADDON_TRIVIAL_H
#define __ADDON_TRIVIAL_H

#include "addon.h"

void trivial_score_succs(game g, int depth);

#define trivial_reset(...)		do { } while (0)
#define trivial_notify_cut(...)		do { } while (0)
#define trivial_suggest(...)		do { } while (0)
#define trivial_notify_return(...)	do { } while (0)
#define trivial_notify_entry(...)	do { } while (0)

#endif
