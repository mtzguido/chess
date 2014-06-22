#include "ai.h"
#include "board.h"
#include "common.h"
#include "ztable.h"
#include "mem.h"
#include "addon.h"
#include "succs.h"

#include <stdint.h>
#include <math.h> /* INFINITY */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Score definition */
static const score minScore = -1e7;
static const score maxScore =  1e7;


struct stats stats;

static score negamax(game start, int maxDepth, int curDepth,
			     move *mm, score alpha, score beta);

static int genCaps_wrap(game g, struct MS **arr, int depth) {
	int n;

	/* Generar sucesores */
	n = genCaps(g, arr);
	stats.ngen += n;

	addon_score_succs(g, *arr, n, depth);

	return n;
}

static int genSuccs_wrap(game g, struct MS **arr, int depth) {
	int n;

	/* Generar sucesores */
	n = genSuccs(g, arr);
	stats.ngen += n;

	addon_score_succs(g, *arr, n, depth);

	return n;
}

static void sort_succ(game g, struct MS *arr, int i, int len, int depth_rem) {
	/* Mezclarlos si es necesario */
	if (copts.shuffle) {
		int t = rand()%(len-i) + i;
		struct MS swap;
		swap = arr[i];
		arr[i] = arr[t];
		arr[t] = swap;
	}

	/* Ordenarlos si es necesario */
	if (depth_rem > 2) {
		int j;
		int best = i;
		score s = arr[i].s;

		for (j=i+1; j<len; j++) {
			if (arr[j].s > s) {
				best = j;
				s = arr[j].s;
			}
		}

		struct MS swap;
		swap = arr[best];
		arr[best] = arr[i];
		arr[i] = swap;
	}
}

void reset_stats() {
	int i;

	n_collision = 0;
	stats.nopen_s = 0;
	stats.nopen_q = 0;
	stats.ngen = 0;
	stats.nbranch = 0;
	for (i = 0; i < 30; i++)
		stats.depthsn[i] = 0;
}

void print_stats(score exp, clock_t t1, clock_t t2) {
	int i;

	fprintf(stderr, "stats: searched %lld (%lld) nodes in %.3f seconds\n", stats.nopen_s, stats.nopen_q, 1.0*(t2-t1)/CLOCKS_PER_SEC);
	fprintf(stderr, "stats: branching aprox: %.3f\n", 1.0 * stats.nbranch / stats.nopen_s);
	fprintf(stderr, "stats: total nodes generated: %lld\n", stats.ngen);
	fprintf(stderr, "stats: depth:n_nodes - ");
	fprintf(stderr, "expected score: %i\n", exp);
	for (i = 0; stats.depthsn[i] != 0; i++) 
		fprintf(stderr, "%i:%i, ", i, stats.depthsn[i]);

	fprintf(stderr, "\n");

}

move machineMove(game start) {
	move ret = {0};
	score t = 0;
	clock_t t1,t2;
	int i;

	addon_reset();
	reset_stats();

	t1 = clock();
	for (i=1; i<copts.depth; i++)
		negamax(start, i, 0, NULL, minScore, maxScore);

	t = negamax(start, copts.depth, 0, &ret, minScore, maxScore);
	assert(ret.move_type >= 0);
	t2 = clock();

	stats.totalopen += stats.nopen_s + stats.nopen_q;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_stats(t, t1, t2);

	fprintf(stderr, "stats: Number of hash collisions: %i\n", n_collision);
	fflush(NULL);
	return ret;
}

static score quiesce(game g, score alpha, score beta, int curDepth, int maxDepth) {
	score t;

	int nsucc, i;
	int nvalid;
	game ng;
	struct MS *succs;
	score ret;

	if (isDraw(g))
		return 0;

	stats.nopen_q++;

	t = boardEval(g);

	if (t >= beta)
		return beta;

	if (t > alpha)
		alpha = t;

	if (curDepth >= maxDepth && !inCheck(g, g->turn))
		return t;

	ng = copyGame(g);
	nsucc = genCaps_wrap(g, &succs, curDepth);
	nvalid = 0;
	for (i=0; i<nsucc; i++) {
		sort_succ(g, succs, i, nsucc, maxDepth - curDepth);

		if (succs[i].m.move_type != MOVE_REGULAR)
			continue;

		/* We only consider captures and promotions */
		assert(enemy_piece(g, succs[i].m.R, succs[i].m.C)
				|| succs[i].m.promote
				|| (succs[i].m.R == g->en_passant_x
					&& succs[i].m.C == g->en_passant_y));

		if (!doMove_unchecked(ng, succs[i].m))
			continue;

		nvalid++;

		mark(ng);
		t = -quiesce(ng, -beta, -alpha, curDepth+1, maxDepth);
		unmark(ng);
		*ng = *g;

		if (t > alpha) {
			if (t >= beta) {
				ret = beta;
				goto out;
			}

			alpha = t;
		}
	}

	if (nvalid == 0)
		ret = t;
	else
		ret = alpha;

out:
	freeSuccs(succs, nsucc);
	freeGame(ng);

	return ret;
}

static score negamax(
		game g, int maxDepth, int curDepth,
		move *mm, score alpha, score beta) {

	score t, ret, best;
	struct MS *succs = NULL;
	int i, nsucc;
	int nvalid = 0;
	game ng;
	int bestmove = -1;
	const score alpha_orig = alpha;

	if (isDraw(g)) {
		ret = 0;
		assert(mm == NULL);
		goto out;
	}

	if (inCheck(g, g->turn))
		maxDepth++;

	if (curDepth >= maxDepth) {
		ret = quiesce(g, alpha, beta, curDepth, curDepth+4);
		goto out;
	}

	stats.nopen_s++;

	if (mm == NULL)
		addon_notify_entry(g, maxDepth - curDepth, &alpha, &beta);

	if (alpha >= beta) {
		ret = alpha;
		assert(mm == NULL);
		goto out;
	}

	best = minScore;
	ng = copyGame(g);

	nsucc = genSuccs_wrap(g, &succs, curDepth);

	for (i=0; i<nsucc; i++) {
		sort_succ(g, succs, i, nsucc, maxDepth - curDepth);

		if (!doMove_unchecked(ng, succs[i].m))
			continue;

		nvalid++;

		mark(ng);
		t = -negamax(ng, maxDepth, curDepth+1, NULL, -beta, -alpha);
		unmark(ng);

		/* Ya no necesitamos a ng */
		*ng = *g;

		if (t > best) {
			best = t;
			bestmove = i;
		}

		if (t > alpha)
			alpha = t;

		if (alpha >= beta) {
			ret = best;
			addon_notify_cut(g, succs[i].m, curDepth);
			break;
		}
	}

	if (bestmove != -1 && mm != NULL)
		*mm = succs[bestmove].m;

	freeSuccs(succs, nsucc);
	freeGame(ng);

	stats.nbranch += nvalid;

	flag_t flag;

	if (nvalid == 0) {
		if (inCheck(g, g->turn))
			ret = -100000 + curDepth;
		else
			ret = 0; /* Stalemate */

		flag = FLAG_EXACT;
	} else {
		ret = best;

		if (best <= alpha_orig)
			flag = FLAG_UPPER_BOUND;
		else if (best > beta)
			flag = FLAG_LOWER_BOUND;
		else
			flag = FLAG_EXACT;
	}

	if (maxDepth - curDepth > 1 && bestmove != -1)
		addon_notify_return(g, succs[bestmove].m,
				    maxDepth - curDepth, ret, flag);

out:
	return ret;
}

static int pieceScore(game g) {
	int x = g->totalScore - 40000;
	int pps = (x*(g->pps_O - g->pps_E))/8000 + g->pps_E;

	return g->pieceScore + pps;
}


static score eval_wpawn(int i, int j, int pawn_rank[2][10]) {
	score ret = 0;

	if (pawn_rank[WHITE][j+1] > i)
		ret += DOUBLED_PAWN;

	if (pawn_rank[WHITE][j] == 0
			&& pawn_rank[WHITE][j+2] == 0)
		ret += ISOLATED_PAWN;

	if (pawn_rank[WHITE][j] < i &&
			pawn_rank[WHITE][j+2] < i)
		ret += BACKWARDS_PAWN;

	if (pawn_rank[BLACK][j] >= i &&
			pawn_rank[BLACK][j+1] >= i &&
			pawn_rank[BLACK][j+2] >= i)
		ret += (7 - i) * PASSED_PAWN;

	return ret;
}

static score eval_bpawn(int i, int j, int pawn_rank[2][10]) {
	score ret = 0;

	if (pawn_rank[BLACK][j+1] < i)
		ret += DOUBLED_PAWN;

	if (pawn_rank[BLACK][j] == 0
			&& pawn_rank[BLACK][j+2] == 0)
		ret += ISOLATED_PAWN;

	if (pawn_rank[BLACK][j] > i &&
			pawn_rank[BLACK][j+2] > i)
		ret += BACKWARDS_PAWN;

	if (pawn_rank[WHITE][j] <= i &&
			pawn_rank[WHITE][j+1] <= i &&
			pawn_rank[WHITE][j+2] <= i)
		ret += i * PASSED_PAWN;

	return ret;
}

score boardEval(game g) {
	int i, j;
	score score = pieceScore(g);
	int pawn_rank[2][10];

	for (i=0; i<10; i++) {
		pawn_rank[WHITE][i] = 0;
		pawn_rank[BLACK][i] = 7;
	}

	/*
	 * Primera pasada, llenamos pawn_rank con el peon
	 * menos avanzado de cada lado.
	 */
	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			piece_t piece = g->board[i][j];

			if (piece == WPAWN) {
				if (pawn_rank[WHITE][j+1] < i)
					pawn_rank[WHITE][j+1] = i;
			} else if (piece == BPAWN) {
				if (pawn_rank[BLACK][j+1] > i)
					pawn_rank[BLACK][j+1] = i;
			}
		}
	}

	/*
	 * Segunda pasada. Con la información de los peones
	 * evaluamos filas abiertas y status de peones
	 */
	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			piece_t piece = g->board[i][j];

			if (piece == WPAWN) {
				score += eval_wpawn(i, j, pawn_rank);
			} else if (piece == BPAWN) {
				score -= eval_bpawn(i, j, pawn_rank);
			} else if (piece == WROOK) {
				if (pawn_rank[WHITE][j+1] == 0) {
					if (pawn_rank[BLACK][j+1] == 7)
						score += ROOK_OPEN_FILE;
					else
						score += ROOK_SEMI_OPEN_FILE;
				}
			} else if (piece == BROOK) {
				if (pawn_rank[BLACK][j+1] == 7) {
					if (pawn_rank[WHITE][j+1] == 0)
						score -= ROOK_OPEN_FILE;
					else
						score -= ROOK_SEMI_OPEN_FILE;
				}
			}
		}
	}

	if (!g->castled[WHITE]) {
		if (!g->castle_king[WHITE] && !g->castle_queen[WHITE])
			score -= 15;
		else if (!g->castle_king[WHITE])
			score -= 12;
		else if (!g->castle_queen[WHITE])
			score -= 8;
	}

	if (!g->castled[BLACK]) {
		if (!g->castle_king[BLACK] && !g->castle_queen[BLACK])
			score += 15;
		else if (!g->castle_king[BLACK])
			score += 12;
		else if (!g->castle_queen[BLACK])
			score += 8;
	}

	/*
	 * Acercamos a 0 los tableros que tengan
	 * muchos movimientos idle
	 */
	if (g->idlecount >= 68)
		score = (score * (100 - g->idlecount))/32;

	return g->turn == WHITE ? score : -score;
}
