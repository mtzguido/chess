#include "ai.h"
#include "eval.h"

static inline int pieceScore() {
	const int pps = interpolate(G->pps_O, G->pps_E);
	const int w = G->pieceScore[WHITE];
	const int b = G->pieceScore[BLACK];
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

static inline score eval_wpawn(const int i, const int j) {
	score ret = 0;

	if (G->pawn_rank[WHITE][j+1] > i)
		ret += DOUBLED_PAWN;

	if (G->pawn_rank[WHITE][j] == 0
			&& G->pawn_rank[WHITE][j+2] == 0)
		ret += ISOLATED_PAWN;

	if (G->pawn_rank[WHITE][j] < i &&
			G->pawn_rank[WHITE][j+2] < i)
		ret += BACKWARDS_PAWN;

	if (G->pawn_rank[BLACK][j] >= i &&
			G->pawn_rank[BLACK][j+1] >= i &&
			G->pawn_rank[BLACK][j+2] >= i)
		ret += (7 - i) * PASSED_PAWN;

	return ret;
}

static inline score eval_bpawn(const int i, const int j) {
	score ret = 0;

	if (G->pawn_rank[BLACK][j+1] < i)
		ret += DOUBLED_PAWN;

	if (G->pawn_rank[BLACK][j] == 0
			&& G->pawn_rank[BLACK][j+2] == 0)
		ret += ISOLATED_PAWN;

	if (G->pawn_rank[BLACK][j] > i &&
			G->pawn_rank[BLACK][j+2] > i)
		ret += BACKWARDS_PAWN;

	if (G->pawn_rank[WHITE][j] <= i &&
			G->pawn_rank[WHITE][j+1] <= i &&
			G->pawn_rank[WHITE][j+2] <= i)
		ret += i * PASSED_PAWN;

	return ret;
}

static inline score eval_pawn(const u8 col, const int i,
			      const int j) {
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
	assert(G->pawn_rank[BLACK][c+1] > 0);
	assert(G->pawn_rank[BLACK][c+1] < 8);
	assert(7 - G->pawn_rank[WHITE][c+1] > 0);
	assert(7 - G->pawn_rank[WHITE][c+1] < 8);
	return shield_own[7 - G->pawn_rank[WHITE][c+1]]
	     + shield_opp[G->pawn_rank[BLACK][c+1]];
}

static inline score eval_bshield(const int c) {
	assert(G->pawn_rank[BLACK][c+1] > 0);
	assert(G->pawn_rank[BLACK][c+1] < 8);
	assert(7 - G->pawn_rank[WHITE][c+1] > 0);
	assert(7 - G->pawn_rank[WHITE][c+1] < 8);
	return shield_own[G->pawn_rank[BLACK][c+1]]
	     + shield_opp[7 - G->pawn_rank[WHITE][c+1]];
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
		if (G->pawn_rank[BLACK][c] == 7 && G->pawn_rank[WHITE][c] == 0)
			ret -= 10;
		if (G->pawn_rank[BLACK][c+1] == 7 && G->pawn_rank[WHITE][c+1] == 0)
			ret -= 10;
		if (G->pawn_rank[BLACK][c+2] == 7 && G->pawn_rank[WHITE][c+2] == 0)
			ret -= 10;
	}

	return ret;
}

static inline score eval_with_ranks(const u8 col) {
	int bishop_count = 0;
	score score = 0;
	u64 temp;
	int i;

	const int top = col == WHITE ? 0 : 7;
	const int bot = col == WHITE ? 7 : 0;
	const int opp = flipTurn(col);

	mask_for_each(G->piecemask[col], temp, i) {
		const int r = (i-1) / 8;
		const int c = (i-1) % 8;
		const piece_t piece = G->board[r][c];

		switch (piece&7) {
		/* Evaluate pawns individually */
		case WPAWN:
			score += eval_pawn(col, r, c);
			break;

		/* Penalize knight at end game */
		case WKNIGHT:
			score += interpolate(0, KNIGHT_ENDGAME);
			break;

		/* Count bishops */
		case WBISHOP:
			bishop_count++;
			break;

		/* Bonus for rook on (semi)open files */
		case WROOK:
			if (G->pawn_rank[col][c+1] == top) {
				if (G->pawn_rank[opp][c+1] == bot)
					score += ROOK_OPEN_FILE;
				else
					score += ROOK_SEMI_OPEN_FILE;
			}
			break;

		/* Evaluate king safety */
		case WKING:
			score += eval_king(col, r, c)
				* G->pieceScore[opp] / SIDE_SCORE ;
			break;
		}
	}

	if (bishop_count > 1) score += DOUBLE_BISHOP;

	return score;
}

static inline score castle_score(const u8 col) {
	if (!G->castled[col]) {
		switch(2*G->castle_king[col] + G->castle_queen[col]) {
		case 0x00: return CASTLE_NN;
		case 0x01: return CASTLE_NY;
		case 0x02: return CASTLE_YN;
		case 0x03: return CASTLE_YY;
		}
	}
	return 0;
}

score boardEval() {
	score score = pieceScore(G);

	/*
	 * Con la información de los peones
	 * evaluamos filas abiertas y status de peones
	 */
	score += eval_with_ranks(WHITE);
	score -= eval_with_ranks(BLACK);

	/* Penalizamos según posibilidades de enroque */
	score += castle_score(WHITE);
	score -= castle_score(BLACK);

	if (inCheck(WHITE)) score += INCHECK;
	if (inCheck(BLACK)) score -= INCHECK;

	/*
	 * Acercamos a 0 los tableros que tengan
	 * muchos movimientos idle
	 */
	if (unlikely(G->idlecount > 100-FIFTYMOVE_THRESHOLD))
		score = (score * (100 - G->idlecount))/FIFTYMOVE_THRESHOLD;

	assert(score < maxScore);
	assert(score > minScore);

	return G->turn == WHITE ? score : -score;
}
