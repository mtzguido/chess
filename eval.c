#include "ai.h"
#include "check.h"
#include "moves.h"
#include "eval.h"
#include <stdio.h>

static score pieceScore() {
	const int w = G->pieceScore[WHITE];
	const int b = G->pieceScore[BLACK];
	int bonus;

	if (copts.h11n && w != b)
		bonus = w > b ? (w + 1)/(b + 1) : -(b + 1)/(w + 1);
	else
		bonus = 0;

	const int ret = w - b + bonus;

	assert(ret <  99000);
	assert(ret > -99000);
	return ret;
}

static score ppsScore() {
	return interpolate(G->pps_O, G->pps_E);
}

static score eval_wpawn(const int i, const int j) {
	score ret = 0;

	if (G->pawn_rank[WHITE][j+1] > i)
		ret += DOUBLED_PAWN;

	if (G->pawn_rank[WHITE][j] == 0
			&& G->pawn_rank[WHITE][j+2] == 0) {
		ret += ISOLATED_PAWN;
	} else if (G->pawn_rank[WHITE][j] < i &&
			G->pawn_rank[WHITE][j+2] < i) {
		ret += BACKWARDS_PAWN;
	}

	if (G->pawn_rank[BLACK][j] >= i &&
			G->pawn_rank[BLACK][j+1] >= i &&
			G->pawn_rank[BLACK][j+2] >= i)
		ret += (7 - i) * PASSED_PAWN;

	return ret;
}

static score eval_bpawn(const int i, const int j) {
	score ret = 0;

	if (G->pawn_rank[BLACK][j+1] < i)
		ret += DOUBLED_PAWN;

	if (G->pawn_rank[BLACK][j] == 7
			&& G->pawn_rank[BLACK][j+2] == 7) {
		ret += ISOLATED_PAWN;
	} else if (G->pawn_rank[BLACK][j] > i &&
			G->pawn_rank[BLACK][j+2] > i) {
		ret += BACKWARDS_PAWN;
	}

	if (G->pawn_rank[WHITE][j] <= i &&
			G->pawn_rank[WHITE][j+1] <= i &&
			G->pawn_rank[WHITE][j+2] <= i)
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
	[6] = -25,	/* Dangerously close */
	[7] = 0,	/* No pawn */
};

/*
 * Subtract 7 for white to keep it from 0 to 7. 7 being
 * no pawn and 1 being a never advanced pawn
 */
static score eval_wshield(const int c) {
	assert(G->pawn_rank[BLACK][c+1] > 0);
	assert(G->pawn_rank[BLACK][c+1] < 8);
	assert(7 - G->pawn_rank[WHITE][c+1] > 0);
	assert(7 - G->pawn_rank[WHITE][c+1] < 8);
	return shield_own[7 - G->pawn_rank[WHITE][c+1]]
	     + shield_opp[G->pawn_rank[BLACK][c+1]];
}

static score eval_bshield(const int c) {
	assert(G->pawn_rank[BLACK][c+1] > 0);
	assert(G->pawn_rank[BLACK][c+1] < 8);
	assert(7 - G->pawn_rank[WHITE][c+1] > 0);
	assert(7 - G->pawn_rank[WHITE][c+1] < 8);
	return shield_own[G->pawn_rank[BLACK][c+1]]
	     + shield_opp[7 - G->pawn_rank[WHITE][c+1]];
}

static score eval_king(const u8 col, const int r, const int c) {
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

static score eval_one_piece(const u8 col, const u8 r, const u8 c,
				   piece_t piece) {
	const int top = col == WHITE ? 0 : 7;
	const int bot = 7 ^ top;
	const int opp = flipTurn(col);

	assert(piece != EMPTY);
	assert(colorOf(piece) == col);
	assert(piece == G->board[r][c]);

	switch (piece) {
	/* Evaluate pawns individually */
	case WPAWN:
		return eval_wpawn(r, c);
	case BPAWN:
		return eval_bpawn(r, c);

	/* Penalize knight at end game */
	case WKNIGHT:
	case BKNIGHT:
		return interpolate(0, KNIGHT_ENDGAME);

	/* Bonus for rook on (semi)open files */
	case WROOK:
	case BROOK:
		if (G->pawn_rank[col][c+1] == top) {
			if (G->pawn_rank[opp][c+1] == bot)
				return ROOK_OPEN_FILE;
			else
				return ROOK_SEMI_OPEN_FILE;
		} else {
			return 0;
		}

	default:
		return 0;
	}

	__builtin_unreachable();
}

static score eval_with_ranks(const u8 col) {
	score score = 0;
	u64 temp;
	int i;
	const piece_t bishop = mkPiece(WBISHOP, col);

	mask_for_each(G->piecemask[col], temp, i) {
		const int r = bitrow(i);
		const int c = bitcol(i);
		const piece_t piece = G->board[r][c];

		score += eval_one_piece(col, r, c, piece);
	}

	if (G->n_piece[bishop] > 1)
		score += DOUBLE_BISHOP;

	return score;
}

static score castle_score(const u8 col) {
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

static score boardEval_material() {
	score score = 0;

	score += pieceScore(G);
	score += ppsScore(G) / copts.pps;

	assert(score < maxScore);
	assert(score > minScore);

	return G->turn == WHITE ? score : -score;
}

static score boardEval_structure() {
	score score = 0;

	stats.neval++;

	/*
	 * Con la información de los peones
	 * evaluamos filas abiertas y status de peones
	 */
	score += eval_with_ranks(WHITE);
	score -= eval_with_ranks(BLACK);

	assert(score < maxScore);
	assert(score > minScore);

	return G->turn == WHITE ? score : -score;
}

static score boardEval_castling() {
	score score = 0;

	/* Penalizamos según posibilidades de enroque */
	score += castle_score(WHITE);
	score -= castle_score(BLACK);

	assert(score < maxScore);
	assert(score > minScore);

	return G->turn == WHITE ? score : -score;
}

static score boardEval_king_col(const u8 col) {
	const u8 opp = flipTurn(col);
	const u8 r = G->kingx[col];
	const u8 c = G->kingy[col];

	/* Evaluate king safety */
	return eval_king(col, r, c) * G->pieceScore[opp] / SIDE_SCORE;
}
static score boardEval_king() {
	score score = 0;

	score += boardEval_king_col(WHITE);
	score -= boardEval_king_col(BLACK);

	return G->turn == WHITE ? score : -score;
}

/*
 * The only player that can be in check is the current one,
 * and that's the one we're evaluating for so there's no need
 * to check G->turn.
 */
static score boardEval_check() {
	if (inCheck())
		return INCHECK;
	else
		return 0;
}

const evalFun_t evalFuns[] = {
	boardEval_material,
	boardEval_castling,
	boardEval_check,
	boardEval_king,
	boardEval_structure,
};

const int nEval = ARRSIZE(evalFuns);

#define EVAL_MATERIAL_BOUND	9000
#define EVAL_CASTLING_BOUND	15
#define EVAL_CHECK_BOUND	100
#define EVAL_KING_BOUND		100
#define EVAL_STRUCTURE_BOUND	150

const score evalBound[] = {
	EVAL_MATERIAL_BOUND,
	EVAL_CASTLING_BOUND,
	EVAL_CHECK_BOUND,
	EVAL_KING_BOUND,
	EVAL_STRUCTURE_BOUND,
};

const score fullBound =
	EVAL_MATERIAL_BOUND +
	EVAL_CASTLING_BOUND +
	EVAL_CHECK_BOUND +
	EVAL_KING_BOUND +
	EVAL_STRUCTURE_BOUND;

score boardEval() {
	int i;
	score t;
	score score = 0;

	for (i = 0; i < nEval; i++) {
		t = evalFuns[i]();
		dbg("eval %i returned %i\n", i, t);
		score += t;
	}

	return score;
}

#define TABLE_STEP 250

int h11n_table_mode() {
	int i, j;
	int sc;

	startingGame();

	printf("w\\b\t");
	for (j = 0; j <= 4000; j += TABLE_STEP)
		printf("%i\t", j);
	printf("\n");
	printf("-----------------------------------------------------------"
	       "-----------------------------------------------------------\n");

	for (i = 0; i <= 4000; i += TABLE_STEP) {
		printf("%i\t| ", i);
		for (j = 0; j <= 4000; j += TABLE_STEP) {
			G->pieceScore[WHITE] = i;
			G->pieceScore[BLACK] = j;

			sc = pieceScore();
			printf("%i\t", sc);
		}
		printf("\n");
	}

	return 0;
}
