#include "ai.h"
#include "game.h"
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
	float won;
	float heur;
	float tiebreak;
} score;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);
static score heur(game g);
static int scoreCmp(score a, score b);

score minScore = { -INFINITY, 0, 0 };
score maxScore = {  INFINITY, 0, 0 };

game machineMove(game start) {
	game ret;
	score sc;

	printf("Turno: %s\n", start.turn == BLACK ? "Black" : "White");

	sc = machineMoveImpl(start, 3, 0, &ret, minScore, maxScore);
	fprintf(stderr, "machineMove returned board with expected score (%f,%f,%f)\n", sc.won, sc.heur, sc.tiebreak);

	return ret;
}

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

//	printf("%.*smachineMoveImpl\n", curDepth, "                             ");

	if (curDepth >= maxDepth) {
		if (nb != NULL)
			*nb = g;

		return heur(g);
	}

	game *succs;
	score ret, t;
	int i, n;
	int fst = 1;

	n = genSuccs(g, &succs);

	if (n == 0) {
		printBoard(g);
		printf("--------------------\n");
		assert("NO MOVES!\n" == NULL);
	}

	if (g.turn == machineColor) {
		/* maximizing */
		for (i=0; i<n; i++) {
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);
			if (fst || scoreCmp(t, ret) > 0) {
				fst = 0;
				ret = t;
				if (nb != NULL)
					*nb = succs[i];
			}
		}
	} else {
		/* minimizing */
		for (i=0; i<n; i++) {
			t = machineMoveImpl(succs[i], maxDepth, curDepth+1, NULL, alpha, beta);
			if (fst || scoreCmp(t, ret) < 0) {
				fst = 0;
				ret = t;
				if (nb != NULL)
					*nb = succs[i];
			}
		}
	}

	free(succs);
	return ret;
}

static score heur(game g) {
	float res = 0;
	int i, j;
	score ret = {0,0,0};

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g.board[i][j] == 0)
				continue;

			if (colorOf(g.board[i][j]) == machineColor) {
				if (isPawn(g.board[i][j])) {
					res += 1;
				} else if (isRook(g.board[i][j])) {
					res += 10;
				} else if (isKnight(g.board[i][j])) {
					res += 10;
				} else if (isBishop(g.board[i][j])) {
					res += 10;
				} else if (isQueen(g.board[i][j])) {
					res += 50;
				} else if (isKing(g.board[i][j])) {
					res += 10000;
				}
			} else {
				if (isPawn(g.board[i][j])) {
					res -= 1;
				} else if (isRook(g.board[i][j])) {
					res -= 10;
				} else if (isKnight(g.board[i][j])) {
					res -= 10;
				} else if (isBishop(g.board[i][j])) {
					res -= 10;
				} else if (isQueen(g.board[i][j])) {
					res -= 50;
				} else if (isKing(g.board[i][j])) {
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
	ret.tiebreak = rand(); /* !!!!!!!!!!!!!!!!!!!!!!! */

	return ret;
}

static int scoreCmp(score a, score b) {
	if (a.won > b.won) return  1;
	if (a.won < b.won) return -1;
	if (a.heur > b.heur) return  1;
	if (a.heur < b.heur) return -1;
	if (a.tiebreak > b.tiebreak) return  1;
	if (a.tiebreak < b.tiebreak) return -1;
	return 0;
}

