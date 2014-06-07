#ifndef __AI_H__
#define __AI_H__

#include "board.h"

extern struct stats {
	int depthsn[30];
	long long nopen_s;
	long long nopen_q;
	long long nbranch;
	long long ngen;
	long long totalopen;
	long long totalms;
} stats;

move machineMove(game start);

score boardEval(game g);

#endif
