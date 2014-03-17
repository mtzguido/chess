#include "ai.h"
#include "board.h"
#include "piece-square.h"

#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define SEARCH_DEPTH	6
#define KTABLE_SIZE	(SEARCH_DEPTH + 7)
#define NKILLER	2

typedef int score;

int machineColor = -1;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);

static score heur(game g);

static const score minScore = -1e7;
static const score maxScore =  1e7;

static void sortSuccs(game *succs, int n);

static int nopen;

static move killerTable[KTABLE_SIZE][NKILLER];

game machineMove(game start) {
	game ret = NULL;
	score t;
	clock_t t1,t2;
	int i, j;

	for (i=0; i<KTABLE_SIZE; i++)
		for (j=0; j<NKILLER; j++)
			killerTable[i][j].move_type = -1;

	nopen = 0;
	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

	fprintf(stderr, "%i \t nodes in \t %.3fs\n", nopen, (t2-t1)*1.0/CLOCKS_PER_SEC);
	fprintf(stderr, "expected score: %i\n", t);
//	printBoard(ret);
	fflush(NULL);
	return ret;
}

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {
	
	int extraDepth = 
		  1 * inCheck(g, WHITE)
		+ 1 * inCheck(g, BLACK)
		+ 2 * g->lastmove.was_capture
		+ 3 * g->lastmove.was_promotion;

	/* Si el tablero es terminal */
	int rc;
	if ((rc=isFinished(g)) != -1) {
		if (rc == WIN(machineColor))
			return 1000000 - curDepth;
		else if (rc == DRAW)
			return 0;
		else
			return -1000000 + curDepth;
	}

	/* Si llegamos a la profundidad deseada */
	if (curDepth >= maxDepth + extraDepth) {
		if (nb != NULL)
			*nb = copyGame(g);

		return (machineColor == WHITE ?
		            heur(g)
			    : - heur(g));
	}

	game *succs;
	score t;
	int i, n;
	int kindex;

	/* Generamos los sucesores del tablero */
	nopen++;
	n = genSuccs(g, &succs);

	/* No debería ocurrir nunca */
	if (n == 0) {
		printBoard(g);
		fprintf(stderr, "--NO MOVES!!! ------\n");
		abort();
	}

	int k;

	/*
	 * Ordenamos rudimentariamente los
	 * sucesores. Lo hacemos antes de
	 * ordenar las killer move para
	 * darle prioridad a las killer
	 */
	sortSuccs(succs, n);

	/*
	 * Pongo las killer move primero
	 *
	 * usamos también las killers de 2
	 * plies atrás.
	 */
	kindex = 0;
	for (i=0; i<n; i++) {
		for (k=0; k<NKILLER; k++) {
			if (equalMove(succs[i]->lastmove, killerTable[curDepth][k])
				|| (curDepth >= 2 && equalMove(succs[i]->lastmove, killerTable[curDepth-2][k]))
				) {
				game temp;

				if (i > kindex) {
					temp = succs[i];
					succs[i] = succs[kindex];
					succs[kindex] = temp;
				}
				kindex++;

				break;
			}
		}
	}

	/* Itero por los sucesores */
	if (g->turn == machineColor) {
		/* Maximizar */
		for (i=0; i<n; i++) {
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);

			if (t > alpha) {
				alpha = t;

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(succs[i]);
				}
			}

			/* Corte, alpha o beta */
			if (beta <= alpha) {
				/* Si no era una killer move, la agrego */
				if (i >= kindex) {
					int k;
					for (k=NKILLER-1; k>0; k--)
						killerTable[curDepth][k] = killerTable[curDepth][k-1];

					killerTable[curDepth][0] = succs[i]->lastmove;
				}

				break;
			}
		}

		freeSuccs(succs, n);
		return alpha;
	} else {
		/* Minimizar */
		for (i=0; i<n; i++) {
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);

			if (t < beta) {
				beta = t;

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(succs[i]);
				}
			}

			/* Corte, alpha o beta */
			if (beta <= alpha) {
				/* Si no era una killer move, la agrego */
				if (i >= kindex) {
					int k;
					for (k=NKILLER-1; k>0; k--)
						killerTable[curDepth][k] = killerTable[curDepth][k-1];

					killerTable[curDepth][0] = succs[i]->lastmove;
				}

				break;
			}
		}

		freeSuccs(succs, n);
		return beta;
	}
}

int pieceScore(game g) {
	int x = g->totalScore - 40000;
	int pps = (x*g->pps_O + (7800-x)*g->pps_E)/7800;

	return g->pieceScore + pps;
}

static score heur(game g) {
	score ret = 0;

	ret = (pieceScore(g))
		+ (inCheck(g, BLACK) ?  200 : 0)
		+ (inCheck(g, WHITE) ? -200 : 0)
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

static void sortSuccs(game *succs, int n)
{
	qsort(succs, n, sizeof (game), succCmp);
}

