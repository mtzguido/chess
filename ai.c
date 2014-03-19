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

static void sortSuccs(game g, game *succs, int n, int depth);

static int nopen;
int totalnopen = 0;

static move killerTable[KTABLE_SIZE][NKILLER];

static move counterTable[2][8][8][8][8];

static void counterNotify(game g, game next);
static void killerNotify(game g, game next, int depth);

game machineMove(game start) {
	game ret = NULL;
	score t;
	clock_t t1,t2;
	int i, j;

	for (i=0; i<KTABLE_SIZE; i++)
		for (j=0; j<NKILLER; j++)
			killerTable[i][j].move_type = -1;

	/* ugh... */
	int a, b, c, d, e;
	for (a=0; a<2; a++)
	for (b=0; b<8; b++)
	for (c=0; c<8; c++)
	for (d=0; d<8; d++)
	for (e=0; e<8; e++)
		counterTable[a][b][c][d][e].move_type = -1;

	nopen = 0;
	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

	fprintf(stderr, "%i \t nodes in \t %.3fs\n", nopen, (t2-t1)*1.0/CLOCKS_PER_SEC);
	totalnopen += nopen;
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

	nopen++;

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

	/* Generamos los sucesores del tablero */
	n = genSuccs(g, &succs);

	/* No debería ocurrir nunca */
	if (n == 0) {
		printBoard(g);
		fprintf(stderr, "--NO MOVES!!! ------\n");
		abort();
	}

	/* Ordenamos los sucesores */
	sortSuccs(g, succs, n, curDepth);

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

			/* Alpha cut-off */
			if (beta <= alpha) {
				killerNotify(g, succs[i], curDepth);
				counterNotify(g, succs[i]);

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

			/* Beta cut-off */
			if (beta <= alpha) {
				killerNotify(g, succs[i], curDepth);
				counterNotify(g, succs[i]);

				break;
			}
		}

		freeSuccs(succs, n);
		return beta;
	}
}

static int pieceScore(game g) {
	int x = g->totalScore - 40000;
	int pps = (x*(g->pps_O - g->pps_E))/7800 + g->pps_E;


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

static void killerMoves(game *succs, int n, int depth);
static void counterMoves(game *succs, int n, game g);

static void sortSuccs(game g, game *succs, int n, int depth)
{
	qsort(succs, n, sizeof (game), succCmp);

	counterMoves(succs, n, g);
	killerMoves(succs, n, depth);
}

static void killerMoves(game *succs, int n, int depth) {
	int kindex = 0;
	int i, k;

	/* Ordenamos las killer move al
	 * principio
	 *
	 * usamos también las killers de 2
	 * plies atrás. */

	for (i=0; i<n; i++) {
		for (k=0; k<NKILLER; k++) {
			if (equalMove(succs[i]->lastmove, killerTable[depth][k])) {
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
}

static void counterMoves(game *succs, int n, game g) {
	int i;

	/* Buscamos la counter move */
	move l = g->lastmove;
	move m = counterTable[g->turn][l.r][l.c][l.R][l.C];

	if (m.move_type == -1)
		return;

	for (i=0; i<n; i++) {
		if (equalMove(succs[i]->lastmove, m)) {
			game temp;

			if (i > 0) {
				temp = succs[i];
				succs[i] = succs[0];
				succs[0] = temp;
			}

			break;
		}
	}
}

static void killerNotify(game g, game next, int depth) {
	int i;

	for (i=0; i<NKILLER; i++)
		if (equalMove(killerTable[depth][i], next->lastmove))
			return;

	for (i=NKILLER-1; i >= 1; i--)
		killerTable[depth][i] = killerTable[depth][i-1];

	killerTable[depth][0] = next->lastmove;
}

static void counterNotify(game g, game next) {
	if (!next->lastmove.was_capture) {
		move m = g->lastmove;
		counterTable[g->turn][m.r][m.c][m.R][m.C] = next->lastmove;
	}
}

