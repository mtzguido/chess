#ifndef __SEARCH_H
#define __SEARCH_H

#include "board.h"
#include "ai.h"

score search(game g, int maxDepth, move *mm, score alpha, score beta);

#endif
