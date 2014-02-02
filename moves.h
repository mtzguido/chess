#ifndef __MOVES_H__
#define __MOVES_H__

#include "board.h"

int isLegalMove(game g, int r, int c, int R, int C);
int isFinished(game g);

int genSuccs(game g, game **arr);

game doMove(game g, int r, int c, int R, int C);

#endif
