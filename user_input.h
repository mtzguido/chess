#ifndef __USER_INPUT_H__
#define __USER_INPUT_H__

#include "board.h"

void printMove(move m);
move parseMove(game g, char *line);

#endif
