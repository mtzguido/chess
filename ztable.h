#ifndef __ZTABLE_H__
#define __ZTABLE_H__

#include "board.h"

extern int NN;

struct bucket {
	game g;
	int n;
	struct bucket *next;
};

extern struct bucket * ztable[CFG_ZTABLE_SIZE];

extern int n_collision;

void mark(game g);
void unmark(game g);
int reps(game g);

#endif
