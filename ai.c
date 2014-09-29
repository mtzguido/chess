#define _GNU_SOURCE

#include "ai.h"
#include "search.h"
#include "board.h"
#include "common.h"
#include "ztable.h"
#include "addon.h"
#include "succs.h"
#include "book.h"
#include "moves.h"
#include "user_input.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

	for (i = 0; i < 100; i++)
		stats.depthsn[i] = 0;
}

void print_stats(score exp, bool expect_ok) {
	const struct stats S = stats;

	dbg("stats: searched %lld (%lld) nodes\n", S.nopen_s, S.nopen_q);
	dbg("stats: branching aprox: %.3f\n", 1.0 * (S.nall - 1) / S.nopen_s);
	dbg("stats: total nodes generated: %lld\n", S.ngen);
	dbg("stats: null move cuts: %lld/%lld (%.2f%%)\n",
			S.null_cuts, S.null_tries,
			100.0 * S.null_cuts / S.null_tries);
	dbg("stats: TT hits: %lld\n", S.tt_hits);
	dbg("stats: TT collisions: %lld\n", S.tt_collision);
	dbg("stats: Late move reductions: %lld/%lld (%.2f%%)\n",
			S.lmrs_ok, S.lmrs,
			100.0 * S.lmrs_ok / S.lmrs);
	dbg("stats: Number of hash collisions: %i\n", n_collision);

	if (expect_ok)
		dbg("stats: expected score: %i\n", exp);
	else
		dbg("stats: expected score: ????\n");
}

static inline void print_time(clock_t t1, clock_t t2) {
	dbg("stats: moved in %.3f seconds\n", 1.0*(t2-t1)/CLOCKS_PER_SEC);
}

static inline bool forced(move *m) {
	int i;
	int c = -1;

	assert(ply == 0);

	genSuccs();
	for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
		if (doMove_unchecked(&gsuccs[i].m)) {
			undoMove();

			if (c != -1)
				return false;

			c = i;
		}
	}

	assert(c != -1);
	*m = gsuccs[c].m;
	return true;
}

/* Time limit data */
bool timelimited;
bool timeup;
unsigned long timelimit;
unsigned long timestart;
int ticks;

static void print_pv(char *buf) {
	int i;

	buf[0] = 0;
	for (i = 0; i < pv_len[0]; i++) {
		move_text(pv[0][i], buf);
		strcat(buf, " ");
		buf += strlen(buf);
	}
}

move machineMove(unsigned long long maxms) {
	move ret = {0};
	clock_t t1, t2;
	score expected = 0;
	bool expect_ok;
	unsigned long long iterstart;
	unsigned long long now;
	int d, md;
	move temp;
	score t;
	score alpha, beta;
	bool stop = false;
	char pv_text[200];

	ret.move_type = MOVE_INVAL;

	addon_reset();
	reset_stats();

	assert(isFinished() == -1);
	assert(ply == 0);

	t1 = clock();
	if (copts.book) {
		if (bookMove(&ret)) {
			dbg("stats: book move.\n");
			expect_ok = false;
			goto out;
		}
	}

	if (forced(&ret)) {
		dbg("stats: forced move.\n");
		expect_ok = false;
		goto out;
	}

	alpha = minScore;
	beta = maxScore;
	expected = minScore;
	md = -1;

	if (maxms != 0) {
		timelimited = true;
		timeup = false;
		timestart = getms();
		timelimit = timestart + maxms;
		ticks = 0;
	} else {
		timelimited = false;
	}

	for (d = 1; !stop && !timeup; d++) {
		assert(ply == 0);
		iterstart = getms();
		t = search(d, &temp, alpha, beta);

		if (t >= beta) {
			beta = maxScore;
			t = search(d, &temp, alpha, beta);
			dbg("ASP_HIGH: %i %i %i\n", t, alpha, maxScore);
		} else if (t <= alpha) {
			alpha = minScore;
			t = search(d, &temp, alpha, beta);
			dbg("ASP_LOW: %i %i %i\n", t, minScore, beta);
		}

		if (t <= alpha || t >= beta) {
			dbg("%i %i\n", t<=alpha, t>=beta);
			alpha = minScore;
			beta = maxScore;
			t = search(d, &temp, alpha, beta);
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

		print_pv(pv_text);

		/* Time should be in centiseconds. No, really. */
		printf("%2d %6i %8llu %10llu %s\n", d, t,
				(now-iterstart)/10, stats.nopen_s,
				pv_text);
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

out:
	t2 = clock();

	print_stats(expected, expect_ok);
	assert(ret.move_type != MOVE_INVAL);
	assert(ret.who == G->turn);

	*strchrnul(pv_text, ' ') = 0;

	dbg("move was %s\n", pv_text);

	stats.totalopen += stats.nopen_s + stats.nopen_q;
	stats.totalms += 1000*(t2-t1)/CLOCKS_PER_SEC;

	print_time(t1, t2);
	fflush(NULL);
	return ret;
}
