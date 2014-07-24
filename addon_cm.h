#ifndef __ADDON_CM_H__
#define __ADDON_CM_H__

#include "addon.h"

void cm_reset() ;
void cm_score_succs(game g, int depth);
void cm_notify_cut(game g, move m, int depth);
int cm_suggest(game g, move *arr, int depth);

#define cm_notify_return(...)	do { } while (0)
#define cm_notify_entry(...)	do { } while (0)

#endif
