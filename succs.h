#ifndef __SUCCS_H__
#define __SUCCS_H__

#include "board.h"

int genSuccs(game g, move **arr);
void freeSuccs(move *arr, int n);
int hasNextGame(game g);
int nSucc(game g);

#endif
