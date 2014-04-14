#include "ai.h"
#include "board.h"
#include "config.h"
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
static void sortSuccs(game g, move *succs, int n, int depth, int maxDepth);
static score machineMoveImpl(game start, int maxDepth, int curDepth,
			     move *mm, score alpha, score beta);

typedef int (*succgen_t)(game, move**, int);

static int genSuccs_wrap(game g, move **arr, int depth) {
	return genSuccs(g, arr);
}

/* Stats */
struct stats stats;

void reset_stats() {
	n_collision = 0;
	stats.nopen = 0;
	stats.ngen = 0;
	stats.nbranch = 0;
	for (i = 0; i < 30; i++)
		stats.depthsn[i] = 0;
}

void print_stats() {
	fprintf(stderr, "stats: searched %i nodes in %.3f seconds\n", stats.nopen, 1.0*(t2-t1)/CLOCKS_PER_SEC);
	fprintf(stderr, "stats: branching aprox: %.3f\n", 1.0 * stats.nbranch / stats.nopen);
	fprintf(stderr, "stats: total nodes generated: %i\n", stats.ngen);
	fprintf(stderr, "stats: depth:n_nodes - ");
	fprintf(stderr, "expected score: %i (i am %i)\n", t, start->turn);
	for (i = 0; stats.depthsn[i] != 0; i++) 
		fprintf(stderr, "%i:%i, ", i, stats.depthsn[i]);

	fprintf(stderr, "\n");

}

move machineMove(game start) {
	move ret;
	score t;
	clock_t t1,t2;

	addon_reset();

	int i;

	reset_stats();

	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

	stats.totalopen += stats.nopen;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_stats();

	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);
	fflush(NULL);
	return ret;
}

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta);

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta) {
	score rc1 = machineMoveImpl_(g, maxDepth, curDepth, mm, alpha, beta);

	/* Wrap de machineMoveImpl, para debug */
	/* printf("mm (%i/%i) (a=%i, b=%i) returns %i\n", curDepth, maxDepth, alpha, beta, rc); */

	return rc1;
}

const succgen_t gen_funs[] = {
#ifdef CFG_SUGG
	addon_suggest,
#endif
	genSuccs_wrap,
};

#define ARRSIZE(a) ((sizeof (a))/(sizeof ((a)[0])))

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta) {

	score t, ret, best;
	move *succs;
	int i, nsucc;
	int nvalid = 0;
	int COPIED = 0;
	game ng;

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
			ret = heur(g);
		else
			ret = - heur(g);

		goto out;
	}

	stats.nopen++;

	unsigned ii;
	best = minScore;
	for (ii = 0; ii < ARRSIZE(gen_funs); ii++) {

		assert(ii<2);
		nsucc = gen_funs[ii](g, &succs, curDepth);

		if (nsucc == 0)
			goto loop;

		assert(nsucc > 0);
		assert(succs != NULL);
		stats.ngen += nsucc;

		sortSuccs(g, succs, nsucc, curDepth, maxDepth);

		ng = galloc();
		for (i=0; i<nsucc; i++) {
			*ng = *g;

			if (!doMove(ng, succs[i]))
				continue;

			nvalid++;

			mark(ng);
			t = -machineMoveImpl(ng, maxDepth, curDepth+1,
					     NULL, -beta, -alpha);
			unmark(ng);

			if (t > best) {
				best = t;
				if (mm != NULL) {
					*mm = succs[i];
					COPIED = 1;
				}
			}

			if (t > alpha)
				alpha = t;

			if (alpha_beta && beta <= alpha) {
				addon_notify_cut(g, succs[i], curDepth);
				break;
			}
		}
		freeGame(ng);

loop:	
		freeSuccs(succs, nsucc);
	}

	stats.nbranch += nvalid;

	if (nvalid == 0) {
		if (inCheck(g, g->turn))
			ret = -100000 + curDepth;
		else
			ret = 0; /* Stalemate */
	} else {
		ret = alpha;
	}

out:

	if (mm != NULL)
		assert(COPIED);

	return ret;
}

static int pieceScore(game g) {
	int x = g->totalScore - 40000;
	int pps = (x*(g->pps_O - g->pps_E))/8000 + g->pps_E;

	return g->pieceScore  + pps;
}

score heur(game g) {
	score ret = 0;

	ret = (pieceScore(g))
	    + (inCheck(g, WHITE) ? -30 : 0)
	    + (inCheck(g, BLACK) ?  30 : 0);

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
	if (g->idlecount >= 34)
		ret = (ret * (50 - g->idlecount))/16;

	return ret;
}

/*
static int succCmp(const void *bp, const void *ap) {
	game a = *((game*)ap);
	game b = *((game*)bp);

	if (a->lastmove.was_capture != b->lastmove.was_capture)
		return a->lastmove.was_capture - b->lastmove.was_capture;

	if (a->lastmove.was_promotion != b->lastmove.was_promotion)
		return a->lastmove.was_promotion - b->lastmove.was_promotion;

	return 0;
}
	qsort(succs, n, sizeof (game), succCmp);
*/

static void sortSuccs(game g, move *succs, int n, int depth, int maxDepth) {
	score *vals;
	int i, j;
	score ts;
	move tm;

	if (depth+2 >= maxDepth)
		return;
	/* Shuffle */
	shuffleSuccs(g, succs, n);

	vals = malloc(n * sizeof (score));
	for (i=0; i<n; i++)
		vals[i] = 0;

	addon_score_succs(g, succs, vals, n, depth);

	for (i=1; i<n; i++) {
		for (j=i; j>0; j--) {
			if (vals[j-1] >= vals[j])
				break;

			ts = vals[j-1];
			vals[j-1] = vals[j];
			vals[j] = ts;

			tm = succs[j-1];
			succs[j-1] = succs[j];
			succs[j] = tm;
		}
	}

	free(vals);
}

static void shuffleSuccs(game g, move *succs, int n) {
	if (flag_shuffle) {
		int i, j;
		move t;

		for (i=0; i<n-1; i++) {
			j = i + rand() % (n-i);

			t = succs[i];
			succs[i] = succs[j];
			succs[j] = t;
		}
	}
}
