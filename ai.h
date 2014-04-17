#ifndef __AI_H__
#define __AI_H__

#include "board.h"


/* Stats */
extern struct stats {
	int depthsn[30];
	int nopen;
	int nbranch;
	int ngen;
	int totalopen;
	int totalms;
} stats;

move machineMove(game start);

typedef int score;

score boardEval(game g);

#endif
