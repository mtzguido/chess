#ifndef __MEM_H
#define __MEM_H

#include "board.h"

void init_mem(void);
game galloc(void);
void gfree(game g);

#endif
