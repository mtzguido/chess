#include "ai.h"
#include "board.h"
#include "config.h"
#include "ztable.h"

#include <stdint.h>
#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Score definition */
typedef int score;
static const score minScore = -1e7;
static const score maxScore =  1e7;

/* Color to play */
int machineColor = -1;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);

static void sortSuccs(game g, game *succs, int n, int depth);

/* Stats */
int depths[30];
static int nopen;
int totalnopen = 0;
int totalms = 0;

/* Addon functions */
static void addon_init();
static bool addon_notify_entry(game g, int depth, score *ret);
static void addon_notify_return(game g, score s, int depth);
static void addon_notify_cut(game g, game next, int depth);
static void addon_sort(game g, game *succs, int nsucc, int depth);

static int roll0(int n) {
	return (rand()%n) == 0;
}

/*
 * Addons (heuristicas) en archivos
 * separados. Se usan .h para poder
 * aprovechar optimizaciones del 
 * compilador en mayor medida
 */
#include "addon_trans.h"
#include "addon_killer.h"
#include "addon_cm.h"

game machineMove(game start) {
	game ret = NULL;
	score t;
	clock_t t1,t2;

	addon_init();

	int i;

	n_collision = 0;
	nopen = 0;
	for (i = 0; i < 30; i++)
		depths[i] = 0;

	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

	assert(ret != NULL);

	totalnopen += nopen;
	totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	fprintf(stderr, "%i nodes in %.3f seconds\n", nopen, 1.0*(t2-t1)/CLOCKS_PER_SEC);
	fprintf(stderr, "depth:nnodes - ");
	for (i = 0; i < 15; i++) 
		fprintf(stderr, "%i:%i, ", i, depths[i]);

	fprintf(stderr, "Number of hash collisions: %i\n", n_collision);

	fprintf(stderr, "expected score: %i\n", t);
	fflush(NULL);
	return ret;
}

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	score rc = machineMoveImpl_(g, maxDepth, curDepth, nb, alpha, beta);

	/* Wrap de machineMoveImpl, para debug */
	/* printf("mm (%i/%i) (a=%i, b=%i) returns %i\n", curDepth, maxDepth, alpha, beta, rc); */

	return rc;
}

static score machineMoveImpl_(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	score ret;
	const score lalpha __attribute__((unused)) = alpha;
	const score lbeta __attribute__((unused)) = beta;

	score maxa = alpha;
	score maxb = beta;

	int rep_count = 1;

	if (addon_notify_entry(g, curDepth, &ret))
		return ret;

	const int extraDepth = 
		  EXTRA_CHECK * inCheck(g, WHITE)
		+ EXTRA_CHECK * inCheck(g, BLACK)
		+ EXTRA_CAPTURE * g->lastmove.was_capture
		+ EXTRA_PROMOTION * g->lastmove.was_promotion;

	nopen++;
	depths[curDepth]++;

	game *succs = NULL;
	score t;
	int i, n = 0;

//	fprintf(stderr, "at prof %i: ", curDepth); pr_board(g);

	/* Si el tablero es terminal */
	int rc;
	if ((rc=isFinished(g)) != -1) {
		if (rc == WIN(machineColor))
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

		if (machineColor == WHITE)
			ret = heur(g);
		else
			ret = - heur(g);

		goto out;
	}

	/* Generamos los sucesores del tablero */
	n = genSuccs(g, &succs);
	assert(succs != NULL);

	/* No debería ocurrir nunca */
	if (n == 0) {
		printBoard(g);
		fprintf(stderr, "--NO MOVES!!! ------\n");
		fflush(NULL);
		abort();
	}

	/* Shuffle */
	if (flag_shuffle) {
		int i, j;
		game t;

		for (i=0; i<n-1; i++) {
			j = i + rand() % (n-i);

			t = succs[i];
			succs[i] = succs[j];
			succs[j] = t;
		}
	}

	/* Ordenamos los sucesores */
	sortSuccs(g, succs, n, curDepth);

	/* Itero por los sucesores */
	if (g->turn == machineColor) {
		/* Maximizar */
		for (i=0; i<n; i++) {
			mark(succs[i]);
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);
			unmark(succs[i]);

			if (t > maxa)
				maxa = t;

			if (t > alpha || (flag_randomize && t == alpha && roll0(rep_count+1))) {
				if (t > alpha)
					rep_count = 1;
				else
					rep_count++;

				alpha = t;

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(succs[i]);
				}
			}

			if (alpha_beta && beta <= alpha) {
				addon_notify_cut(g, succs[i], curDepth);
				break;
			}
		}

		ret = alpha;
		assert(ret == maxa);
		goto out;
	} else {
		/* Minimizar */
		for (i=0; i<n; i++) {
			mark(succs[i]);
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);
			unmark(succs[i]);

			if (t < maxb)
				maxb = t;

			if (t < beta || (flag_randomize && t == beta && roll0(rep_count+1))) {
				if (t < beta)
					rep_count = 1;
				else
					rep_count++;

				beta = t;

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(succs[i]);
				}
			}

			if (alpha_beta && beta <= alpha) {
				addon_notify_cut(g, succs[i], curDepth);
				break;
			}
		}

		ret = beta;
		assert(ret == maxb);
		goto out;
	}

out:

	addon_notify_return(g, ret, curDepth);

	if (succs != NULL)
		freeSuccs(succs, n);

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
	    + (inCheck(g, WHITE) ? -200 : 0)
	    + (inCheck(g, BLACK) ?  200 : 0);

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

	return ret;
}

static int succCmp(const void *bp, const void *ap) {
	game a = *((game*)ap);
	game b = *((game*)bp);

	if (a->lastmove.was_capture != b->lastmove.was_capture)
		return a->lastmove.was_capture - b->lastmove.was_capture;

	if (a->lastmove.was_promotion != b->lastmove.was_promotion)
		return a->lastmove.was_promotion - b->lastmove.was_promotion;

	return 0;
}

static void sortSuccs(game g, game *succs, int n, int depth) {
	qsort(succs, n, sizeof (game), succCmp);

	addon_sort(g, succs, n, depth);
}

static void addon_init() {
	killer_init();
	cm_init();
	trans_init();
}

static void addon_notify_return(game g, score s, int depth) {
	trans_notify_return(g, s, depth);
}

static bool addon_notify_entry(game g, int depth, score *ret) {
	if (trans_notify_entry(g, depth, ret))
		return true;

	return false;
}

static void addon_notify_cut(game g, game next, int depth) {
	killer_notify_cut(g, next, depth);
	cm_notify_cut(g, next, depth);
}

static void addon_sort(game g, game *succs, int nsucc, int depth) {
	cm_sort(g, succs, nsucc, depth);
	killer_sort(g, succs, nsucc, depth);
}

