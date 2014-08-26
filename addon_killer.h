#ifndef __ADDON_KILLER_H
#define __ADDON_KILLER_H

#include "addon.h"

void killer_reset();
void killer_score_succs(game g __unused, int depth);
void killer_notify_cut(game g __unused, move m, int depth);
void killer_suggest(game g, move *arr, int *n, int depth);

#define killer_notify_entry(...)	do { } while (0)
#define killer_notify_return(...)	do { } while (0)

#endif
