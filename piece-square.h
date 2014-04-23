#ifndef __PIECE_SQUARE_H__
#define __PIECE_SQUARE_H__

#include "board.h"

#include "board.h"
#include <assert.h>

/*
 * Fuente:
 * http://chessprogramming.wikispaces.com/Simplified+evaluation+function
 */

char piece_square_val_O(i8 piece, i8 r, i8 c);
char piece_square_val_E(i8 piece, i8 r, i8 c);
__maybe_unused void piecePosFullRecalc(game g);

#endif
