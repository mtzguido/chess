#ifndef __USER_INPUT_H
#define __USER_INPUT_H

#include "board.h"

void printMove(move m);
move parseMove(game g, char *line);

#endif
