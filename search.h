#ifndef __SEARCH_H
#define __SEARCH_H

#include "board.h"
#include "succs.h"
#include "ai.h"

extern move pv[MAX_PLY][MAX_PLY];
extern int pv_len[MAX_PLY];

score search(int maxDepth, move *mm, score alpha, score beta);

#endif
