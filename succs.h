#ifndef __SUCCS_H
#define __SUCCS_H

#include "board.h"

#define MAX_PLY	64

extern struct MS gsuccs[];
extern int first_succ[MAX_PLY];
extern int ply;

void genSuccs();
void genCaps();

#endif
