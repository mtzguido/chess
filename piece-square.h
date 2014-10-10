#ifndef __PIECE_SQUARE_H
#define __PIECE_SQUARE_H

#include "board.h"
#include <assert.h>

/*
 * Fuente:
 * http://chessprogramming.wikispaces.com/Simplified+evaluation+function
 */

signed char piece_square_val_O(piece_t piece, i8 r, i8 c);
signed char piece_square_val_E(piece_t piece, i8 r, i8 c);
void piecePosFullRecalc(void);

#endif
