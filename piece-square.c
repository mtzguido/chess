#include "piece-square.h"
#include "board.h"

#include <assert.h>

/*
 * Fuente:
 * http://chessprogramming.wikispaces.com/Simplified+evaluation+function
 */

static const int pawn[8][8];
static const int bishop[8][8];
static const int knight[8][8];
static const int rook[8][8];
static const int queen[8][8];
static const int kingO[8][8];
static const int kingE[8][8];

int piece_square_val(int piece, int phase, int r, int c) {
	int m = colorOf(piece) == WHITE ? 1 : -1;
	int p = abs(piece);

	assert(phase >= 0 && phase <= 47800);

	/* Simetría para el negro */
	if (colorOf(piece) != WHITE) {
		r = 7-r;
		c = 7-c;
	}

	switch (p) {
	case WPAWN:
		return m*(pawn[r][c]);
	case WBISHOP:
		return m*(bishop[r][c]);
	case WROOK:
		return m*(rook[r][c]);
	case WKNIGHT:
		return m*(knight[r][c]);
	case WQUEEN:
		return m*(queen[r][c]);
	case WKING:
		return m*((phase*(kingO[r][c] - kingE[r][c]))/47800 + kingE[r][c]);
	default:
		return 0;
	}
}

static const int pawn[8][8] = 
{
	{	0,	0,	0,	0,	0,	0,	0,	0	},
	{	80,	80,	80,	80,	80,	80,	80,	80	},
	{	10,	10,	20,	30,	30,	20,	10,	10	},
	{	5,	5,	10,	25,	25,	10,	5,	5	},
	{	0,	0,	0,	20,	20,	0,	0,	0	},
	{	5,	-5,	-10,	0,	0,	-10,	-5,	5	},
	{	5,	10,	10,	-20,	-20,	10,	10,	5	},
	{	0,	0,	0,	0,	0,	0,	0,	0	}
};

static const int knight[8][8] =
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

static const int bishop[8][8] = 
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

static const int rook[8][8] =
{
	{	0,	0,	0,	0,	0,	0,	0,	0	},
	{	5,	10,	10,	10,	10,	10,	10,	5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	-5,	0,	0,	0,	0,	0,	0,	-5	},
	{	0,	0,	0,	5,	5,	0,	0,	0	}
};

static const int queen[8][8] =
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

static const int kingO[8][8] =
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

static const int kingE[8][8] =
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

