#include "ai.h"
#include "board.h"
#include "common.h"
#include "ztable.h"
#include "mem.h"
#include "addon.h"
#include "succs.h"

#include <stdint.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Score definition */
static const score minScore = -1e7;
static const score maxScore =  1e7;


struct stats stats;

static score negamax(game start, int maxDepth, int curDepth,
			     move *mm, score alpha, score beta);

static int genSuccs_wrap(game g, struct MS **arr, int depth) {
	int n;

	/* Generar sucesores */
	n = genSuccs(g, arr);
	stats.ngen += n;

	addon_score_succs(g, *arr, n, depth);

	return n;
}

static void sort_succ(game g, struct MS *arr, int i, int len, int depth) {
	/* Mezclarlos si es necesario */
	if (copts.shuffle) {
		int t = rand()%(len-i) + i;
		struct MS swap;
		swap = arr[i];
		arr[i] = arr[t];
		arr[t] = swap;
	}

	/* Ordenarlos si es necesario */
	if (depth < copts.depth - 1) {
		int j;
		int best = i;
		score s = arr[i].s;

		for (j=i+1; j<len; j++) {
			if (arr[j].s > s) {
				best = j;
				s = arr[j].s;
			}
		}

		struct MS swap;
		swap = arr[best];
		arr[best] = arr[i];
		arr[i] = swap;
	}
}

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
	score t = 0;
	clock_t t1,t2;
	int i;

	addon_reset();
	reset_stats();

	t1 = clock();
	for (i=1; i<copts.depth; i++)
		negamax(start, i, 0, NULL, minScore, maxScore);

	t = negamax(start, i, 0, &ret, minScore, maxScore);
	assert(ret.move_type >= 0);
	t2 = clock();

	stats.totalopen += stats.nopen;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_stats(t, t1, t2);

	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);
	fflush(NULL);
	return ret;
}

static score quiesce(game g, score alpha, score beta, int d) {
	score t;

	int nsucc, i;
	int nvalid;
	game ng;
	struct MS *succs;
	score ret;

	if (isDraw(g))
		return 0;

	t = boardEval(g);
	if (g->turn == BLACK)
		t = -t;

	if (d > 5)
		return t;

	if (t >= beta)
		return beta;

	if (t > alpha)
		alpha = t;

	ng = copyGame(g);
	nsucc = genSuccs(g, &succs);
	nvalid = 0;
	for (i=0; i<nsucc; i++) {
		sort_succ(g, succs, i, nsucc, d);

		if (succs[i].m.move_type != MOVE_REGULAR)
			continue;

		if (!enemy_piece(g, succs[i].m.R, succs[i].m.C))
			continue;

		if (!doMove_unchecked(ng, succs[i].m))
			continue;

		stats.nopen++;
		nvalid++;

		mark(ng);
		t = -quiesce(ng, -beta, -alpha, d+1);
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
		ret = 0;
	else
		ret = alpha;

out:
	freeSuccs(succs, nsucc);
	freeGame(ng);

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

static score negamax_(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta) {

	score t, ret, best;
	struct MS *succs = NULL;
	int i, nsucc;
	int nvalid = 0;
	game ng;
	move bestmove;
	const score alpha_orig = alpha;

	if (isDraw(g)) {
		ret = 0;
		assert(mm == NULL);
		goto out;
	}

	if (inCheck(g, g->turn))
		maxDepth++;

	if (curDepth >= maxDepth) {
		ret = quiesce(g, alpha, beta, 0);
		goto out;
	}
	if (mm == NULL)
		addon_notify_entry(g, maxDepth - curDepth, &alpha, &beta);

	if (alpha >= beta) {
		ret = alpha;
		assert(mm == NULL);
		goto out;
	}

	best = minScore;
	ng = copyGame(g);

	nsucc = genSuccs_wrap(g, &succs, curDepth);

	for (i=0; i<nsucc; i++) {
		sort_succ(g, succs, i, nsucc, curDepth);

		if (!doMove_unchecked(ng, succs[i].m))
			continue;

		stats.nopen++;
		nvalid++;

		mark(ng);
		t = -negamax(ng, maxDepth, curDepth+1, NULL, -beta, -alpha);
		unmark(ng);

		/* Ya no necesitamos a ng */
		*ng = *g;

		if (t > best) {
			best = t;
			bestmove = succs[i].m;
			if (mm != NULL) {
				*mm = succs[i].m;
				assert(mm->move_type >= 0);
			}
		}

		if (t > alpha) {
			alpha = t;

			if (beta <= alpha) {
				addon_notify_cut(g, succs[i].m, curDepth);
				break;
			}
		}

	}

	freeSuccs(succs, nsucc);
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
