#include "ai.h"
#include "board.h"
#include "common.h"
#include "ztable.h"
#include "mem.h"
#include "addon.h"
#include "succs.h"
#include "book.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

/* Score definition */
static const score minScore = -1e7;
static const score maxScore =  1e7;

struct stats stats;
static bool doing_null_move	= false;
static bool doing_lmr		= false;

static inline score quiesce(game g, score alpha, score beta, int curDepth,
			    int maxDepth);
static inline score negamax(game start, int maxDepth, int curDepth, move *mm,
			    score alpha, score beta);

static inline void genCaps_wrap(game g, int depth) {
	genCaps(g);
	stats.ngen += first_succ[ply+1] - first_succ[ply];

	addon_score_succs(g, depth);
}

static inline void genSuccs_wrap(game g, int depth) {
	int i, j;

	/* Generar sucesores */
	genSuccs(g);
	stats.ngen += first_succ[ply+1] - first_succ[ply];

	addon_score_succs(g, depth);

	/* Mezclarlos si es necesario */
	if (copts.shuffle) {
		struct MS swap;
		const int lo = first_succ[ply];
		const int hi = first_succ[ply+1];

		for (i=lo; i<hi-1; i++) {
			j = i + rand() % (hi-i);

			if (i == j)
				continue;

			swap = gsuccs[j];
			gsuccs[j] = gsuccs[i];
			gsuccs[i] = swap;
		}
	}
}

/*
 * Ordena el arreglo de sucesores de manera lazy.
 * Deja en arr[i] el sucesor correcto, asume que arr[0..i-1] ya
 * está ordenado.
 */
static inline void sort_succ(game g, int i, int depth_rem) {
	if (!copts.sort)
		return;

	if (i == first_succ[ply+1] - 1) {
		/* Nada para hacer */
		return;
	}

	assert(gsuccs[i].m.who == g->turn);

	/* Ordenarlos si es necesario */
	if (depth_rem > 2) {
		int j;
		int best = i;
		score s = gsuccs[i].s;

		for (j=i+1; j<first_succ[ply+1]; j++) {
			assert(gsuccs[j].m.who == g->turn);
			if (gsuccs[j].s > s) {
				best = j;
				s = gsuccs[j].s;
			}
		}

		if (best != i) {
			struct MS swap;

			swap = gsuccs[best];
			gsuccs[best] = gsuccs[i];
			gsuccs[i] = swap;
		}
	}

	assert(gsuccs[i].m.who == g->turn);
	assert(gsuccs[i].m.move_type != MOVE_INVAL);
	assert(gsuccs[i].s >= 0);
}

static inline void reset_stats() {
	int i;

	n_collision		= 0;
	stats.nopen_s		= 0;
	stats.nopen_q		= 0;
	stats.ngen		= 0;
	stats.nall		= 0;
	stats.null_tries	= 0;
	stats.null_cuts		= 0;
	stats.tt_hits		= 0;
	stats.lmrs		= 0;
	stats.lmrs_ok		= 0;

	for (i = 0; i < 100; i++) {
		stats.depthsn[i] = 0;
		stats.picked[i] = 0;
	}
}

void print_stats(score exp) {
	fprintf(stderr, "stats: searched %lld (%lld) nodes\n",
			stats.nopen_s, stats.nopen_q);
	fprintf(stderr, "stats: branching aprox: %.3f\n",
			1.0 * (stats.nall - 1) / stats.nopen_s);
	fprintf(stderr, "stats: total nodes generated: %lld\n", stats.ngen);
	fprintf(stderr, "stats: null move cuts: %lld/%lld (%.2f%%)\n",
			stats.null_cuts, stats.null_tries,
			100.0 * stats.null_cuts / stats.null_tries);
	fprintf(stderr, "stats: TT hits : %lld\n", stats.tt_hits);
	fprintf(stderr, "stats: Late move reductions : %lld/%lld (%.2f%%)\n",
			stats.lmrs_ok, stats.lmrs,
			100.0 * stats.lmrs_ok / stats.lmrs);
	fprintf(stderr, "stats: expected score: %i\n", exp);
	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);
}

static inline void print_time(clock_t t1, clock_t t2) {
	fprintf(stderr, "stats: moved in %.3f seconds\n",
			1.0*(t2-t1)/CLOCKS_PER_SEC);
}

static inline bool forced(const game g, move *m) {
	int i;
	int c = -1;
	game ng;

	assert(ply == 0);
	genSuccs(g);
	ng = copyGame(g);
	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		if (doMove_unchecked(ng, gsuccs[i].m)) {
			if (c != -1) {
				freeGame(ng);
				return false;
			}

			c = i;
			*ng = *g;
		}
	}

	assert(c != -1);
	*m = gsuccs[c].m;
	freeGame(ng);
	return true;
}

move machineMove(const game start) {
	move ret = {0};
	clock_t t1,t2;

	ret.move_type = MOVE_INVAL;

	addon_reset();
	reset_stats();

	t1 = clock();
	if (copts.usebook && bookMove(start, &ret)) {
		fprintf(stderr, "stats: book move.\n");
	} else if (forced(start, &ret)) {
		fprintf(stderr, "stats: forced move.\n");
	} else {
		/* Profundización para llenar la TT */
		if (copts.iter) {
			int d;
			for (d=2 - copts.depth%2; d<copts.depth; d += 2) {
				assert(ply == 0);
				negamax(start, d, 0, NULL, minScore, maxScore);
			}
		}

		assert(ply == 0);
		score t = negamax(start, copts.depth, 0,
				  &ret, minScore, maxScore);

		assert(ret.move_type != MOVE_INVAL);
		assert(ret.who == start->turn);
		print_stats(t);
		fprintf(stderr, "move was %i %i %i %i %i\n",
				ret.move_type, ret.r, ret.c, ret.R, ret.C);
	}
	t2 = clock();

	stats.totalopen += stats.nopen_s + stats.nopen_q;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_time(t1, t2);
	fflush(NULL);
	return ret;
}

static inline int calcExtension(game g, int maxDepth, int curDepth) {
	int ret = 0;

	if (inCheck(g, g->turn) || g->lastmove.promote != EMPTY)
		ret++;

	return ret;
}

static inline score null_move_score(game g, int curDepth, int maxDepth,
				    score alpha, score beta) {
	score t;
	game ng;
	move m = { .move_type = MOVE_NULL, .who = g->turn };

	if (!copts.nullmove)
		goto dont;

	/* Dont do two null moves in the same variation */
	if (doing_null_move)
		goto dont;

	/*
	 * Dont null-move when in check or when low in material since
	 * we're likely to be in Zugzwang
	 */
	if (inCheck(g, g->turn) || g->pieceScore[g->turn] <= NMH_THRESHOLD)
		goto dont;

	/* Not even worth it */
	if (maxDepth - curDepth <= 1)
		goto dont;

	ng = copyGame(g);
	if (doMove(ng, m)) {
		stats.null_tries++;

		mark(ng);
		doing_null_move = true;
		t = -negamax(ng, maxDepth-3, curDepth+1, NULL, -beta, -alpha);
		doing_null_move = false;
		unmark(ng);
	} else {
		/*
		 * doMoveNull's only restriction is not being in check
		 * and we already provided a case for that so this
		 * should never be reached
		 */
		assert(0);
		t = minScore;
	}

	freeGame(ng);
	return t;

dont:
	return alpha;
}

static inline score quiesce(game g, score alpha, score beta, int curDepth,
			    int maxDepth) {
	int nvalid, i;
	int ext, onlymove;
	game ng;
	score ret, t;

	if (isDraw(g) || reps(g) >= 2)
		return 0;

	stats.nopen_q++;

	t = boardEval(g);

	if (t >= beta)
		return beta;

	if (copts.delta_prune) {
		score delta = scoreOf(WQUEEN) - scoreOf(WPAWN);

		if (g->lastmove.promote != EMPTY)
			delta += scoreOf(WQUEEN);

		if (t + delta < alpha)
			return alpha;
	}

	if (t > alpha)
		alpha = t;

	if (ply >= MAX_PLY-1)
		return t;

	ext = calcExtension(g, maxDepth, curDepth);
	maxDepth += ext;
	if (curDepth >= maxDepth)
		return t;

	const score alpha_orig = alpha;
	ng = copyGame(g);
	genCaps_wrap(g, curDepth);
	nvalid = 0;
	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		sort_succ(g, i, maxDepth - curDepth);

		assert(gsuccs[i].m.move_type == MOVE_REGULAR);

		/* We only consider captures and promotions */
		assert(isCapture(g, gsuccs[i].m)
				|| isPromotion(g, gsuccs[i].m));

		if (!doMove_unchecked(ng, gsuccs[i].m))
			continue;

		nvalid++;
		onlymove = i;

		mark(ng);
		ply++;
		t = -quiesce(ng, -beta, -alpha, curDepth+1, maxDepth);
		ply--;
		unmark(ng);
		*ng = *g;

		if (t > alpha) {
			if (t >= beta) {
				ret = beta;
				goto out;
			}

			alpha = t;
		}
	}

	if (nvalid == 0) {
		ret = t;
	} else if (nvalid == 1 && alpha < beta && copts.forced_extend) {
		__maybe_unused bool check =
			doMove_unchecked(ng, gsuccs[onlymove].m);
		assert(check);

		/*
		 * If only one move was valid, we're in a forced position,
		 * so re-search everything with +1 to the depth
		 */

		alpha = alpha_orig;

		mark(ng);
		ply++;
		t = -quiesce(ng, -beta, -alpha, curDepth+1, maxDepth+1);
		ply--;
		unmark(ng);

		if (t > alpha)
			alpha = t;

		ret = alpha;
	} else {
		ret = alpha;
	}

out:
	freeGame(ng);

	assert(ret < maxScore);
	assert(ret > minScore);

	return ret;
}

static inline score negamax(game g, int maxDepth, int curDepth, move *mm,
			    score alpha, score beta) {
	score t, ret, best, alpha_orig;
	int i;
	int ext;
	int nvalid = 0;
	game ng;
	int bestmove = -1;

	stats.nall++;
	if (isDraw(g)) {
		ret = 0;
		assert(!mm);
		goto out;
	}

	/*
	 * Si ya pasamos por este tablero, podemos asumir
	 * que vamos a reaccionar igual y vamos a llevar a un empate
	 * por repetición.
	 */
	if (reps(g) >= 2 && !mm) {
		ret = 0;
		goto out;
	}

	ext = calcExtension(g, maxDepth, curDepth);
	maxDepth += ext;

	/*
	 * Corte por profundidad, hacemos búsqueda por quietud, para
	 * mejorar nuestra evaluación de tablero.
	 */
	if (curDepth >= maxDepth) {
		assert(!mm);

		/*
		 * Si esto ocurre, tenemos una recursion mutua
		 * infinita con quiesce. No debería ocurrir nunca,
		 * pero dejamos el assert por las dudas.
		 */
		assert(!inCheck(g, g->turn));

		if (copts.quiesce)
			ret = quiesce(g, alpha, beta, curDepth, 999);
		else
			ret = boardEval(g);

		goto out;
	}

	/*
	 * Only try to null-move if beta was less than maxScore.
	 * Otherwise we will never suceed in the test.
	 */
	if (beta < maxScore) {
		assert(!mm);
		t = null_move_score(g, curDepth, maxDepth, alpha, beta);
		if  (t > beta) {
			stats.null_cuts++;
			return beta;
		}
	}

	if (!mm)
		addon_notify_entry(g, maxDepth - curDepth, &alpha, &beta);

	if (alpha >= beta && copts.ab) {
		/* Deshabilitado por ahora */
		assert(0);
		ret = alpha;
		assert(!mm);
		goto out;
	}

	if (ply >= MAX_PLY-1)
		return boardEval(g);

	alpha_orig = alpha;
	best = minScore;
	ng = copyGame(g);

	stats.nopen_s++;
	stats.depthsn[curDepth]++;

	genSuccs_wrap(g, curDepth);

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		sort_succ(g, i, maxDepth - curDepth);

		if (!doMove_unchecked(ng, gsuccs[i].m))
			continue;

		nvalid++;

		mark(ng);

		/* LMR */
		if (copts.lmr
			&& !doing_lmr
			&& i >= 4 + first_succ[ply] /* crap */
			&& curDepth >= 2
			&& gsuccs[i].s*10 < gsuccs[first_succ[ply]].s /* 2x crap */
			&& ext == 0
			&& !inCheck(ng, ng->turn)
			&& !isCapture(g, gsuccs[i].m)
			&& !isPromotion(g, gsuccs[i].m)) {
			stats.lmrs++;

			doing_lmr = true;
			ply++;
			t = -negamax(ng, maxDepth-1, curDepth+1, NULL, -beta, -alpha);
			ply--;
			doing_lmr = false;

			/* Do a full search if it didn't fail low */
			if (t > alpha && t < beta) {
				ply++;
				t = -negamax(ng, maxDepth, curDepth+1, NULL,
					     -beta, -alpha);
				ply--;
			} else {
				stats.lmrs_ok++;
			}
		} else {
			ply++;
			t = -negamax(ng, maxDepth, curDepth+1, NULL, -beta, -alpha);
			ply--;
		}

		unmark(ng);

		/* Ya no necesitamos a ng */
		*ng = *g;

		if (t > best) {
			best = t;
			bestmove = i;
		}

		if (t > alpha)
			alpha = t;

		if (alpha >= beta && copts.ab) {
			addon_notify_cut(g, gsuccs[i].m, curDepth);
			break;
		}
	}

	if (bestmove != -1)
		stats.picked[bestmove - first_succ[ply]]++;
	else
		assert(nvalid == 0);

	/* Era un tablero terminal? */
	if (nvalid == 0) {
		assert(!mm);

		move dummy = {0};

		if (inCheck(g, g->turn))
			ret = -100000 + curDepth;
		else
			ret = 0; /* Stalemate */

		/*
		 * Lo guardamos en la TT, podría ahorrar unos
		 * pocos ciclos
		 */
		addon_notify_return(g, dummy, 999, ret, FLAG_EXACT);
	} else if (nvalid == 1 && alpha < beta && copts.forced_extend) {
		bool check __maybe_unused;
		check = doMove(ng, gsuccs[bestmove].m);
		assert(check);

		alpha = alpha_orig;
		mark(ng);
		ply++;
		t = -negamax(ng, maxDepth+1, curDepth+1, NULL, -beta, -alpha);
		ply--;
		unmark(ng);

		if (t > alpha)
			alpha = t;

		ret = alpha;
	} else {
		flag_t flag;

		assert(bestmove != -1);

		if (mm)
			*mm = gsuccs[bestmove].m;

		/*
		 * Devolver alpha o best es lo mismo (o son ambos menores o
		 * iguales a alpha_orig o son iguales) pero en la TT debemos
		 * guardar ${WHAT} porque ${REASON}
		 */
		assert(best == alpha ||
				(best < alpha_orig && alpha == alpha_orig));

		ret = best;

		if (best <= alpha_orig)
			flag = FLAG_UPPER_BOUND;
		else if (best > beta)
			flag = FLAG_LOWER_BOUND;
		else
			flag = FLAG_EXACT;

		if (maxDepth - curDepth > 1) {
			addon_notify_return(g, gsuccs[bestmove].m,
					    maxDepth - curDepth, ret, flag);
		}
	}

	freeGame(ng);

out:
	if (mm)
		assert(mm->move_type != MOVE_INVAL);

	assert(ret > minScore);
	assert(ret < maxScore);

	return ret;
}

static inline int pieceScore(const game g) {
	const int pps = interpolate(g, g->pps_O, g->pps_E);
	const int w = g->pieceScore[WHITE];
	const int b = g->pieceScore[BLACK];
	double bonus;

	if (copts.h11n)
		bonus = fabs(log2((double)(w + 1)/(b + 1)));
	else
		bonus = 0;

	const int ret = (w - b) * (1 + bonus) + pps;

	assert(ret <  99000);
	assert(ret > -99000);
	return ret;
}

/*
 * Compartido por todas las funciones de
 * evaluación de tablero.
 */
int pawn_rank[2][10] = {
	[BLACK] = { [0] = 7, [9] = 7 },
	[WHITE] = { [0] = 0, [9] = 0 },
};

static inline void fill_ranks(const u8 rows[], const u8 cols[], const int npcs,
			      const game g) {
	int i;

	for (i=0; i<npcs; i++) {
		const int r = rows[i];
		const int c = cols[i];
		const piece_t piece = g->board[r][c];

		switch (piece) {
		case WPAWN:
			if (pawn_rank[WHITE][c+1] < r)
				pawn_rank[WHITE][c+1] = r;
			break;

		case BPAWN:
			if (pawn_rank[BLACK][c+1] > r)
				pawn_rank[BLACK][c+1] = r;
			break;
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

static inline score eval_wshield(const int c) {
	score ret = 0;

	switch (pawn_rank[WHITE][c+1]) {
		case 6:		ret -= 0;	/* Never moved */
		case 5:		ret -= 10;	/* Moded 1 square */
		case 0:		ret -= 25;	/* No pawn */
		default:	ret -= 20;	/* Somewhere else */
	}

	switch (pawn_rank[BLACK][c+1]) {
		case 7:		ret -= 0;	/* No pawn */
		case 4:		ret -= 5;	/* Close */
		case 5:		ret -= 10;	/* Closer */
		default:	ret += 5;	/* Somewhere else */
	}

	return ret;
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

static inline score eval_bshield(const int c) {
	score ret = 0;

	switch (pawn_rank[BLACK][c+1]) {
		case 1:		ret -= 0;	/* Never moved */
		case 2:		ret -= 10;	/* Moded 1 square */
		case 7:		ret -= 25;	/* No pawn */
		default:	ret -= 20;	/* Somewhere else */
	}

	switch (pawn_rank[WHITE][c+1]) {
		case 0:		ret -= 0;	/* No pawn */
		case 3:		ret -= 5;	/* Close */
		case 2:		ret -= 10;	/* Closer */
		default:	ret += 5;	/* Somewhere else */
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

static inline score eval_with_ranks(const u8 rows[], const u8 cols[],
				    const int npcs, const game g) {
	int i;
	int bishop_count[2] = {0};
	score score = 0;

	for (i=0; i<npcs; i++) {
		const int r = rows[i];
		const int c = cols[i];
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
			score -= interpolate(g, 0, KNIGHT_ENDGAME);
			break;
		case BKNIGHT:
			score += interpolate(g, 0, KNIGHT_ENDGAME);
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
	u8 rows[32], cols[32];
	int i, npcs;

	assert(pawn_rank[WHITE][0] == 0);
	assert(pawn_rank[WHITE][9] == 0);
	assert(pawn_rank[BLACK][0] == 7);
	assert(pawn_rank[BLACK][9] == 7);

	for (i=1; i<9; i++) {
		pawn_rank[WHITE][i] = 0;
		pawn_rank[BLACK][i] = 7;
	}

	npcs = on_bits(g->piecemask[WHITE] | g->piecemask[BLACK], rows, cols);

	/*
	 * Primera pasada, llenamos pawn_rank con el peon
	 * menos avanzado de cada lado.
	 */
	fill_ranks(rows, cols, npcs, g);

	/*
	 * Segunda pasada. Con la información de los peones
	 * evaluamos filas abiertas y status de peones
	 */
	score += eval_with_ranks(rows, cols, npcs, g);

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
