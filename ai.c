#include "ai.h"
#include "board.h"
#include "common.h"
#include "ztable.h"
#include "mem.h"
#include "addon.h"
#include "succs.h"

#include <stdint.h>
#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Score definition */
static const score minScore = -1e7;
static const score maxScore =  1e7;


static void shuffleSuccs(game g, move *succs, int n);
static void sortSuccs(game g, move *succs, int n, int depth);
static score negamax(game start, int maxDepth, int curDepth,
			     move *mm, score alpha, score beta);

typedef int (*succgen_t)(game, move**, int);

static int genSuccs_wrap(game g, move **arr, int depth) {
	int n;

	/* Generar sucesores */
	n = genSuccs(g, arr);

	/* Mezclarlos si es necesario */
	if (copts.shuffle)
		shuffleSuccs(g, *arr, n);

	/* Ordenarlos si es necesario */
	if (depth < copts.depth - 1)
		sortSuccs(g, *arr, n, depth);

	return n;
}

/* Stats */
struct stats stats;

void reset_stats() {
	int i;

	n_collision = 0;
	stats.nopen = 0;
	stats.ngen = 0;
	stats.nbranch = 0;
	for (i = 0; i < 30; i++)
		stats.depthsn[i] = 0;
}

void print_stats(score exp, clock_t t1, clock_t t2) {
	int i;

	fprintf(stderr, "stats: searched %lld nodes in %.3f seconds\n", stats.nopen, 1.0*(t2-t1)/CLOCKS_PER_SEC);
	fprintf(stderr, "stats: branching aprox: %.3f\n", 1.0 * stats.nbranch / stats.nopen);
	fprintf(stderr, "stats: total nodes generated: %lld\n", stats.ngen);
	fprintf(stderr, "stats: depth:n_nodes - ");
	fprintf(stderr, "expected score: %i\n", exp);
	for (i = 0; stats.depthsn[i] != 0; i++) 
		fprintf(stderr, "%i:%i, ", i, stats.depthsn[i]);

	fprintf(stderr, "\n");

}

move machineMove(game start) {
	move ret;
	score t;
	clock_t t1,t2;
	int i;

	addon_reset();
	reset_stats();

	t1 = clock();
	for (i=1; i<=copts.depth; i++)
		t = negamax(start, i, 0, &ret, minScore, maxScore);
	t2 = clock();

	stats.totalopen += stats.nopen;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_stats(t, t1, t2);

	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);
	fflush(NULL);
	return ret;
}

static score negamax_(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta);

static score negamax(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta) {
	score rc1 = negamax_(g, maxDepth, curDepth, mm, alpha, beta);

	/* Wrap de negamax, para debug */
	/* printf("mm (%i/%i) (a=%i, b=%i) returns %i\n", curDepth, maxDepth, alpha, beta, rc); */

	return rc1;
}

static const succgen_t gen_funs[] = {
#ifdef CFG_SUGG
	addon_suggest,
#endif
	genSuccs_wrap,
};

static score negamax_(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta) {

	score t, ret, best;
	move *succs = NULL;
	int i, nsucc;
	int nvalid = 0;
	__maybe_unused int COPIED = 0;
	game ng;
	move bestmove;

	const score alpha_orig = alpha;
	__maybe_unused const score beta_orig = beta;

	if (isDraw(g)) {
		if (mm != NULL) {
			mm->move_type = -1;
			COPIED = 1;
		}
		assert(mm == NULL);
		ret = 0;
		goto out;
	}

	if (curDepth >= maxDepth && !inCheck(g, g->turn)) {
		if (mm != NULL) {
			mm->move_type = -1;
			COPIED = 1;
		}
		assert(mm == NULL);
		if (g->turn == WHITE)
			ret = boardEval(g);
		else
			ret = -boardEval(g);

		goto out;
	}

	if (mm == NULL)
		addon_notify_entry(g, maxDepth - curDepth, &alpha, &beta);

	if (copts.alphabeta && alpha >= beta) {
		ret = alpha;
		goto out;
	}

	unsigned ii;
	best = minScore;
	ng = galloc();
	for (ii = 0; ii < ARRSIZE(gen_funs); ii++) {

		nsucc = gen_funs[ii](g, &succs, curDepth);

		if (nsucc == 0) {
			freeSuccs(succs, nsucc);
			continue;
		}

		assert(nsucc > 0);
		assert(succs != NULL);
		stats.ngen += nsucc;

		for (i=0; i<nsucc; i++) {
			*ng = *g;

			if (!doMove(ng, succs[i]))
				continue;

			stats.nopen++;

			nvalid++;

			mark(ng);
			t = -negamax(ng, maxDepth, curDepth+1,
				     NULL, -beta, -alpha);
			unmark(ng);

			if (t > best) {
				best = t;
				bestmove = succs[i];
				if (mm != NULL) {
					*mm = succs[i];
					COPIED = 1;
				}
			}

			if (t > alpha)
				alpha = t;

			if (copts.alphabeta && beta <= alpha) {
				addon_notify_cut(g, succs[i], curDepth);
				break;
			}
		}

		freeSuccs(succs, nsucc);
	}
	freeGame(ng);

	stats.nbranch += nvalid;

	flag_t flag;

	if (nvalid == 0) {
		if (inCheck(g, g->turn))
			ret = -100000 + curDepth;
		else
			ret = 0; /* Stalemate */

		flag = FLAG_EXACT;
	} else {
		ret = best;

		if (best <= alpha_orig)
			flag = FLAG_UPPER_BOUND;
		else if (best > beta)
			flag = FLAG_LOWER_BOUND;
		else
			flag = FLAG_EXACT;
	}

	if (maxDepth - curDepth > 1)
		addon_notify_return(g, bestmove, maxDepth - curDepth, ret, flag);

out:
	if (mm != NULL)
		assert(COPIED);

	if (0 && curDepth == 1) {
		fprintf(stderr, "depth1: devuelvo %i (nvalid=%i)\n", ret, nvalid);
		fprintf(stderr, "hash= %" PRIx64 "\n", g->zobrist);

	}

	return ret;
}

static int pieceScore(game g) {
	int x = g->totalScore - 40000;
	int pps = (x*(g->pps_O - g->pps_E))/8000 + g->pps_E;

	return g->pieceScore  + pps;
}

score boardEval(game g) {
	score ret = 0;

	ret = pieceScore(g);

	if (!g->castled[WHITE]) {
		if (!g->castle_king[WHITE] && !g->castle_queen[WHITE])
			ret -= 15;
		else if (!g->castle_king[WHITE])
			ret -= 12;
		else if (!g->castle_queen[WHITE])
			ret -= 8;
	}

	if (!g->castled[BLACK]) {
		if (!g->castle_king[BLACK] && !g->castle_queen[BLACK])
			ret += 15;
		else if (!g->castle_king[BLACK])
			ret += 12;
		else if (!g->castle_queen[BLACK])
			ret += 8;
	}

	/*
	 * Acercamos a 0 los tableros que tengan
	 * muchos movimientos idle
	 */
	if (g->idlecount >= 68)
		ret = (ret * (100 - g->idlecount))/32;

	return ret;
}

static int MS_cmp(const void *_a, const void *_b) {
	const struct MS *a = _a;
	const struct MS *b = _b;

	/* Decreciente en score */
	return b->s - a->s;
}

static void sortSuccs(game g, move *succs, int n, int depth) {
	int i;
	struct MS *ss;

	ss = malloc(n * sizeof (struct MS));
	for (i=0; i<n; i++) {
		ss[i].m = succs[i];
		ss[i].s = 0;
	}

	addon_score_succs(g, ss, n, depth);
	qsort(ss, n, sizeof (struct MS), MS_cmp);

	for (i=0; i<n; i++)
		succs[i] = ss[i].m;

	free(ss);
}

static void shuffleSuccs(game g, move *succs, int n) {
	int i, j;
	move t;

	for (i=0; i<n-1; i++) {
		j = i + rand() % (n-i);

		t = succs[i];
		succs[i] = succs[j];
		succs[j] = t;
	}
}
