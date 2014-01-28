#include "ai.h"
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>

typedef struct {
	float won;
	float heur;
	float tiebreak;
} score;

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta);

score minScore = { -INFINITY, 0, 0 };
score maxScore = {  INFINITY, 0, 0 };

game machineMove(game start) {
	game ret;
	score sc;

	sc = machineMoveImpl(start, 3, 0, &ret, minScore, maxScore);
	fprintf(stderr, "machineMove returned board with expected score (%f,%f,%f)\n", sc.won, sc.heur, sc.tiebreak);

	return ret;
}

static score machineMoveImpl(
		game start, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	int i, j;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (start.board[i][j] == 0 || colorOf(start.board[i][j] != start.turn))
				continue;
				
			/* hay una pieza y es del color correcto,
			 * analizamos los posibles movimientos */
			if (isPawn(start.board[i][j])) {
			} else if (isRook(start.board[i][j])) {
			} else if (isKnight(start.board[i][j])) {
			} else if (isBishop(start.board[i][j])) {
			} else if (isQueen(start.board[i][j])) {
			} else if (isKing(start.board[i][j])) {
			} else {
				assert("WHAT?!" == NULL);
			}
		}
	}

	*nb = start;

	return minScore;
}


