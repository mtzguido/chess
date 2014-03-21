#ifndef __AI_H__
#define __AI_H__

#include "board.h"

extern int machineColor;
extern int totalnopen;
extern int totalms;

game machineMove(game start);

typedef int score;

score heur(game g);

#endif
