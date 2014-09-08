#ifndef __ADDON_CM_H
#define __ADDON_CM_H

#include "addon.h"

void cm_reset(void);
void cm_score_succs(int depth);
void cm_notify_cut(move m, int depth);
void cm_suggest(move *arr, int *n, int depth);

#define cm_notify_return(...)	do { } while (0)
#define cm_notify_entry(...)	do { } while (0)

#endif
