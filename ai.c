#include "ai.h"
#include "search.h"
#include "board.h"
#include "common.h"
#include "ztable.h"
#include "mem.h"
#include "addon.h"
#include "succs.h"
#include "book.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

struct stats stats;

static inline void reset_stats() {
	int i;

	n_collision		= 0;
	stats.nopen_s		= 0;
	stats.nopen_q		= 0;
	stats.ngen		= 0;
	stats.nall		= 0;
	stats.null_tries	= 0;
	stats.null_cuts		= 0;
	stats.tt_hits		= 0;
	stats.lmrs		= 0;
	stats.lmrs_ok		= 0;

	for (i = 0; i < 100; i++) {
		stats.depthsn[i] = 0;
		stats.picked[i] = 0;
	}
}

void print_stats(score exp, bool expect_ok) {
	dbg("stats: searched %lld (%lld) nodes\n",
			stats.nopen_s, stats.nopen_q);
	dbg("stats: branching aprox: %.3f\n",
			1.0 * (stats.nall - 1) / stats.nopen_s);
	dbg("stats: total nodes generated: %lld\n", stats.ngen);
	dbg("stats: null move cuts: %lld/%lld (%.2f%%)\n",
			stats.null_cuts, stats.null_tries,
			100.0 * stats.null_cuts / stats.null_tries);
	dbg("stats: TT hits: %lld\n", stats.tt_hits);
	dbg("stats: TT collisions: %lld\n", stats.tt_collision);
	dbg("stats: Late move reductions: %lld/%lld (%.2f%%)\n",
			stats.lmrs_ok, stats.lmrs,
			100.0 * stats.lmrs_ok / stats.lmrs);

	if (expect_ok)
		dbg("stats: expected score: %i\n", exp);
	else
		dbg("stats: expected score: ????\n");

	dbg("stats: Number of hash collisions: %i\n", n_collision);
}

static inline void print_time(clock_t t1, clock_t t2) {
	dbg("stats: moved in %.3f seconds\n",
			1.0*(t2-t1)/CLOCKS_PER_SEC);
}

static inline bool forced(const game g, move *m) {
	int i;
	int c = -1;
	game ng;

	assert(ply == 0);
	genSuccs(g);
	ng = copyGame(g);
	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		if (doMove_unchecked(ng, gsuccs[i].m)) {
			if (c != -1) {
				freeGame(ng);
				return false;
			}

			c = i;
			*ng = *g;
		}
	}

	assert(c != -1);
	*m = gsuccs[c].m;
	freeGame(ng);
	return true;
}

/* Time limit data */
bool timelimited;
bool timeup;
unsigned long timelimit;
unsigned long timestart;
int ticks;

move machineMove(const game start, unsigned long long maxms) {
	move ret = {0};
	clock_t t1, t2;
	score expected = 0;
	bool expect_ok;
	unsigned long long iterstart;
	unsigned long long now;

	ret.move_type = MOVE_INVAL;

	addon_reset();
	reset_stats();

	assert(isFinished(start) == -1);

	t1 = clock();
	G = start;
	if (copts.book && bookMove(&ret)) {
		dbg("stats: book move.\n");
		expect_ok = false;
	} else if (forced(start, &ret)) {
		dbg("stats: forced move.\n");
		expect_ok = false;
	} else {
		int d, md;
		move temp;
		score t;
		score alpha, beta;
		bool stop = false;

		alpha = minScore;
		beta = maxScore;
		expected = minScore;
		timelimited = maxms != 0;
		timeup = false;
		timestart = getms();
		timelimit = timestart + maxms;
		ticks = 0;
		md = -1;

		for (d = 1; !stop && !timeup; d++) {
			assert(ply == 0);
			iterstart = getms();
			t = search(start, d, &temp, alpha, beta);

			if (t >= beta) {
				beta = maxScore;
				t = search(start, d, &temp, alpha, beta);
				dbg("ASP_HIGH: %i %i %i\n", t, alpha, maxScore);
			} else if (t <= alpha) {
				alpha = minScore;
				t = search(start, d, &temp, alpha, beta);
				dbg("ASP_LOW: %i %i %i\n", t, minScore, beta);
			}

			if (t <= alpha || t >= beta) {
				dbg("%i %i\n", t<=alpha, t>=beta);
				alpha = minScore;
				beta = maxScore;
				t = search(start, d, &temp, alpha, beta);
				dbg("ASP_DUB: %i %i %i\n", t, minScore, maxScore);
			}

			if (timeup) {
				/*
				 * If we have a timeout, don't even consider
				 * this iteration
				 */
				dbg("machineMove: time up!\n");
				break;
			}

			now = getms();

			/* Time should be in centiseconds. No, really. */
			printf("%2d %6i %8llu %10llu e1f1\n", d, t,
					(now-iterstart)/10, stats.nopen_s);
			fflush(stdout);

			expected = t;
			ret = temp;
			md = d;
			alpha = copts.asp ? t - ASPIRATION_WINDOW : minScore;
			beta  = copts.asp ? t + ASPIRATION_WINDOW : maxScore;
			/*
			 * Smart stop, assume the next iteration will
			 * take _at least twice_ as long as the current one,
			 * and if we would time out, just time out now and
			 * save some precious seconds
			 */
			if (timelimited
				&& copts.smart_stop
				&& now + 2*(now - iterstart) >= timelimit) {
				dbg("Smart stopping! I think I saved %llu ms\n",
						timelimit - now);
				stop = true;
				continue;
			}

			/*
			 * Stop at the maximum depth, but think for at least
			 * copts.lbound miliseconds.
			 */
			stop = d >= copts.depth &&
				(copts.lbound == 0 ||
				now - timestart > copts.lbound);
		}

		dbg("machineMove: actual depth was: %i\n", md);

		expect_ok = true;
		assert(ply == 0);
	}
	t2 = clock();

	print_stats(expected, expect_ok);
	assert(ret.move_type != MOVE_INVAL);
	assert(ret.who == start->turn);
	dbg("move was %i %i %i %i %i\n",
			ret.move_type, ret.r, ret.c, ret.R, ret.C);

	stats.totalopen += stats.nopen_s + stats.nopen_q;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_time(t1, t2);
	fflush(NULL);
	return ret;
}
