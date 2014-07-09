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
	long long null_cuts;
} stats;

move machineMove(game start);

score boardEval(game g);

#define ROOK_OPEN_FILE		15
#define ROOK_SEMI_OPEN_FILE	10
#define DOUBLED_PAWN		(-10)
#define ISOLATED_PAWN		(-20)
#define BACKWARDS_PAWN		(-8)
#define PASSED_PAWN		20

#endif
