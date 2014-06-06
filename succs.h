#ifndef __SUCCS_H__
#define __SUCCS_H__

#include "board.h"

int genSuccs(game g, struct MS **arr);
void freeSuccs(struct MS *arr, int n);

#endif
