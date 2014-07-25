#ifndef __SUCCS_H
#define __SUCCS_H

#include "board.h"

extern struct MS gsuccs[];
extern int first_succ[];
extern int ply;

void genSuccs(game g);
void genCaps(game g);

#endif
