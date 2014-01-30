#ifndef __GAME_H__
#define __GAME_H__

#include "board.h"

int isLegalMove(game g, int row1, int col1, int row2, int col2);
int genSuccs(game g, game **arr);

#endif
