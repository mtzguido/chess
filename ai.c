#include "ai.h"
#include "board.h"

#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
	float won;
	float heur;
	float tiebreak;
} score;

int machineColor = -1;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);
static score heur(game g);
static int scoreCmp(score a, score b);

score minScore = { -INFINITY, 0, 0 };
score maxScore = {  INFINITY, 0, 0 };

static int nopen;

game machineMove(game start) {
	game ret = NULL;
	score sc;
	clock_t t1,t2;

	nopen = 0;
	t1 = clock();
	sc = machineMoveImpl(start, 6, 0, &ret, minScore, maxScore);
	t2 = clock();

	printf("machineMove returned board with expected score (%f,%f,%f)\n", sc.won, sc.heur, sc.tiebreak);
	printf("time: %.3fs\n", (t2-t1)*1.0/CLOCKS_PER_SEC);
	printBoard(ret);
	printf("(nopen = %i)\n", nopen);
	fflush(NULL);

	printf("move=%i %i %i %i\n", ret->lastmove.r, ret->lastmove.c, ret->lastmove.R, ret->lastmove.C);
	assert(isLegalMove(start, ret->lastmove));

	return ret;
}

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	/* si hay jaque mate buscamos el mejor o
	 * lo agarramos y listo? */

	if (curDepth >= maxDepth || isFinished(g) != -1) {
//		printf("LEAF BOARD!!\n");
//		printBoard(g);
//		printf("HEUR= %f %f %f\n", heur(g).won, heur(g).heur, heur(g).tiebreak);
//		getchar();

		if (nb != NULL)
			*nb = copyGame(g);

//		printf("%.*s(leaf) = %f,%f,%f\n", curDepth, "            ", heur(g).won, heur(g).heur, heur(g).tiebreak);
		return heur(g);
	}

	game *succs;
	score t;
	score ret;
	int i, n;

	n = genSuccs(g, &succs);

	nopen++;

	if (n == 0) {
		printBoard(g);
		printf("--------------------\n");
		assert("NO MOVES!\n" == NULL);
	}

	if (g->turn == machineColor) {
		/* Maximizing */
		for (i=0; i<n; i++) {
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);
			if (scoreCmp(t, alpha) > 0) {
				alpha = t;

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(succs[i]);
				}

				if (scoreCmp(beta, alpha) <= 0)
					break;
			}
		}

		ret = alpha;
		goto out;
	} else {
		/* Minimizing */
		for (i=0; i<n; i++) {
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);
			if (scoreCmp(t, beta) < 0) {
				beta = t;

				if (nb != NULL) {
					if (*nb != NULL)
						freeGame(*nb);

					*nb = copyGame(succs[i]);
				}

				if (scoreCmp(beta, alpha) <= 0)
					break;
			}
		}

		ret = beta;
		goto out;
	}

out:
	freeSuccs(succs, n);
	return ret;
}

static score heur(game g) {
	float res = 0;
	int i, j;
	score ret = {0,0,0};

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			char piece = g->board[i][j];

			if (piece == 0)
				continue;

			if (colorOf(piece) == machineColor) {
				if (isPawn(piece)) {
					res += 1;
				} else if (isRook(piece)) {
					res += 10;
				} else if (isKnight(piece)) {
					res += 10;
				} else if (isBishop(piece)) {
					res += 10;
				} else if (isQueen(piece)) {
					res += 50;
				} else if (isKing(piece)) {
					res += 10000;
				}
			} else {
				if (isPawn(piece)) {
					res -= 1;
				} else if (isRook(piece)) {
					res -= 10;
				} else if (isKnight(piece)) {
					res -= 10;
				} else if (isBishop(piece)) {
					res -= 10;
				} else if (isQueen(piece)) {
					res -= 50;
				} else if (isKing(piece)) {
					res -= 10000;
				}
			}
		}
	}

	int t = isFinished(g);

	if (t != -1) {
		if (t == machineColor)
			ret.won = 1;
		else
			ret.won = -1;

	} else {
		ret.won = 0;
	}

	ret.heur = res;
	ret.tiebreak = 0*rand(); /* !!!!!!!!!!!!!!!!!!!!!!! */

	return ret;
}

static int scoreCmp_(score a, score b) {
	if (a.won      > b.won)      return  1;
	if (a.won      < b.won)      return -1;
	if (a.heur     > b.heur)     return  1;
	if (a.heur     < b.heur)     return -1;
	if (a.tiebreak > b.tiebreak) return  1;
	if (a.tiebreak < b.tiebreak) return -1;
	return 0;
}

static int scoreCmp(score a, score b) {
	int t = scoreCmp_(a,b);

//	printf("cmp (%f %f %f) (%f %f %f) = %i\n", a.won, a.heur, a.tiebreak, b.won, b.heur, b.tiebreak, t);

	return t;
}
