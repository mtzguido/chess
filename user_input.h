#ifndef __USER_INPUT_H
#define __USER_INPUT_H

#include "board.h"
#include <stdio.h>

void printMove(FILE *stream, move m);
move parseMove(char *line);

#endif
