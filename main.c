#include "ai.h"
#include "board.h"
#include "pgn.h"
#include "mem.h"
#include "ztable.h"
#include "succs.h"
#include "user_input.h"
#include "common.h"
#include "opts.h"
#include "autoversion.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct player {
	void (*start)(int color);
	move (*getMove)(game);
	void (*notify)(game, move);
	void (*notify_res)(int res);
};

game startingGame2() {
	game g;

	if (copts.custom_start) {
		g = fromstr(copts.custom_start_str);
		printBoard(g);
	} else {
		g = startingGame();
	}

	return g;
}

static int board_eval_mode() {
	game g = startingGame2();

	dbg("Board evaluation: %i\n", boardEval(g));

	return 0;
}

void checkMove(game g, move m) {
	game t = galloc();
	game ng = galloc();
	int i;

	*ng = *g;
	__maybe_unused int rc = doMove(ng, m);
	if (!rc) {
		dbg("-----------------------------------\n");
		dbg("MOVIDA ILEGAL?!?!?!!\n");
		printBoard(g);
		dbg("%i %i %i %i %i\n", m.move_type, m.r, m.c, m.R, m.C);
		dbg("-----------------------------------\n");
		abort();
	}

	if (isCapture(g, m) || isPromotion(g, m))
		genCaps(g);
	else
		genSuccs(g);

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		*t = *g;
		if (!doMove(t, gsuccs[i].m))
			continue;

		if (equalGame(t, ng))
			goto ok;
	}

	dbg("-----------------------------------\n");
	dbg("MOVIDA NO CONTEMPLADA!!!!\n");
	printBoard(g);
	dbg("%i %i %i %i %i\n", m.move_type, m.r, m.c, m.R, m.C);
	dbg("-----------------------------------\n");
	abort();

ok:
	freeGame(t);
	freeGame(ng);
}

void logToBook(game g, move m) {
	static FILE *game_log = NULL;
	static int movenum = 1;
	struct pgn pp;
	char mbuf[10];

	if (game_log == NULL)
		game_log = fopen("gamelog", "w");

	pp = toPGN(g, m);
	stringPGN(mbuf, pp);

	fprintf(game_log, "%i. %s ", movenum, mbuf);

	if (movenum % 2 == 0)
		fprintf(game_log, "\n");

	movenum++;
}

move playerMove(game g) {
	char *line = NULL;
	size_t crap = 0;
	move m;

	getline(&line, &crap, stdin);
	line[strlen(line)-1] = 0;

	dbg("LINE= <%s>\n", line);

	if (isPrefix("1/2-1/2 {", line)
			|| isPrefix("0-1 {", line)
			|| isPrefix("1-0 {", line)) {
		dbg("CLAIMED RES: <%s>\n", line);
		exit(1);
	}

	if (!isPrefix("move ", line))
		return playerMove(g);

	m = parseMove(g, line+5);

	if (m.move_type == MOVE_INVAL)
		return playerMove(g);

	free(line);
	return m;
}

int match(struct player pwhite, struct player pblack) {
	move m;
	game g;

	g = startingGame2();

	pwhite.start(WHITE);
	pblack.start(BLACK);

	while(1) {
		int rc;

		mark(g);

		rc = isFinished(g);
		if (rc > 0) {
			usleep(50000);
			if (rc == WIN(WHITE)) {
				dbg("RES: Win (checkmate)\n");
				return 12;
			} else if (rc == WIN(BLACK)) {
				dbg("RES: Lose (checkmate)\n");
				return 10;
			} else if (rc == DRAW_3FOLD) {
				dbg("RES: Draw (threefold)\n");
				return 11;
			} else if (rc == DRAW_50MOVE) {
				dbg("RES: Draw (fifty move)\n");
				return 11;
			} else if (rc == DRAW_STALE) {
				dbg("RES: Draw (stalemate)\n");
				return 11;
			} else {
				assert(0);
			}
		}

		printBoard(g);

		game prev = copyGame(g);

		if (g->turn == WHITE) {
			m = pwhite.getMove(g);
			checkMove(g, m);
			__maybe_unused int rc = doMove(g, m);
			assert(rc);
			pblack.notify(g, g->lastmove);
		} else {
			m = pblack.getMove(g);
			checkMove(g, m);
			__maybe_unused int rc = doMove(g, m);
			assert(rc);
			pwhite.notify(g, g->lastmove);
		}

		logToBook(prev, g->lastmove);
		freeGame(prev);
	}

	printf("quit\n");

	freeGame(g);
	dbg("RES: WHAT?");

	return 0;
}

int nmoves() {
	int i;
	game g;
	move m;

	g = startingGame2();

	for (i=0; i<copts.nmoves; i++) {
		mark(g);

		if (isFinished(g) != -1)
			break;

		assert(ply == 0);
		m = machineMove(g);
		assert(ply == 0);
		doMove(g, m);
		printBoard(g);
		printMove(stdout, m);

		dbg("Moves %i/%i\n", i+1, copts.nmoves);
	}

	return 0;
};

void printMove_wrap(game g, move m) {
	printMove(stdout, m);
	fflush(NULL);
}

void fairy_start(int col) {
	printf("new\n");
	if (col == WHITE) 
		printf("go\n");
}

void fairy_notify_res(int res) {
	switch (res) {
	case WIN(WHITE):
		printf("1-0 {White mates G}\n");
		break;
	case WIN(BLACK):
		printf("0-1 {Black mates G}\n");
		break;
	case DRAW_3FOLD:
		printf("1/2-1/2 {Draw by repetition G}\n");
		break;
	case DRAW_50MOVE:
		printf("1/2-1/2 {50 Move rule G}\n");
		break;
	case DRAW_STALE:
		printf("1/2-1/2 {Stalemate G}\n");
		break;
	}
	fflush(NULL);
}

struct player ui_player =
{
	.getMove = playerMove,
	.notify = printMove_wrap,
	.start = fairy_start,
	.notify_res = fairy_notify_res,
};

void ai_start() {
}

void nothing() {
}

struct player ai_player =
{
	.start = ai_start,
	.getMove = machineMove,
	.notify = nothing,
	.notify_res = nothing,
};

move random_move(game g) {
	int i;

	genSuccs(g);
	game ng = copyGame(g);
	do {
		i = rand()%(first_succ[ply+1] - first_succ[ply]) +
			first_succ[ply];

		if (doMove(ng, gsuccs[i].m)) {
			struct MS temp = gsuccs[i];
			freeGame(ng);
			return temp.m;
		}

		*ng = *g;
	} while (1);
}

struct player random_player = {
	.start = nothing,
	.notify = nothing,
	.getMove = random_move,
	.notify_res = nothing,
};

int main(int argc, char **argv) {
	int rc = 0;

	init_mem();

	if (!parse_opt(argc, argv)) {
		fprintf(stderr, "FATAL: could not parse options\n");
		exit(1);
	}

	dbg("random seed: %u\n", copts.seed);
	srand(copts.seed);

	switch (copts.mode) {
	case normal:
		if (copts.black)
			rc = match(ui_player, ai_player);
		else
			rc = match(ai_player, ui_player);

		break;
	case self:
		rc = match(ai_player, ai_player);
		break;
	case moves:
		rc = nmoves();
		break;
	case randplay:
		if (copts.black)
			rc = match(ui_player, random_player);
		else
			rc = match(random_player, ui_player);

		break;
	case ai_vs_rand:
		rc = match(ai_player, random_player);
		break;
	case board_eval:
		board_eval_mode();
		break;
	case version:
		print_version();
		exit(0);
		break;
	case help:
		print_help(argv[0]);
		exit(0);
		break;
	}

	dbg("Total nodes: %lld\n", stats.totalopen);
	dbg("Total time: %lldms\n", stats.totalms);
	dbg("Returned %i\n", rc);
	return 0;
}
