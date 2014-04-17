#ifndef __PIECE_SQUARE_H__
#define __PIECE_SQUARE_H__

#include "board.h"

#include "board.h"
#include <assert.h>

/*
 * Fuente:
 * http://chessprogramming.wikispaces.com/Simplified+evaluation+function
 */

const char t_pawn[8][8];
const char t_bishop[8][8];
const char t_knight[8][8];
const char t_rook[8][8];
const char t_queen[8][8];
const char t_kingO[8][8];
const char t_kingE[8][8];

char piece_square_val_O(int piece, int r, int c);
char piece_square_val_E(int piece, int r, int c);
__maybe_unused void piecePosFullRecalc(game g);

#endif
