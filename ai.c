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

/* Score definition */
static const score minScore = -1e7;
static const score maxScore =  1e7;

struct stats stats;
static bool doing_null_move	= false;
static bool doing_lmr		= false;

static score quiesce(game g, score alpha, score beta, int curDepth,
		     int maxDepth);
static score negamax(game start, int maxDepth, int curDepth, move *mm,
		     score alpha, score beta);

static void genCaps_wrap(game g, int depth) {
	genCaps(g);
	stats.ngen += first_succ[ply+1] - first_succ[ply];

	addon_score_succs(g, depth);
}

static void genSuccs_wrap(game g, int depth) {
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
static void sort_succ(game g, int i, int depth_rem) {
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

static void reset_stats() {
	int i;

	n_collision	= 0;
	stats.nopen_s	= 0;
	stats.nopen_q	= 0;
	stats.ngen	= 0;
	stats.nall	= 0;
	stats.null_tot	= 0;
	stats.null_cuts	= 0;
	stats.tt_hits	= 0;
	stats.lmrs	= 0;
	stats.lmrs_ok	= 0;

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
			stats.null_cuts, stats.null_tot,
			100.0 * stats.null_cuts / stats.null_tot);
	fprintf(stderr, "stats: TT hits : %lld\n", stats.tt_hits);
	fprintf(stderr, "stats: Late move reductions : %lld/%lld (%.2f%%)\n",
			stats.lmrs_ok, stats.lmrs,
			100.0 * stats.lmrs_ok / stats.lmrs);
	fprintf(stderr, "stats: expected score: %i\n", exp);
	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);
}

static void print_time(clock_t t1, clock_t t2) {
	fprintf(stderr, "stats: moved in %.3f seconds\n",
			1.0*(t2-t1)/CLOCKS_PER_SEC);
}

static bool forced(const game g, move *m) {
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
	}
	t2 = clock();

	stats.totalopen += stats.nopen_s + stats.nopen_q;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_time(t1, t2);
	fflush(NULL);
	return ret;
}

static int calcExtension(game g, int maxDepth, int curDepth) {
	int ret = 0;

	if (inCheck(g, g->turn) || g->lastmove.promote != EMPTY)
		ret++;

	return ret;
}

static score quiesce(game g, score alpha, score beta, int curDepth, int maxDepth) {
	int nvalid, i;
	int ext;
	game ng;
	score ret, t;

	if (isDraw(g) || reps(g) >= 2)
		return 0;

	stats.nopen_q++;

	t = boardEval(g);

	if (t >= beta)
		return beta;

	if (t > alpha)
		alpha = t;

	ext = calcExtension(g, maxDepth, curDepth);
	maxDepth += ext;
	if (curDepth >= maxDepth)
		return t;

	ng = copyGame(g);
	genCaps_wrap(g, curDepth);
	nvalid = 0;
	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		sort_succ(g, i, maxDepth - curDepth);

		if (gsuccs[i].m.move_type != MOVE_REGULAR)
			continue;

		/* We only consider captures and promotions */
		assert(isCapture(g, gsuccs[i].m)
				|| isPromotion(g, gsuccs[i].m));

		if (!doMove_unchecked(ng, gsuccs[i].m))
			continue;

		nvalid++;

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

	if (nvalid == 0)
		ret = t;
	else
		ret = alpha;

out:
	freeGame(ng);

	assert(ret < maxScore);
	assert(ret > minScore);

	return ret;
}

static score negamax(game g, int maxDepth, int curDepth,
		     move *mm, score alpha, score beta) {
	score t, ret, best;
	int i;
	int ext;
	int nvalid = 0;
	game ng;
	int bestmove = -1;
	const score alpha_orig __maybe_unused = alpha;
	const score  beta_orig __maybe_unused = beta;

	stats.nall++;
	if (isDraw(g)) {
		ret = 0;
		assert(mm == NULL);
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
		assert(mm == NULL);

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

	if (mm == NULL)
		addon_notify_entry(g, maxDepth - curDepth, &alpha, &beta);

	if (alpha >= beta && copts.ab) {
		/*
		 * La posición es no-alcanzable por best play,
		 * devolvemos alpha_orig para no modificar nada
		 * en upstream. Deberíamos ser mas agresivos y devolver
		 * alpha? Es lo mismo?
		 */
		assert(0);
		ret = alpha_orig;
		assert(mm == NULL);
		goto out;
	}

	/* NMH */
	if (copts.nullmove
		&& beta < maxScore
		&& !doing_null_move
		&& !inCheck(g, g->turn)
		&& g->pieceScore[g->turn] > 21000
		&& maxDepth - curDepth > 1) {
		score t;
		move m = { .move_type = MOVE_NULL, .who = g->turn };

		assert(mm == NULL);

		ng = copyGame(g);

		if (doMove(ng, m)) {
			mark(ng);
			doing_null_move = true;
			t = -negamax(ng, maxDepth-3, curDepth+1,
				     NULL, -beta, -alpha);
			doing_null_move = false;
			unmark(ng);

			freeGame(ng);

			/* Failed high */
			if (t > beta) {
				stats.null_cuts++;
				ret = beta_orig;
				goto out;
			} else {
				stats.null_tot++;
			}
		} else {
			freeGame(ng);
		}
	}

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
		assert(mm == NULL);

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

static int pieceScore(const game g) {
	int x = g->pieceScore[WHITE] + g->pieceScore[BLACK] - 40000;
	int pps = (x*(g->pps_O - g->pps_E))/8000 + g->pps_E;

	return g->pieceScore[WHITE] - g->pieceScore[BLACK] + pps;
}

/*
 * Compartido por todas las funciones de
 * evaluación de tablero.
 */
int pawn_rank[2][10] = {
	[BLACK] = { [0] = 7, [9] = 7 },
	[WHITE] = { [0] = 0, [9] = 0 },
};

static void fill_ranks(const u8 rows[], const u8 cols[], const int npcs,
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

static score eval_wpawn(const int i, const int j) {
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

static score eval_bpawn(const int i, const int j) {
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

static score eval_with_ranks(const u8 rows[], const u8 cols[], const int npcs,
			     const game g) {
	int i;
	score score = 0;

	for (i=0; i<npcs; i++) {
		const int r = rows[i];
		const int c = cols[i];
		const piece_t piece = g->board[r][c];

		switch (piece) {
		case WPAWN:
			score += eval_wpawn(r, c);
			break;
		case WROOK:
			if (pawn_rank[WHITE][c+1] == 0) {
				if (pawn_rank[BLACK][c+1] == 7)
					score += ROOK_OPEN_FILE;
				else
					score += ROOK_SEMI_OPEN_FILE;
			}
			break;
		case BPAWN:
			score -= eval_bpawn(r, c);
			break;
		case BROOK:
			if (pawn_rank[BLACK][c+1] == 7) {
				if (pawn_rank[WHITE][c+1] == 0)
					score -= ROOK_OPEN_FILE;
				else
					score -= ROOK_SEMI_OPEN_FILE;
			}
			break;
		}
	}

	return score;
}

static score castle_score(const game g) {
	score score = 0;

	if (!g->castled[WHITE]) {
		switch(2*g->castle_king[WHITE] + g->castle_queen[WHITE]) {
		case 0x00: score -= 15; break;
		case 0x01: score -= 12; break;
		case 0x02: score -= 8;  break;
		case 0x03: score -= 5;  break;
		}
	}

	if (!g->castled[BLACK]) {
		switch(2*g->castle_king[BLACK] + g->castle_queen[BLACK]) {
		case 0x00: score += 15; break;
		case 0x01: score += 12; break;
		case 0x02: score += 8;  break;
		case 0x03: score += 5;  break;
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

	if (inCheck(g, WHITE)) score -= 200;
	if (inCheck(g, BLACK)) score += 200;

	/*
	 * Acercamos a 0 los tableros que tengan
	 * muchos movimientos idle
	 */
	if (unlikely(g->idlecount > 68))
		score = (score * (100 - g->idlecount))/32;

	assert(score < maxScore);
	assert(score > minScore);

	return g->turn == WHITE ? score : -score;
}
