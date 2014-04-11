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

/* Color to play */
int machineColor = -1;

static void shuffleSuccs(game g, move *succs, int n);
static void sortSuccs(game g, move *succs, int n, int depth);
static score machineMoveImpl(game start, int maxDepth, int curDepth,
			     game *nb, score alpha, score beta, int color);

typedef int (*succgen_t)(game, move**, int);

static int genSuccs_wrap(game g, move **arr, int depth) {
	return genSuccs(g, arr);
}

/* Stats */
int depths[30];
static int nopen;
static int ngen;
int totalnopen = 0;
int totalms = 0;

game machineMove(game start) {
	game ret = NULL;
	score t;
	clock_t t1,t2;

	addon_reset();

	int i;

	n_collision = 0;
	nopen = 0;
	ngen = 0;
	for (i = 0; i < 30; i++)
		depths[i] = 0;

	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore, machineColor);
	t2 = clock();

	assert(ret != NULL);

	totalnopen += nopen;
	totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	fprintf(stderr, "stats: searched %i nodes in %.3f seconds\n", nopen, 1.0*(t2-t1)/CLOCKS_PER_SEC);
	fprintf(stderr, "stats: total nodes generated: %i\n", ngen);
	fprintf(stderr, "stats: depth:n_nodes - ");
	for (i = 0; depths[i] != 0; i++) 
		fprintf(stderr, "%i:%i, ", i, depths[i]);

	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);

	fprintf(stderr, "expected score: %i\n", t);
	fflush(NULL);
	return ret;
}

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta, int color, bool doguiBreak);

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta, int color) {

	score rc1 = machineMoveImpl_(g, maxDepth, curDepth, nb, alpha, beta, color, true);
	score rc2 = machineMoveImpl_(g, maxDepth, curDepth, nb, alpha, beta, color, false);
	assert(rc1 == rc2);

	/* Wrap de machineMoveImpl, para debug */
	/* printf("mm (%i/%i) (a=%i, b=%i) returns %i\n", curDepth, maxDepth, alpha, beta, rc); */

	return rc1;
}

const succgen_t gen_funs[] = {
	addon_suggest,
	genSuccs_wrap,
};

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta, int color, bool doguiBreak) {

	score ret = minScore;

	if (nb == NULL && addon_notify_entry(g, curDepth, &ret)) {
		assert(0);
		return ret;
	}

	bool cut = false;
	game ng = galloc();
	move *succs = NULL;
	score t;
	int i, n = 0;
	int nvalid = 0;
	move best;
	best.move_type = -1;

	const int extraDepth = 
		  EXTRA_CHECK * inCheck(g, WHITE)
		+ EXTRA_CHECK * inCheck(g, BLACK)
		+ EXTRA_CAPTURE * g->lastmove.was_capture
		+ EXTRA_PROMOTION * g->lastmove.was_promotion;

	nopen++;
	depths[curDepth]++;

	/* Si el tablero es terminal */
	if (isDraw(g)) {
		ret = 0;
		goto out;
	}

	if (beta <= alpha) {
		ret = alpha;
//		assert(0);
		goto out;
	}

	/* Si llegamos a la profundidad deseada */
	if (curDepth >= maxDepth + extraDepth) {
		assert(nb == NULL);

		if (color == WHITE)
			ret = heur(g);
		else
			ret = - heur(g);

		goto out;
	}


	unsigned ii;
	for (ii = 0; ii < sizeof gen_funs / sizeof gen_funs[0]; ii++) {
		/*
		 * Generamos los sucesores del tablero
		 * según la función generadora actual
		 */

		n = gen_funs[ii](g, &succs, curDepth);
		assert(succs != NULL);

		if (n == 0) {
			freeSuccs(succs, n);
			continue;
		}

		assert(n > 0);
		ngen += n;

		/* Ordenamos los sucesores */
		sortSuccs(g, succs, n, curDepth);

		/* Itero por los sucesores, maximizando */
		for (i=0; i<n; i++) {
			*ng = *g;

			/*
			 * Admitimos que genSuccs nos dé
			 * movidas inválidas y simplemente
			 * las ignoramos.
			 */
			assert(succs[i].who == g->turn);
			if (! doMove(ng, succs[i]))
				continue;

			nvalid++;

			mark(ng);
			t = - machineMoveImpl(ng, maxDepth, curDepth+1, NULL,
					-beta, -alpha, flipTurn(color));
			unmark(ng);

			if (t > alpha) {
				alpha = t;
				best = succs[i];

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(ng);
				}
			}

			if (alpha_beta && beta <= alpha) {
				cut = true;
				addon_notify_cut(g, succs[i], curDepth);
				break;
			}
		}

		freeSuccs(succs, n);

		/*
		 * Si ocurrió un corte, sabemos que *nada*
		 * puede cambiar el resultado. Por lo tanto
		 * no seguimos con las nuevas generadoras
		 */
		assert(cut == (beta <= alpha));
		if (doguiBreak && alpha_beta && beta <= alpha) {
			printf("doguicortando.. %i %i %i %i\n", curDepth, alpha, beta, 0);
			assert(cut);
			assert(nvalid > 0);
			break;
		}
	}

	ret = alpha;

	/*
	 * Si no pudimos encontrar un sucesor
	 * estamos en un tablero terminal.
	 */
	if (nvalid == 0) {
		assert(best.move_type == -1);

		if (inCheck(g, color))
			ret = -10000 + curDepth;
		else
			ret = 0;

		goto out;
	}
	
	/*
	 * Si hubo una movida que superó el alpha,
	 * la notificamos.
	 */
	if (best.move_type != -1)
		addon_notify_return(g, best, ret, curDepth);

out:

	freeGame(ng);
	return ret;
}

static int pieceScore(game g) {
	int x = g->totalScore - 40000;
	int pps = (x*(g->pps_O - g->pps_E))/8000 + g->pps_E;

	return g->pieceScore + pps;
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

static void sortSuccs(game g, move *succs, int n, int depth) {
	score *vals = malloc(n * sizeof (score));
	int i, j;
	score ts;
	move tm;

	/* Shuffle */
	shuffleSuccs(g, succs, n);

	for (i=0; i<n; i++)
		vals[i] = 0;

	addon_score_succs(g, succs, vals, n, depth);

	for (i=1; i<n; i++) {
		for (j=i; j>0; j--) {
			if (vals[j-1] < vals[j]) {
				ts = vals[j-1];
				vals[j-1] = vals[j];
				vals[j] = ts;

				tm = succs[j-1];
				succs[j-1] = succs[j];
				succs[j] = tm;
			} else break;
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
