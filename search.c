#include "search.h"
#include "board.h"
#include "addon.h"
#include "ztable.h"
#include "common.h"
#include "ai.h"
#include "eval.h"
#include "moves.h"
#include "check.h"
#include <stdbool.h>

move pv[MAX_PLY][MAX_PLY];
int pv_len[MAX_PLY];

static score negamax(int curDepth, int maxDepth, move *mm, score alpha, score beta);
static score _negamax(int curDepth, int maxDepth, move *mm, score alpha, score beta);

static inline void shuffle_succs() {
	struct MS swap;
	const int lo = first_succ[ply];
	const int hi = first_succ[ply+1];
	int i, j;

	if (!copts.shuffle)
		return;

	for (i = lo; i < hi-1; i++) {
		j = i + rand() % (hi-i);

		if (i == j)
			continue;

		swap = gsuccs[j];
		gsuccs[j] = gsuccs[i];
		gsuccs[i] = swap;
	}
}

static inline void genSuccs_wrap(int depth) {
	/* Generar sucesores */
	genSuccs();
	stats.ngen += first_succ[ply+1] - first_succ[ply];

	addon_score_succs(depth);

	/* Mezclarlos si es necesario */
	shuffle_succs();
}

static inline void genCaps_wrap(int depth) {
	genCaps();
	stats.ngen += first_succ[ply+1] - first_succ[ply];

	addon_score_succs(depth);
}


/*
 * Ordena el arreglo de sucesores de manera lazy.
 * Deja en arr[i] el sucesor correcto, asume que arr[0..i-1] ya
 * está ordenado.
 */
static inline void sort_succ(int i) {
	int j;
	int best;

	assert(gsuccs[i].m.who == G->turn);
	assert(i >= first_succ[ply]);
	assert(i < first_succ[ply+1]);

	score s = gsuccs[i].s;
	best = i;

	for (j=i+1; j<first_succ[ply+1]; j++) {
		assert(gsuccs[j].m.who == G->turn);
		if (gsuccs[j].s > s) {
			best = j;
			s = gsuccs[j].s;
		}
	}

	if (best != i) {
		struct MS swap;

		swap = gsuccs[best];
		gsuccs[best] = gsuccs[i];
		gsuccs[i] = swap;
	}

	assert(gsuccs[i].m.who == G->turn);
	assert(gsuccs[i].m.move_type != MOVE_INVAL);
	assert(gsuccs[i].s >= 0);
}

static inline int calcExtension(int maxDepth, int curDepth) {
	int ret = 0;

	if (inCheck(G->turn) || hstack[hply - 1].was_promote)
		ret++;

	return ret;
}

static inline score null_move_score(int curDepth, int maxDepth, score alpha,
				    score beta)
{
	static bool doing_null_move = false;
	score t;
	move m = { .move_type = MOVE_NULL, .who = G->turn };
	__unused bool check;

	if (!copts.null)
		goto dont;

	/* Dont do two null moves in the same variation */
	if (doing_null_move)
		goto dont;

	/*
	 * Dont null-move when in check or when low in material since
	 * we're likely to be in Zugzwang
	 */
	if (inCheck(G->turn) || G->pieceScore[G->turn] <= NMH_THRESHOLD)
		goto dont;

	if (maxDepth - curDepth <= NMH_REDUCTION)
		goto dont;

	stats.null_tries++;

	check = doMove(m);
	/*
	 * doMoveNull's only restriction is not being in check and we already
	 * provided a case for that so this should never fail
	 */
	assert(check);

	first_succ[ply] = first_succ[ply - 1];
	doing_null_move = true;

	t = -negamax(maxDepth - NMH_REDUCTION, curDepth+1, NULL, -beta, -alpha);

	doing_null_move = false;

	undoMove();
	return t;

dont:
	return alpha;
}

static inline score _quiesce(score alpha, score beta, int curDepth);

static score quiesce(int curDepth, score alpha, score beta) {
	__unused game bak = G;
	__unused u64 h = G->zobrist;
	score ret;

	ret = _quiesce(curDepth, alpha, beta);

	assert(G == bak);
	assert(G->zobrist == h);
	return ret;
}

static inline score _quiesce(score alpha, score beta, int curDepth) {
	int nvalid, i;
	score ret, ev;
	score t;

	assert(ply == curDepth);

	if (timeup) {
		ret = 0;
		goto out;
	} else if (timelimited) {
		ticks++;

		if ((ticks & 0xfff) == 0) {
			if (getms() >= timelimit) {
				timeup = true;
				ret = 0;
				goto out;
			}
		}
	}

	if (isDraw() || reps() >= 2) {
		ret = 0;
		goto out;
	}

	stats.nopen_q++;

	if (copts.lazy) {
		score bound = fullBound;
		ev = 0;

		for (i = 0; i < nEval; i++) {
			ev += evalFuns[i]();
			bound -= evalBound[i];

			if (ev - bound >= beta) {
				ret = beta;
				goto out;
			}
		}
		assert(bound == 0);
	} else {
		ev = boardEval();
		if (ev >= beta) {
			ret = beta;
			goto out;
		}
	}

	if (copts.delta_prune) {
		score delta = QUEEN_SCORE - PAWN_SCORE;

		if (hstack[hply - 1].was_promote)
			delta += QUEEN_SCORE;

		if (ev + delta < alpha) {
			ret = alpha;
			goto out;
		}
	}

	if (ev > alpha)
		alpha = ev;

	if (ply >= MAX_PLY-1) {
		ret = ev;
		goto out;
	}

	genCaps_wrap(curDepth);
	nvalid = 0;
	for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
		sort_succ(i);
		const move m = gsuccs[i].m;

		assert(m.move_type == MOVE_REGULAR);

		/* We only consider captures and promotions */
		assert(isCapture(m) || isPromotion(m));

		if (!doMove_unchecked(m))
			continue;

		nvalid++;

		t = -quiesce(-beta, -alpha, curDepth+1);

		undoMove();

		if (t > alpha) {
			if (t >= beta) {
				ret = beta;
				goto out;
			}

			alpha = t;
		}
	}

	if (nvalid == 0)
		ret = ev;
	else
		ret = alpha;

out:

	assert(timeup || ret > minScore);
	assert(timeup || ret < maxScore);

	return ret;
}

static
score negamax(int maxDepth, int curDepth, move *mm, score alpha, score beta) {
	__unused game bak = G;
	__unused u64 h = G->zobrist;
	score ret;

	ret = _negamax(maxDepth, curDepth, mm, alpha, beta);

	assert(G == bak);
	assert(G->zobrist == h);
	return ret;
}

static
score _negamax(int maxDepth, int curDepth, move *mm, score alpha, score beta) {
	score t, ret, best, alpha_orig;
	int i;
	int ext;
	int nvalid = 0;
	int bestmove = -1;

	stats.nall++;

	pv_len[ply] = 0;

	assert(ply == curDepth);

	if (timeup) {
		ret = 0;
		goto out;
	} else if (timelimited) {
		ticks++;

		if ((ticks & 0xfff) == 0) {
			if (getms() >= timelimit) {
				timeup = true;
				ret = 0;
				goto out;
			}
		}
	}

	/*
	 * The absolute best we can ever do is CHECKMATE_SCORE - curDepth -
	 * 1, so use it as a bound. This causes the search to run a lot more
	 * quickly in the final endgame.
	 */
	if (alpha >= CHECKMATE_SCORE - curDepth - 1) {
		ret = alpha;
		goto out;
	}

	if (isDraw()) {
		ret = 0;
		goto out;
	}

	/*
	 * Si ya pasamos por este tablero, podemos asumir
	 * que vamos a reaccionar igual y vamos a llevar a un empate
	 * por repetición.
	 */
	if (reps() >= 2 && !mm) {
		ret = 0;
		goto out;
	}

	ext = calcExtension(maxDepth, curDepth);
	maxDepth += ext;

	/*
	 * Corte por profundidad, hacemos búsqueda por quietud, para
	 * mejorar nuestra evaluación de tablero.
	 */
	if (curDepth >= maxDepth) {
		assert(!mm);
		assert(curDepth == maxDepth);

		/*
		 * Si esto ocurre, tenemos una recursion mutua
		 * infinita con quiesce. No debería ocurrir nunca,
		 * pero dejamos el assert por las dudas.
		 */
		assert(!inCheck(G->turn));

		if (copts.quiesce)
			ret = quiesce(alpha, beta, curDepth);
		else
			ret = boardEval();

		goto out;
	}

	/*
	 * Only try to null-move if beta was less than maxScore.
	 * Otherwise we will never suceed in the test.
	 */
	if (!mm && beta < maxScore) {
		t = null_move_score(curDepth, maxDepth, alpha, beta);
		if (t >= beta) {
			stats.null_cuts++;
			ret = beta;
			goto out;
		}
	}

	if (!mm) {
		addon_notify_entry(maxDepth - curDepth, &alpha, &beta);

		if (alpha >= beta && copts.ab) {
			/* Revisit this */
			ret = alpha;
			goto out;
		}
	}

	if (ply >= MAX_PLY-1) {
		ret = boardEval();
		goto out;
	}

	alpha_orig = alpha;
	best = minScore;

	stats.nopen_s++;
	stats.depthsn[curDepth]++;

	genSuccs_wrap(curDepth);

	for (i = first_succ[ply]; i < first_succ[ply + 1]; i++) {
		sort_succ(i);
		const move m = gsuccs[i].m;

		if (!doMove_unchecked(m))
			continue;

		nvalid++;

		/* LMR */
		if (copts.lmr
			&& i >= first_succ[ply - 1] + LMR_FULL
			&& curDepth >= LMR_MINDEPTH
			&& gsuccs[i].s * 3 < gsuccs[first_succ[ply - 1]].s
			&& !inCheck(G->turn)
			&& maxDepth - curDepth >= 2
			&&  hstack[hply - 1].capt == EMPTY
			&& !hstack[hply - 1].was_promote) {
			stats.lmrs++;

			t = -negamax(maxDepth-1, curDepth+1, NULL, -beta, -alpha);

			/* Do a full search if it didn't fail low */
			if (t > alpha) {
				t = -negamax(maxDepth, curDepth+1, NULL,
					     -beta, -alpha);
			} else {
				stats.lmrs_ok++;
			}
		} else {
			t = -negamax(maxDepth, curDepth+1, NULL, -beta, -alpha);
		}

		undoMove();

		if (t > best) {
			best = t;
			bestmove = i;
			pv[ply][0] = m;
			pv_len[ply] = pv_len[ply + 1] + 1;
			memcpy(&pv[ply][1], &pv[ply+1][0],
			       sizeof(move) * pv_len[ply+1]);
		}

		if (t > alpha)
			alpha = t;

		if (alpha >= beta && copts.ab) {
			addon_notify_cut(m, curDepth);
			break;
		}
	}

	if (bestmove != -1)
		stats.picked[bestmove - first_succ[ply]]++;
	else
		assert(timeup || nvalid == 0);

	/* Era un tablero terminal? */
	if (nvalid == 0) {
		assert(!mm);

		if (inCheck(G->turn))
			ret = -CHECKMATE_SCORE + curDepth;
		else
			ret = 0; /* Stalemate */
	} else if (nvalid == 1 && alpha < beta && copts.forced_extend) {
		__unused bool check;
		check = doMove(gsuccs[bestmove].m);
		assert(check);

		alpha = alpha_orig;
		t = -negamax(maxDepth+1, curDepth+1, NULL, -beta, -alpha);

		undoMove();

		if (t > alpha)
			alpha = t;

		ret = alpha;
	} else {
		flag_t flag;

		assert(bestmove != -1);

		if (mm)
			*mm = gsuccs[bestmove].m;

		/*
		 * Devolver alpha o best es lo mismo (o son ambos menores o
		 * iguales a alpha_orig o son iguales) pero en la TT debemos
		 * guardar ${WHAT} porque ${REASON}
		 */
		assert(best == alpha ||
				(best < alpha_orig && alpha == alpha_orig));

		ret = best;
		assert(best != minScore);
		assert(best != maxScore);

		if (ret <= alpha_orig)
			flag = FLAG_UPPER_BOUND;
		else if (ret >= beta)
			flag = FLAG_LOWER_BOUND;
		else
			flag = FLAG_EXACT;

		addon_notify_return(gsuccs[bestmove].m, maxDepth - curDepth,
				    ret, flag);
	}

out:
	if (mm)
		assert(mm->move_type != MOVE_INVAL);

	assert(timeup || ret > minScore);
	assert(timeup || ret < maxScore);

	return ret;
}

score search(int maxDepth, move *mm, score alpha, score beta) {
	alpha = clamp(alpha, 1-CHECKMATE_SCORE, CHECKMATE_SCORE-1);
	beta  = clamp(beta , 1-CHECKMATE_SCORE, CHECKMATE_SCORE-1);
	assert(ply == 0);
	return negamax(maxDepth, 0, mm, alpha, beta);
}
