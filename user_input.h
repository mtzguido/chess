#ifndef __USER_INPUT_H
#define __USER_INPUT_H

#include "board.h"
#include <stdio.h>

void move_text(move m, char *buf);
void printMove(FILE *stream, move m);
move parseMove(char *line);

#endif
