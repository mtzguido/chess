#ifndef __ADDON_KILLER_H__
#define __ADDON_KILLER_H__

#include "addon.h"

void killer_reset();
void killer_score_succs(game g __maybe_unused, int depth);
void killer_notify_cut(game g __maybe_unused, move m, int depth);
int killer_suggest(game g, move *arr, int depth);

#define killer_notify_entry(...)	do { } while (0)
#define killer_notify_return(...)	do { } while (0)

#endif
