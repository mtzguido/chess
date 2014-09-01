#include "ai.h"
#include "eval.h"

static inline int pieceScore(const game g) {
	const int pps = interpolate(g, g->pps_O, g->pps_E);
	const int w = g->pieceScore[WHITE];
	const int b = g->pieceScore[BLACK];
	int bonus;

	if (copts.h11n)
		bonus = w > b ? (w + 1)/(b + 1) : -(b + 1)/(w + 1);
	else
		bonus = 0;

	const int ret = (w - b) + bonus + pps;

	assert(ret <  99000);
	assert(ret > -99000);
	return ret;
}

/*
 * Compartido por todas las funciones de
 * evaluación de tablero.
 */
static int pawn_rank[2][10] = {
	[BLACK] = { [0] = 7, [9] = 7 },
	[WHITE] = { [0] = 0, [9] = 0 },
};

static inline void fill_ranks(const game g) {
	int r, c;

	for (c = 0; c < 8; c++) {
		for (r = 0; r < 7; r++) {
			if (g->board[r][c] == BPAWN) {
				pawn_rank[BLACK][c+1] = r;
				break;
			}
		}

		for (r = 7; r >= 0; r--) {
			if (g->board[r][c] == WPAWN) {
				pawn_rank[WHITE][c+1] = r;
				break;
			}
		}
	}
}

static inline score eval_wpawn(const int i, const int j) {
	score ret = 0;

	if (pawn_rank[WHITE][j+1] > i)
		ret += DOUBLED_PAWN;

	if (pawn_rank[WHITE][j] == 0
			&& pawn_rank[WHITE][j+2] == 0)
		ret += ISOLATED_PAWN;

	if (pawn_rank[WHITE][j] < i &&
			pawn_rank[WHITE][j+2] < i)
		ret += BACKWARDS_PAWN;

	if (pawn_rank[BLACK][j] >= i &&
			pawn_rank[BLACK][j+1] >= i &&
			pawn_rank[BLACK][j+2] >= i)
		ret += (7 - i) * PASSED_PAWN;

	return ret;
}

static inline score eval_bpawn(const int i, const int j) {
	score ret = 0;

	if (pawn_rank[BLACK][j+1] < i)
		ret += DOUBLED_PAWN;

	if (pawn_rank[BLACK][j] == 0
			&& pawn_rank[BLACK][j+2] == 0)
		ret += ISOLATED_PAWN;

	if (pawn_rank[BLACK][j] > i &&
			pawn_rank[BLACK][j+2] > i)
		ret += BACKWARDS_PAWN;

	if (pawn_rank[WHITE][j] <= i &&
			pawn_rank[WHITE][j+1] <= i &&
			pawn_rank[WHITE][j+2] <= i)
		ret += i * PASSED_PAWN;

	return ret;
}

static inline score eval_pawn(const u8 col, const int i, const int j) {
	if (col == WHITE)
		return eval_wpawn(i, j);
	else
		return eval_bpawn(i, j);
}

static const score shield_own[8] = {
	[0] = -999999,	/* Impossible */
	[1] = 0,	/* Never moved */
	[2] = -10,	/* Moved 1 square */
	[3] = -20,	/* Moved 2 or more squares */
	[4] = -20,
	[5] = -20,
	[6] = -20,
	[7] = -25,	/* No pawn */
};

static const score shield_opp[8] = {
	[0] = -999999,	/* Impossible */
	[1] = 5,	/* Never moved */
	[2] = 5,	/* Moved 1 or more squares */
	[3] = 5,
	[4] = -5,	/* Close to us */
	[5] = -10,	/* Closer */
	[6] = -25,	/* Dangerously close */
	[7] = 0,	/* No pawn */
};

/*
 * Subtract 7 for white to keep it from 0 to 7. 7 being
 * no pawn and 1 being a never advanced pawn
 */
static inline score eval_wshield(const int c) {
	assert(pawn_rank[BLACK][c+1] > 0);
	assert(pawn_rank[BLACK][c+1] < 8);
	assert(7 - pawn_rank[WHITE][c+1] > 0);
	assert(7 - pawn_rank[WHITE][c+1] < 8);
	return shield_own[7 - pawn_rank[WHITE][c+1]]
	     + shield_opp[pawn_rank[BLACK][c+1]];
}

static inline score eval_bshield(const int c) {
	assert(pawn_rank[BLACK][c+1] > 0);
	assert(pawn_rank[BLACK][c+1] < 8);
	assert(7 - pawn_rank[WHITE][c+1] > 0);
	assert(7 - pawn_rank[WHITE][c+1] < 8);
	return shield_own[pawn_rank[BLACK][c+1]]
	     + shield_opp[7 - pawn_rank[WHITE][c+1]];
}

static inline score eval_king(const u8 col, const int r, const int c) {
	score ret = 0;
	score (* const eval_shield)(int) =
		col == WHITE ? eval_wshield : eval_bshield;

	/* Evaluate the pawn shield when the king has castled */
	if (r < 3) {
		ret += eval_shield(0);
		ret += eval_shield(1);
		ret += eval_shield(2) / 2;
	} else if (r > 4) {
		ret += eval_shield(5) / 2;
		ret += eval_shield(6);
		ret += eval_shield(7);
	} else {
		/* Penalize open files near the king */
		if (pawn_rank[BLACK][c] == 7 && pawn_rank[WHITE][c] == 0)
			ret -= 10;
		if (pawn_rank[BLACK][c+1] == 7 && pawn_rank[WHITE][c+1] == 0)
			ret -= 10;
		if (pawn_rank[BLACK][c+2] == 7 && pawn_rank[WHITE][c+2] == 0)
			ret -= 10;
	}

	return ret;
}

static inline score eval_with_ranks(const u8 col, const game g) {
	int bishop_count = 0;
	score score = 0;
	u64 temp;
	int i;

	const int top = col == WHITE ? 0 : 7;
	const int bot = col == WHITE ? 7 : 0;
	const int opp = flipTurn(col);

	mask_for_each(g->piecemask[col], temp, i) {
		const int r = (i-1) / 8;
		const int c = (i-1) % 8;
		const piece_t piece = g->board[r][c];

		switch (piece&7) {
		/* Evaluate pawns individually */
		case WPAWN:
			score += eval_pawn(col, r, c);
			break;

		/* Penalize knight at end game */
		case WKNIGHT:
			score += interpolate(g, 0, KNIGHT_ENDGAME);
			break;

		/* Count bishops */
		case WBISHOP:
			bishop_count++;
			break;

		/* Bonus for rook on (semi)open files */
		case WROOK:
			if (pawn_rank[col][c+1] == top) {
				if (pawn_rank[opp][c+1] == bot)
					score += ROOK_OPEN_FILE;
				else
					score += ROOK_SEMI_OPEN_FILE;
			}
			break;

		/* Evaluate king safety */
		case WKING:
			score += eval_king(col, r, c)
				* g->pieceScore[opp] / SIDE_SCORE ;
			break;
		}
	}

	if (bishop_count > 1) score += DOUBLE_BISHOP;

	return score;
}

static inline score castle_score(const u8 col, const game g) {
	if (!g->castled[col]) {
		switch(2*g->castle_king[col] + g->castle_queen[col]) {
		case 0x00: return CASTLE_NN;
		case 0x01: return CASTLE_NY;
		case 0x02: return CASTLE_YN;
		case 0x03: return CASTLE_YY;
		}
	}
	return 0;
}

score boardEval(const game g) {
	score score = pieceScore(g);
	int i;

	assert(pawn_rank[WHITE][0] == 0);
	assert(pawn_rank[WHITE][9] == 0);
	assert(pawn_rank[BLACK][0] == 7);
	assert(pawn_rank[BLACK][9] == 7);

	for (i=1; i<9; i++) {
		pawn_rank[WHITE][i] = 0;
		pawn_rank[BLACK][i] = 7;
	}

	/*
	 * Primera pasada, llenamos pawn_rank con el peon
	 * menos avanzado de cada lado.
	 */
	fill_ranks(g);

	/*
	 * Segunda pasada. Con la información de los peones
	 * evaluamos filas abiertas y status de peones
	 */
	score += eval_with_ranks(WHITE, g);
	score -= eval_with_ranks(BLACK, g);

	/* Penalizamos según posibilidades de enroque */
	score += castle_score(WHITE, g);
	score -= castle_score(BLACK, g);

	if (inCheck(g, WHITE)) score += INCHECK;
	if (inCheck(g, BLACK)) score -= INCHECK;

	/*
	 * Acercamos a 0 los tableros que tengan
	 * muchos movimientos idle
	 */
	if (unlikely(g->idlecount > 100-FIFTYMOVE_THRESHOLD))
		score = (score * (100 - g->idlecount))/FIFTYMOVE_THRESHOLD;

	assert(score < maxScore);
	assert(score > minScore);

	return g->turn == WHITE ? score : -score;
}
