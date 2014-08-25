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

static game startingGame2() {
	game g;

	if (copts.custom_start) {
		g = fromstr(copts.custom_start_str);
		printBoard(g);
	} else {
		g = startingGame();
	}

	return g;
}

static int nmoves() {
	int i;
	game g;
	move m;

	init_mem();
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

static int board_eval_mode() {
	game g = startingGame2();
	dbg("Board evaluation: %i\n", boardEval(g));
	return 0;
}

static int checkMove(game g, move m) {
	game ng;
	int i;

	/*
	 * Fix promotions to Knights or Queens,
	 * we don't care about not generating rook/bishop
	 * promotions. Fairymax doesn't even care about
	 * knights so we're being extra kind here.
	 */
	if (isPromotion(g, m) && m.promote != WKNIGHT)
		m.promote = WQUEEN;

	ng = copyGame(g);
	int rc = doMove(ng, m);
	freeGame(ng);

	if (rc != true)
		return 1;

	if (isCapture(g, m) || isPromotion(g, m))
		genCaps(g);
	else
		genSuccs(g);

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		if (equalMove(m, gsuccs[i].m))
			return 0;
	}

	/*
	 * We didn't anticipate that move, just call it illegal
	 * like a good proud player and try to carry on
	 */
	printf("Illegal move\n");
	return 1;
}

static void logToBook(game g, move m) {
	static FILE *game_log = NULL;
	static int movenum = 1;
	struct pgn pp;
	char mbuf[10];

	if (!copts.log)
		return;

	if (game_log == NULL)
		game_log = fopen("gamelog", "w");

	pp = toPGN(g, m);
	stringPGN(mbuf, pp);

	fprintf(game_log, "%i. %s ", movenum, mbuf);

	if (movenum % 2 == 0)
		fprintf(game_log, "\n");

	movenum++;
}

static void xboard_printmove(move m) {
	printMove(stdout, m);
}

static void xboard_main() {
	int curPlayer = WHITE;
	int ourPlayer = BLACK;
	char buf[500];
	game g;

	printf("\n");
	printf("dogui's chess engine\n");
	printf("Guido Martínez, 2014\n");
	printf("\n");

	init_mem();
	g = startingGame();

	for (;;) {
		if (curPlayer == ourPlayer) {
			move m = machineMove(g);
			xboard_printmove(m);
			curPlayer = flipTurn(curPlayer);
			continue;
		}

		if (!fgets(buf, sizeof buf, stdin)) {
			fprintf(stderr, "Unexpected end of input\n");
			exit(1);
		}

		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = 0;

		if (isPrefix("move ", buf)) {
			move m = parseMove(g, buf+5);
			if (checkMove(g, m)) {
				printf("Error (illegal move): %s\n", buf+5);
				continue;
			}

			doMove(g, m);
			curPlayer = flipTurn(curPlayer);
		} else if (isPrefix("new", buf)) {
			freeGame(g);
			g = startingGame();
		} else if (isPrefix("go", buf)) {
			__unused bool check;
			move m = machineMove(g);

			check = doMove(g, m);
			assert(check);

			xboard_printmove(m);
			curPlayer = flipTurn(curPlayer);
			ourPlayer = flipTurn(ourPlayer);
		} else if (isPrefix("xboard", buf)) {
		} else {
			printf("Error (unknown command): %s\n", buf);
		}
	}
}

int main(int argc, char **argv) {
	if (!parse_opt(argc, argv)) {
		fprintf(stderr, "FATAL: could not parse options\n");
		exit(1);
	}

	dbg("random seed: %u\n", copts.seed);
	srand(copts.seed);

	switch (copts.mode) {
	case xboard:
		xboard_main();
		break;

	case moves:
		nmoves();
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
	return 0;
}
