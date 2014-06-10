#include "piece-square.h"

#include <assert.h>

/*
 * Fuente:
 * http://chessprogramming.wikispaces.com/Simplified+evaluation+function
 */

static const char t_pawn[8][8];
static const char t_bishop[8][8];
static const char t_knight[8][8];
static const char t_rook[8][8];
static const char t_queen[8][8];
static const char t_kingO[8][8];
static const char t_kingE[8][8];

char piece_square_val_O(piece_t piece, i8 r, i8 c) {
	switch (piece) {
	case WPAWN:	return t_pawn[r][c];
	case WBISHOP:	return t_bishop[r][c];
	case WROOK:	return t_rook[r][c];
	case WKNIGHT:	return t_knight[r][c];
	case WQUEEN:	return t_queen[r][c];
	case WKING:	return t_kingO[r][c];
	case BPAWN:	return -t_pawn[7-r][7-c];
	case BBISHOP:	return -t_bishop[7-r][7-c];
	case BROOK:	return -t_rook[7-r][7-c];
	case BKNIGHT:	return -t_knight[7-r][7-c];
	case BQUEEN:	return -t_queen[7-r][7-c];
	case BKING:	return -t_kingO[7-r][7-c];
	}
	assert(0);
	__builtin_unreachable();
}

char piece_square_val_E(piece_t piece, i8 r, i8 c) {
	switch (piece) {
	case WPAWN:	return t_pawn[r][c];
	case WBISHOP:	return t_bishop[r][c];
	case WROOK:	return t_rook[r][c];
	case WKNIGHT:	return t_knight[r][c];
	case WQUEEN:	return t_queen[r][c];
	case WKING:	return t_kingE[r][c];
	case BPAWN:	return -t_pawn[7-r][7-c];
	case BBISHOP:	return -t_bishop[7-r][7-c];
	case BROOK:	return -t_rook[7-r][7-c];
	case BKNIGHT:	return -t_knight[7-r][7-c];
	case BQUEEN:	return -t_queen[7-r][7-c];
	case BKING:	return -t_kingE[7-r][7-c];
	}
	assert(0);
	__builtin_unreachable();
}

void piecePosFullRecalc(game g) {
	int i, j;

	g->pps_O = 0;
	g->pps_E = 0;

	for (i=0; i<8; i++) 
		for (j=0; j<8; j++) {
			if (!g->board[i][j])
				continue;

			g->pps_O += piece_square_val_O(g->board[i][j], i, j);
			g->pps_E += piece_square_val_E(g->board[i][j], i, j);
		}
}

static const char t_pawn[8][8] =
{
	{       0,	0,	0,	0,	0,	0,	0,	0	},
	{       50,	50,	50,	50,	50,	50,	50,	50	},
	{       10,	10,	20,	30,	30,	20,	10,	10	},
	{       5,	5,	10,	25,	25,	10,	5,	5	},
	{       0,	0,	0,	20,	20,	0,	0,	0	},
	{       5,	-5,	-10,	0,	0,	-10,	-5,	5	},
	{       5,	10,	10,	-20,	-20,	10,	10,	5	},
	{       0,	0,	0,	0,	0,	0,	0,	0	}
};

static const char t_knight[8][8] =
{
	{	-50,	-40,	-30,	-30,	-30,	-30,	-40,	-50	},
	{	-40,	-20,	0,	0,	0,	0,	-20,	-40	},
	{	-30,	0,	10,	15,	15,	10,	0,	-30	},
	{	-30,	5,	15,	20,	20,	15,	5,	-30	},
	{	-30,	0,	15,	20,	20,	15,	0,	-30	},
	{	-30,	5,	10,	15,	15,	10,	5,	-30	},
	{	-40,	-20,	0,	5,	5,	0,	-20,	-40	},
	{	-50,	-40,	-30,	-30,	-30,	-30,	-40,	-50	},
};

static const char t_bishop[8][8] = 
{
	{	-20,	-10,	-10,	-10,	-10,	-10,	-10,	-20	},
	{	-10,	0,	0,	0,	0,	0,	0,	-10	},
	{	-10,	0,	5,	10,	10,	5,	0,	-10	},
	{	-10,	5,	5,	10,	10,	5,	5,	-10	},
	{	-10,	0,	10,	10,	10,	10,	0,	-10	},
	{	-10,	10,	10,	10,	10,	10,	10,	-10	},
	{	-10,	5,	0,	0,	0,	0,	5,	-10	},
	{	-20,	-10,	-10,	-10,	-10,	-10,	-10,	-20	},
};

static const char t_rook[8][8] =
{
	{	0,	0,	0,	0,	0,	0,	0,	0	},
	{	15,	20,	20,	20,	20,	20,	20,	15	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	0,	0,	0,	5,	5,	0,	0,	0	}
};

static const char t_queen[8][8] =
{
	{	-20,	-10,	-10,	-5,	-5,	-10,	-10,	-20	},
	{	-10,	0,	0,	0,	0,	0,	0,	-10	},
	{	-10,	0,	5,	5,	5,	5,	0,	-10	},
	{	-5,	0,	5,	5,	5,	5,	0,	-5	},
	{	0,	0,	5,	5,	5,	5,	0,	-5	},
	{	-10,	5,	5,	5,	5,	5,	0,	-10	},
	{	-10,	0,	5,	0,	0,	0,	0,	-10	},
	{	-20,	-10,	-10,	-5,	-5,	-10,	-10,	-20 }
};

static const char t_kingO[8][8] =
{
	{	-30,	-40,	-40,	-50,	-50,	-40,	-40,	-30	},
	{	-30,	-40,	-40,	-50,	-50,	-40,	-40,	-30	},
	{	-30,	-40,	-40,	-50,	-50,	-40,	-40,	-30	},
	{	-30,	-40,	-40,	-50,	-50,	-40,	-40,	-30	},
	{	-20,	-30,	-30,	-40,	-40,	-30,	-30,	-20	},
	{	-10,	-20,	-20,	-20,	-20,	-20,	-20,	-10	},
	{	20,	20,	0,	0,	0,	0,	20,	20	},
	{	20,	30,	10,	0,	0,	10,	30,	20	}
};

static const char t_kingE[8][8] =
{
	{	-50,	-40,	-30,	-20,	-20,	-30,	-40,	-50	},
	{	-30,	-20,	-10,	0,	0,	-10,	-20,	-30	},
	{	-30,	-10,	20,	30,	30,	20,	-10,	-30	},
	{	-30,	-10,	30,	40,	40,	30,	-10,	-30	},
	{	-30,	-10,	30,	40,	40,	30,	-10,	-30	},
	{	-30,	-10,	20,	30,	30,	20,	-10,	-30	},
	{	-30,	-30,	0,	0,	0,	0,	-30,	-30	},
	{	-50,	-30,	-30,	-30,	-30,	-30,	-30,	-50	}
};
