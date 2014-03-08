#include "ai.h"
#include "board.h"

#include <assert.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define SEARCH_DEPTH	6
#define NKILLER	2

typedef struct {
	float won;
	float heur;
	float tiebreak;
} score;

int machineColor = -1;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);
static score heur(game g, int depth);
static int scoreCmp(score a, score b);

score minScore = { -INFINITY, 0, 0 };
score maxScore = {  INFINITY, 0, 0 };

static int nopen;

static move killerTable[SEARCH_DEPTH][NKILLER];

game machineMove(game start) {
	game ret = NULL;
	score sc;
	clock_t t1,t2;
	int i, j;

	for (i=0; i<SEARCH_DEPTH; i++)
		for (j=0; j<NKILLER; j++)
			killerTable[i][j].move_type = -1;

	nopen = 0;
	t1 = clock();
	sc = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

	printf("machineMove returned board with expected score (%f,%f,%f)\n", sc.won, sc.heur, sc.tiebreak);
	printf("move=(type=%i) %i %i %i %i\n", ret->lastmove.move_type, ret->lastmove.r, ret->lastmove.c, ret->lastmove.R, ret->lastmove.C);
	printf("time: %.3fs\n", (t2-t1)*1.0/CLOCKS_PER_SEC);
	printf("(nopen = %i)\n", nopen);
	fflush(NULL);

	assert(isLegalMove(start, ret->lastmove));

	return ret;
}

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	/* Si nos pasamos de profundidad o el tablero
	 * es terminal */
	if (curDepth >= maxDepth || isFinished(g) != -1) {
		if (nb != NULL)
			*nb = copyGame(g);

		return heur(g, curDepth);
	}

	bool maximizing;
	game *succs;
	score t;
	int i, n;
	int kindex;

	if (g->turn == machineColor)
		maximizing = true;
	else
		maximizing = false;

	/* Generamos los sucesores del tablero */
	n = genSuccs(g, &succs);
	nopen++;

	/* No debería ocurrir nunca */
	if (n == 0) {
		printBoard(g);
		printf("--------------------\n");
		assert("NO MOVES!\n" == NULL);
	}

	/* Pongo las killer move primero */
	int k;

	kindex = 0;
	for (i=0; i<n; i++) {
		for (k=0; k<NKILLER; k++) {
			if (equalMove(succs[i]->lastmove, killerTable[curDepth][k])) {
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
	for (i=0; i< n; i++) {
		t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);

		if (maximizing && scoreCmp(t, alpha) <= 0)
			continue;
		else if (!maximizing && scoreCmp(t, beta) >= 0)
			continue;

		if (maximizing)
			alpha = t;
		else
			beta = t;

		if (nb != NULL) {
			if (*nb != NULL)
				freeGame(*nb);

			*nb = copyGame(succs[i]);
		}

		/* Corte, alpha o beta */
		if (scoreCmp(beta, alpha) <= 0) {
/*			printf("Cut-off! %i %i %i %i %i\n", curDepth,
					succs[i]->lastmove.r,
					succs[i]->lastmove.c,
					succs[i]->lastmove.R,
					succs[i]->lastmove.C);
					*/

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
	return maximizing ? alpha : beta;
}

static float pieceScore(game g) {
	int i, j;
	float res = 0;

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
				}
			}
		}
	}
	
	return res;
}

static float coverScore(game g) {
	return 0;
}

static score heur(game g, int depth) {
	score ret = {0,0,0};

	int t = isFinished(g);

	if (t != -1) {
		if (t == WIN(machineColor))
			ret.won = 1;
		else if (t == WIN(flipTurn(machineColor)))
			ret.won = 0;
		else
			ret.won = 0.5;
	} else {
		ret.won = 0.5;
	}

	ret.heur = (pieceScore(g))
	         + (coverScore(g))
			 + (inCheck(g, flipTurn(machineColor)) ? 20 : 0);

	ret.tiebreak = -depth;

	return ret;
}

static int scoreCmp_(score a, score b) {
	if (a.won      > b.won     ) return  1;
	if (a.won      < b.won     ) return -1;
	if (a.heur     > b.heur    ) return  1;
	if (a.heur     < b.heur    ) return -1;
	if (a.tiebreak > b.tiebreak) return  1;
	if (a.tiebreak < b.tiebreak) return -1;
	return 0;
}

static int scoreCmp(score a, score b) {
	int t = scoreCmp_(a,b);

//	printf("cmp (%f %f %f) (%f %f %f) = %i\n", a.won, a.heur, a.tiebreak, b.won, b.heur, b.tiebreak, t);

	return t;
}
