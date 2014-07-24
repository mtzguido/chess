#ifndef __AI_H__
#define __AI_H__

#include "board.h"

extern struct stats {
	long long depthsn[100];
	long long picked[100];
	long long nopen_s;
	long long nopen_q;
	long long nall;
	long long ngen;
	long long totalopen;
	long long totalms;
	long long null_cuts;
	long long tt_hits;
	long long lmrs;
	long long lmrs_ok;
} stats;

void print_stats(score exp);

move machineMove(game start);

score boardEval(game g);

/* Board evaluation scores */
#define ROOK_OPEN_FILE		15
#define ROOK_SEMI_OPEN_FILE	10
#define DOUBLED_PAWN		(-10)
#define ISOLATED_PAWN		(-20)
#define BACKWARDS_PAWN		(-8)
#define PASSED_PAWN		20

/* Move ordering scores */
#define CAPT_SCORE	200
#define PROMOTE_SCORE	300
#define CM_SCORE	500
#define KILLER_SCORE	1000
#define TRANS_SCORE	10000

#endif
