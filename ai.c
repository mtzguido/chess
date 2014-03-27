#include "ai.h"
#include "board.h"

#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define SEARCH_DEPTH	4

#define EXTRA_CHECK	1
#define EXTRA_CAPTURE	5
#define EXTRA_PROMOTION	99

#ifdef CFG_SHUFFLE
 const int flag_shuffle = 1;
#else
 const int flag_shuffle = 0;
#endif

#ifdef CFG_RANDOMIZE
 const int flag_randomize = 1;
#else
 const int flag_randomize = 0;
#endif

#ifdef CFG_ALPHABETA
 const int alpha_beta = 1;
#else
 const int alpha_beta = 0;
#endif

typedef int score;

int machineColor = -1;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);

// static score heur(game g);

static const score minScore = -1e7;
static const score maxScore =  1e7;

static void sortSuccs(game g, game *succs, int n, int depth);

int depths[30];
static int nopen;
int totalnopen = 0;
int totalms = 0;

static int equalMove(move a, move b) __attribute__((unused));
static int equalGame(game a, game b) __attribute__((unused));

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

	for (i = 0; i < 30; i++)
		depths[i] = 0;

	nopen = 0;
	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

	assert(ret != NULL);

	totalnopen += nopen;
	totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	fprintf(stderr, "%i nodes in %.3f seconds\n", nopen, 1.0*(t2-t1)/CLOCKS_PER_SEC);
	fprintf(stderr, "depth:nnodes - ");
	for (i = 0; i < 30; i++) 
		fprintf(stderr, "%i:%i, ", i, depths[i]);

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

#ifdef CFG_DEPTH_EXTENSION
	const int extraDepth = 
		  EXTRA_CHECK * inCheck(g, WHITE)
		+ EXTRA_CHECK * inCheck(g, BLACK)
		+ EXTRA_CAPTURE * g->lastmove.was_capture
		+ EXTRA_PROMOTION * g->lastmove.was_promotion;
#else
	const int extraDepth = 0;
#endif

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
		else if (rc == DRAW)
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
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);

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
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);

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
		;

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

static int equalMove(move a, move b) {
	if (a.move_type != b.move_type) return 0;
	if (a.who != b.who) return 0;

	if (a.move_type != MOVE_REGULAR)
		return 1;

	return a.r == b.r
		&& a.c == b.c
		&& a.R == b.R
		&& a.C == b.C
		&& a.promote == b.promote;
}

static int equalGame(game a, game b) {
	if (a == NULL && b == NULL)
		return 1;

	if (a == NULL || b == NULL)
		return 0;

	if (a->turn != b->turn
	 || a->idlecount != b->idlecount
	 || a->en_passant_x != b->en_passant_x
	 || a->en_passant_y != b->en_passant_y
	 || a->kingx[0] != b->kingx[0]
	 || a->kingx[1] != b->kingx[1]
	 || a->pieceScore != b->pieceScore
	 || a->totalScore != b->totalScore
	 || a->pps_O != b->pps_O
	 || a->pps_E != b->pps_E
	 || a->castle_king[0] != b->castle_king[0]
	 || a->castle_king[1] != b->castle_king[1]
	)
		return 0;

	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			if (a->board[i][j] != b->board[i][j])
				return 0;

	return 1;
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

