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

#ifdef CFG_DEPTH_EXTENSION
#define KTABLE_SIZE	(SEARCH_DEPTH + EXTRA_CHECK + EXTRA_CAPTURE + EXTRA_PROMOTION)
#else
#define KTABLE_SIZE	(SEARCH_DEPTH)
#endif

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

int depths[30];
static int nopen;
int totalnopen = 0;
int totalms = 0;

static int equalMove(move a, move b);
static int equalGame(game a, game b);

#ifdef CFG_KILLER
static move killerTable[KTABLE_SIZE][NKILLER];
static void killerNotify(game g, game next, int depth);
#endif

#ifdef CFG_COUNTERMOVE
static move counterTable[2][8][8][8][8];
static void counterNotify(game g, game next);
#endif

#ifdef CFG_TRANSPOSITION
#define TTABLE_SIZE 131101

struct tt_entry {
	score v;
	game k;
	int d;
};

struct tt_entry ttable[TTABLE_SIZE] = { [0 ... TTABLE_SIZE-1] = { .k = NULL } };

static int gameHash(game g);
#endif

game machineMove(game start) {
	game ret = NULL;
	score t;
	clock_t t1,t2;

#ifdef CFG_KILLER
	{
	int i, j;
	for (i=0; i<KTABLE_SIZE; i++)
		for (j=0; j<NKILLER; j++)
			killerTable[i][j].move_type = -1;
	}
#endif

#ifdef CFG_COUNTERMOVE
	{
	/* ugh... */
	int a, b, c, d, e;
	for (a=0; a<2; a++)
	for (b=0; b<8; b++)
	for (c=0; c<8; c++)
	for (d=0; d<8; d++)
	for (e=0; e<8; e++)
		counterTable[a][b][c][d][e].move_type = -1;
	}
#endif

#ifdef CFG_TRANSPOSITION
	{
	int i;
	
	for (i=0; i<TTABLE_SIZE; i++)
		if (ttable[i].k != NULL)
			freeGame(ttable[i].k);

		ttable[i].k = NULL;
	}
#endif

	int i;

	for (i = 0; i < 30; i++)
		depths[i] = 0;
	nopen = 0;
	t1 = clock();
	t = machineMoveImpl(start, SEARCH_DEPTH, 0, &ret, minScore, maxScore);
	t2 = clock();

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

static score machineMoveImpl(
		game g, int maxDepth, int curDepth,
		game *nb, score alpha, score beta) {

	score ret;

#ifdef CFG_TRANSPOSITION
	int gh = gameHash(g);

	if (ttable[gh].k != NULL
		&& ttable[gh].d == curDepth
		//&& ttable[gh].d <= curDepth
		&& equalGame(g, ttable[gh].k)) {
		//fprintf(stderr, "trans: skipping node at %i\n", curDepth);

		return ttable[gh].v;
	}

#endif
	
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
	int i, n;

//	fprintf(stderr, "at prof %i: ", curDepth); pr_board(g);

	/* Si el tablero es terminal */
	int rc;
	if ((rc=isFinished(g)) != -1) {
		if (rc == WIN(machineColor))
			ret = 1000000 - curDepth;
		else if (rc == DRAW)
			ret = 0;
		else
			ret = -1000000 + curDepth;

		goto out;
	}

	/* Si llegamos a la profundidad deseada */
	if (curDepth >= maxDepth + extraDepth) {
		if (nb != NULL)
			*nb = copyGame(g);

		ret = (machineColor == WHITE ?
		           heur(g)
			   : - heur(g));
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
#ifdef CFG_KILLER
				killerNotify(g, succs[i], curDepth);
#endif
#ifdef CFG_COUNTERMOVE
				counterNotify(g, succs[i]);
#endif

				break;
			}
		}

		ret = alpha;
		goto out;
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
#ifdef CFG_KILLER
				killerNotify(g, succs[i], curDepth);
#endif
#ifdef CFG_COUNTERMOVE
				counterNotify(g, succs[i]);
#endif

				break;
			}
		}

		ret = beta;
		goto out;
	}

out:

#ifdef CFG_TRANSPOSITION
	if (ttable[gh].k != NULL) {
		//fprintf(stderr, "trans: collision at %i\n", curDepth);
		freeGame(ttable[gh].k);
	}

	ttable[gh].k = copyGame(g);
	ttable[gh].v = ret;
	ttable[gh].d = curDepth;

#endif

	if (succs != NULL)
		freeSuccs(succs, n);

	return ret;
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

#ifdef CFG_KILLER

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

static void killerNotify(game g, game next, int depth) {
	int i;

	for (i=0; i<NKILLER; i++)
		if (equalMove(killerTable[depth][i], next->lastmove))
			return;

	for (i=NKILLER-1; i >= 1; i--)
		killerTable[depth][i] = killerTable[depth][i-1];

	killerTable[depth][0] = next->lastmove;
}

#endif

#ifdef CFG_COUNTERMOVE

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

static void counterNotify(game g, game next) {
	if (!next->lastmove.was_capture) {
		move m = g->lastmove;
		counterTable[g->turn][m.r][m.c][m.R][m.C] = next->lastmove;
	}
}

#endif

static void sortSuccs(game g, game *succs, int n, int depth) {
	qsort(succs, n, sizeof (game), succCmp);

#ifdef CFG_COUNTERMOVE
	counterMoves(succs, n, g);
#endif
#ifdef CFG_KILLER
	killerMoves(succs, n, depth);
#endif
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

#ifdef CFG_TRANSPOSITION
static int gameHash(game g) {
	uint64_t x = 5381;
	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j+=2) {
			unsigned char c = ((g->board[i][j] & 0xF) ^ ((g->board[i][j+1] << 4) & 0xF0));
			x = c + (x*33);
		}

			//x = ((6+g->board[i][j]) + (x*9)) % TTABLE_SIZE;

	x = x % TTABLE_SIZE;

	//fprintf(stderr, "gameHash returns %u\n", x);
	return x;
}
#endif

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

