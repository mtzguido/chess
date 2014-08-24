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
	[6] = -25,	/* ????? */
	[7] = 0,	/* No pawn */
};

/*
 * Subtract 7 for white to keep it from 0 to 7. 7 being
 * no pawn and 1 being a maximally advanced pawn
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

static inline score eval_wking(const int r, const int c) {
	score ret = 0;

	/* Evaluate the pawn shield when the king has castled */
	if (r < 3) {
		ret += eval_wshield(0);
		ret += eval_wshield(1);
		ret += eval_wshield(2) / 2;
	} else if (r > 4) {
		ret += eval_wshield(5) / 2;
		ret += eval_wshield(6);
		ret += eval_wshield(7);
	} else {
		if (pawn_rank[BLACK][c] == 7 && pawn_rank[WHITE][c] == 0)
			ret -= 10;
		if (pawn_rank[BLACK][c+1] == 7 && pawn_rank[WHITE][c+1] == 0)
			ret -= 10;
		if (pawn_rank[BLACK][c+2] == 7 && pawn_rank[WHITE][c+2] == 0)
			ret -= 10;
	}

	return ret;
}

static inline score eval_bking(const int r, const int c) {
	score ret = 0;

	/* Evaluate the pawn shield when the king has castled */
	if (r < 3) {
		ret += eval_bshield(0);
		ret += eval_bshield(1);
		ret += eval_bshield(2) / 2;
	} else if (r > 4) {
		ret += eval_bshield(5) / 2;
		ret += eval_bshield(6);
		ret += eval_bshield(7);
	} else {
		if (pawn_rank[BLACK][c] == 7 && pawn_rank[WHITE][c] == 0)
			ret -= 10;
		if (pawn_rank[BLACK][c+1] == 7 && pawn_rank[WHITE][c+1] == 0)
			ret -= 10;
		if (pawn_rank[BLACK][c+2] == 7 && pawn_rank[WHITE][c+2] == 0)
			ret -= 10;
	}

	return ret;
}

static inline score eval_with_ranks(const game g) {
	int i;
	int bishop_count[2] = {0};
	score score = 0;
	u64 temp;
	const u64 full = g->piecemask[WHITE] | g->piecemask[BLACK];

	mask_for_each(full, temp, i) {
		const int r = (i-1) / 8;
		const int c = (i-1) % 8;
		const piece_t piece = g->board[r][c];

		switch (piece) {
		/* Evaluate pawns individually */
		case WPAWN:
			score += eval_wpawn(r, c);
			break;
		case BPAWN:
			score -= eval_bpawn(r, c);
			break;

		/* Penalize knight at end game */
		case WKNIGHT:
			score += interpolate(g, 0, KNIGHT_ENDGAME);
			break;
		case BKNIGHT:
			score -= interpolate(g, 0, KNIGHT_ENDGAME);
			break;

		/* Count bishops */
		case WBISHOP:
			bishop_count[WHITE]++;
			break;
		case BBISHOP:
			bishop_count[BLACK]++;
			break;

		/* Bonus for rook on (semi)open files */
		case WROOK:
			if (pawn_rank[WHITE][c+1] == 0) {
				if (pawn_rank[BLACK][c+1] == 7)
					score += ROOK_OPEN_FILE;
				else
					score += ROOK_SEMI_OPEN_FILE;
			}
			break;
		case BROOK:
			if (pawn_rank[BLACK][c+1] == 7) {
				if (pawn_rank[WHITE][c+1] == 0)
					score -= ROOK_OPEN_FILE;
				else
					score -= ROOK_SEMI_OPEN_FILE;
			}
			break;

		/* Evaluate king safety */
		case WKING:
			score += eval_wking(r, c)
				* g->pieceScore[BLACK] / SIDE_SCORE ;
			break;

		case BKING:
			score -= eval_bking(r, c)
				* g->pieceScore[WHITE] / SIDE_SCORE ;
			break;
		}
	}

	if (bishop_count[WHITE] > 1) score += DOUBLE_BISHOP;
	if (bishop_count[BLACK] > 1) score -= DOUBLE_BISHOP;

	return score;
}

static inline score castle_score(const game g) {
	score score = 0;

	if (!g->castled[WHITE]) {
		switch(2*g->castle_king[WHITE] + g->castle_queen[WHITE]) {
		case 0x00: score += CASTLE_NN; break;
		case 0x01: score += CASTLE_NY; break;
		case 0x02: score += CASTLE_YN; break;
		case 0x03: score += CASTLE_YY; break;
		}
	}

	if (!g->castled[BLACK]) {
		switch(2*g->castle_king[BLACK] + g->castle_queen[BLACK]) {
		case 0x00: score -= CASTLE_NN; break;
		case 0x01: score -= CASTLE_NY; break;
		case 0x02: score -= CASTLE_YN; break;
		case 0x03: score -= CASTLE_YY; break;
		}
	}

	return score;
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
	score += eval_with_ranks(g);

	/* Penalizamos según posibilidades de enroque */
	score += castle_score(g);

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
