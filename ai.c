#include "ai.h"
#include "board.h"
#include "config.h"
#include "ztable.h"
#include "mem.h"
#include "addon.h"

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

static void sortSuccs(game g, move *succs, int n, int depth);
static score machineMoveImpl(game start, int maxDepth, int curDepth,
			     game *nb, score alpha, score beta, int color);

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
		game *nb, score alpha, score beta, int color);

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta, int color) {

	score rc = machineMoveImpl_(g, maxDepth, curDepth, nb, alpha, beta, color);

	/* Wrap de machineMoveImpl, para debug */
	/* printf("mm (%i/%i) (a=%i, b=%i) returns %i\n", curDepth, maxDepth, alpha, beta, rc); */

	return rc;
}

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta, int color) {

	score ret;
	const score lalpha __attribute__((unused)) = alpha;
	const score lbeta __attribute__((unused)) = beta;
	int best = -1;

	if (nb == NULL && addon_notify_entry(g, curDepth, &ret))
		return ret;

	const int extraDepth = 
		  EXTRA_CHECK * inCheck(g, WHITE)
		+ EXTRA_CHECK * inCheck(g, BLACK)
		+ EXTRA_CAPTURE * g->lastmove.was_capture
		+ EXTRA_PROMOTION * g->lastmove.was_promotion;

	nopen++;
	depths[curDepth]++;

	game ng = galloc();
	move *succs = NULL;
	score t;
	int i, n = 0;

	/* Si el tablero es terminal */
	int rc;
	if ((rc=isFinished(g)) != -1) {
		if (rc == WIN(color))
			ret = 100000 - curDepth;
		else if (rc == DRAW_STALE || rc == DRAW_50MOVE || rc == DRAW_3FOLD)
			ret = 0;
		else
			ret = -100000 + curDepth;

		goto out;
	}

	/* Si llegamos a la profundidad deseada */
	if (curDepth >= maxDepth + extraDepth) {
		if (nb != NULL)
			*nb = copyGame(g);

		if (color == WHITE)
			ret = heur(g);
		else
			ret = - heur(g);

		goto out;
	}

	/*
	 * Antes de generar los sucesores usamos las
	 * sugerencias de las heurísticas. Tal vez
	 * tengamos un corte temprano.
	 */
#if 0
	{
		/* DESHABILITADO POR AHORA */
		/* TODO: Unir todo! */
		goto asd;

		if (nb != NULL)
			goto asd;

		move arr[10];
		int n;
		game succ = galloc();

		n = addon_suggest(g, arr, curDepth);

		/* Itero por los sucesores, maximizando */
		for (i=0; i<n; i++) {
			*succ = *g;
			doMove(succ, arr[i]);

			mark(succ);
			t = - machineMoveImpl(succ, maxDepth, curDepth+1, NULL,
					-beta, -alpha, flipTurn(color));
			unmark(succ);

			if (t > alpha) {
				alpha = t;
				best = i;
			}

			if (alpha_beta && beta <= alpha) {
				addon_notify_cut(g, arr[i], curDepth);
				ret = alpha;
				addon_notify_return(g, arr[i], ret, curDepth);
				freeGame(succ);
				goto out;
			}
		}

		freeGame(succ);
	}

asd:
#endif

	/* Generamos los sucesores del tablero */
	n = genSuccs(g, &succs);
	assert(succs != NULL);
	assert(n > 0);
	ngen += n;

	/* Shuffle */
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

	/* Ordenamos los sucesores */
	sortSuccs(g, succs, n, curDepth);

	/* Itero por los sucesores, maximizando */
	for (i=0; i<n; i++) {
		*ng = *g;

		/*
		 * Admitimos que genSuccs nos dé
		 * movidas inválidas.
		 */
		if (! doMove(ng, succs[i]))
			continue;


		mark(ng);
		t = - machineMoveImpl(ng, maxDepth, curDepth+1, NULL,
				      -beta, -alpha, flipTurn(color));
		unmark(ng);

		if (t > alpha) {
			alpha = t;
			best = i;

			if (nb != NULL) {
				if (*nb != NULL)
					freeGame(*nb);

				*nb = copyGame(ng);
			}
		}

		if (alpha_beta && beta <= alpha) {
			addon_notify_cut(g, succs[i], curDepth);
			break;
		}
	}

	ret = alpha;
	assert((best != -1) == (i < n));
	if (i < n)
		addon_notify_return(g, succs[best], ret, curDepth);

out:

	if (succs != NULL)
		freeSuccs(succs, n);

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

	for (i=0; i<n; i++)
		vals[i] = 0;

	addon_score_succs(g, succs, vals, n, depth);

	for (i=1; i<n; i++) {
		for (j=i; j>0; j--) {
			if (vals[j-1] < vals[j]) {
				score ts;
				move tm;

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
