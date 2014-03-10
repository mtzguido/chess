#include "ai.h"
#include "board.h"

#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define SEARCH_DEPTH	7
#define KTABLE_SIZE	(SEARCH_DEPTH * 2)
#define NKILLER	2

typedef float score;

int machineColor = -1;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);

static score heur(game g, int depth);

static const score minScore = -INFINITY;
static const score maxScore =  INFINITY;

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
	fprintf(stderr, "expected score: %f\n", t);
//	printBoard(ret);
	fflush(NULL);
	return ret;
}

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	/* Si el tablero es terminal */
	if (curDepth == maxDepth || isFinished(g) != -1) {
		if (nb != NULL)
			*nb = copyGame(g);

		return heur(g, curDepth);
	}

	game *succs;
	score t;
	int i, n;
	int kindex;
	score best;

	/* Generamos los sucesores del tablero */
	nopen++;
	n = genSuccs(g, &succs);

	/* No debería ocurrir nunca */
	if (n == 0) {
		printBoard(g);
		printf("--------------------\n");
		assert("NO MOVES!\n" == NULL);
	}

	/*
	 * Pongo las killer move primero
	 *
	 * usamos también las killers de 2
	 * plies atrás.
	 */
	int k;

	sortSuccs(succs, n);

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
	best = minScore;
	for (i=0; i<n; i++) {
		t = - machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, -beta, -alpha);

		if (t > best) {
			best = t;

			if (nb != NULL) {
				if (*nb != NULL)
					freeGame(*nb);

				*nb = copyGame(succs[i]);
			}
		}

		if (t > alpha)
			alpha = t;

		/* Corte, alpha o beta */
#if 1
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
#endif
	}

	freeSuccs(succs, n);
	return best;
}

static float pieceScore(game g) {
	return g->pieceScore * (g->turn == WHITE ? 1 : -1);
}

static float coverScore(game g) {
	return 0;
}

static score heur(game g, int depth) {
	score ret = 0;

	int t = isFinished(g);

	if (t != -1) {
		if (t == WIN(g->turn))
			ret += 100000;
		else if (t == WIN(flipTurn(g->turn)))
			ret += -100000;
		else
			ret += 0;
	} else {
		ret += 0;
	}

	/* Si estaba terminada, no nos importa esto */
	if (ret == 0) {
		ret += (pieceScore(g))
			 + (coverScore(g))
			 + (inCheck(g, flipTurn(g->turn)) ?  2 : 0)
			 + (inCheck(g,          g->turn ) ? -2 : 0)
			 ;
	}

	/* Con esto, ante heuristicas iguales,
	 * preferimos movidas cercanas */
	if (g->turn == machineColor)
		ret -= depth * 0.01;
	else
		ret += depth * 0.01;
	
	return ret;
}

static int succCmp(const void *bp, const void *ap) {
	game a = *((game*)ap);
	game b = *((game*)bp);

	if (a->lastmove.was_capture != b->lastmove.was_capture)
		return a->lastmove.was_capture - b->lastmove.was_capture;

	if (a->lastmove.was_promotion != b->lastmove.was_promotion)
		return a->lastmove.was_promotion - b->lastmove.was_promotion;

	const int ar = a->lastmove.r;
	const int ac = a->lastmove.c;
	const int br = b->lastmove.r;
	const int bc = b->lastmove.c;

	return abs(a->board[ar][ac]) - abs(b->board[br][bc]);
}

static void sortSuccs(game *succs, int n)
{
	qsort(succs, n, sizeof (game), succCmp);
}
