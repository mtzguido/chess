#ifndef __AI_H
#define __AI_H

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
	long long null_tries;
	long long tt_hits;
	long long tt_collision;
	long long lmrs;
	long long lmrs_ok;
} stats;

/* Time limit data, for search.c */
extern bool timelimited;
extern bool timeup;
extern unsigned long timelimit;
extern unsigned long timestart;
extern int ticks;

move machineMove(unsigned long long maxms);

/* Board evaluation scores */
#define ROOK_OPEN_FILE		15
#define ROOK_SEMI_OPEN_FILE	10
#define DOUBLED_PAWN		(-10)
#define ISOLATED_PAWN		(-20)
#define BACKWARDS_PAWN		(-8)
#define PASSED_PAWN		20
#define INCHECK			(-100)
#define DOUBLE_BISHOP		15
#define KNIGHT_ENDGAME		(-10)

/* Castling penalties */
#define CASTLE_NN		(-15)
#define CASTLE_NY		(-12)
#define CASTLE_YN		(-8)
#define CASTLE_YY		(-5)

/* Move ordering scores */
#define CAPT_SCORE		200
#define CM_SCORE		500
#define PROMOTE_SCORE		800
#define KILLER_SCORE		1000
#define TRANS_SCORE		10000

/* Heuristics config */
#define NMH_THRESHOLD		700
#define NMH_REDUCTION		2
#define ASPIRATION_WINDOW	13
#define LMR_FULL		3
#define LMR_MINDEPTH		2

/* Misc tunables */
#define CHECKMATE_SCORE		100000

/* Most valuable victim / least valuable attacker */
static inline int mvv_lva(piece_t a, piece_t v) {
	return 10 * toWhite(v) - toWhite(a);
}

#endif
