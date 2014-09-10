#ifndef __MOVES_H
#define __MOVES_H

bool doMove(move m);
bool doMove_unchecked(move m);
void undoMove(void);

#define MAX_HPLY 2000
extern int hply;

#endif
