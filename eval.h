#ifndef __EVAL_H
#define __EVAL_H

#include "ai.h"
#include "board.h"

/*
 * Sum of all evals, just for comfort in some
 * non time-critical places
 */
score boardEval(void);

typedef score (*evalFun_t)(void);

extern const int nEval;
extern const evalFun_t evalFuns[];
extern const score evalBound[];
extern const score fullBound;

#endif
