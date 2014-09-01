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

#include <syslog.h>
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

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
	mark(g);

	for (i=0; i<copts.nmoves; i++) {

		if (isFinished(g) != -1) {
			dbg("Game finished: %i\n", isFinished(g));
			break;
		}

		assert(ply == 0);
		m = machineMove(g, 0);
		assert(ply == 0);
		doMove(g, m);
		mark(g);
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

static int bench_eval_mode() {
	const int N = 1e6;
	int i;
	unsigned long long t1, t2;
	game g = startingGame2();

	t1 = getms();
	for (i = 0; i < N; i++)
		boardEval(g);
	t2 = getms();

	printf("%i evals in %.3fs\n", N, (t2-t1)/1000.0);
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

__unused
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

static void sigterm(__unused int n) {
	dbg("Received SIGTERM!\n");
}

typedef struct state {
	game g;
	struct state *prev;
} state;

static void xboard_main() {
	int curPlayer = WHITE;
	int ourPlayer = BLACK;
	char line[500];
	char cmd[500];
	state *State;

	/* Timing info */
	int movesmax = 40, movesleft = 40;
	int timeinc = 0, timemax = 300000, timeleft = 300000;

	/* Ignore SIGINT, because xboard is a crappy protocol */
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, sigterm);

	printf("tellics say dogui's chess engine\n");
	printf("tellics say Written by Guido Martínez, 2014\n");

	init_mem();
	State = (state*) malloc(sizeof *State);
	State->g = startingGame();
	State->prev = NULL;
	mark(State->g);

	for (;;) {
		if (isFinished(State->g) == -1 && curPlayer == ourPlayer) {
			__unused bool check;
			unsigned long t1, t2;
			state *newst;
			int maxms;

			t1 = getms();

			if (movesmax == 0)
				maxms = (timeleft - 500) / 50;
			else
				maxms = (timeleft - 500) / movesleft;

			move m = machineMove(State->g, maxms);

			newst = (state*) malloc(sizeof *newst);
			newst->g = copyGame(State->g);
			check = doMove(newst->g, m);
			assert(check);
			mark(newst->g);
			newst->prev = State;
			State = newst;

			xboard_printmove(m);

			t2 = getms();

			timeleft -= t2 -t1;
			movesleft--;
			if (movesleft == 0) {
				movesleft = movesmax;
				if (movesmax == 1)
					timeleft = timemax;
				else
					timeleft += timemax;

				timemax += timeinc;
			}

			curPlayer = flipTurn(curPlayer);
			continue;
		}

		printBoard(State->g);
		dbg("expecting input...\n");

		if (!fgets(line, sizeof line, stdin)) {
			fprintf(stderr, "Unexpected end of input\n");
			exit(1);
		}

		if (line[0] == '\n')
			continue;

		sscanf(line, "%s", cmd);

		if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = 0;

		dbg("received: <%s>, cmd = %s\n", line, cmd);

		if (!strcmp("new", cmd)) {
			while (State) {
				state *t;
				t = State->prev;
				freeGame(State->g);
				free(State);
				State = t;
			}

			State = (state*) malloc(sizeof *State);
			State->prev = NULL;
			State->g = startingGame();
			continue;
		} else if (!strcmp("go", cmd)) {
			ourPlayer = curPlayer;
			continue;
		} else if (!strcmp("black", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("computer", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("easy", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("force", cmd)) {
			ourPlayer = 2; /* No one */
			continue;
		} else if (!strcmp("hard", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("level", cmd)) {
			int t;
			if (sscanf(line, "level %i %i %i", &movesmax, &timemax,
						&timeinc) == 3) {
				timemax *= 60 * 1000;
				timeinc *= 1000;
			} else if (sscanf(line, "level %i %i:%i %i", &movesmax,
						&timemax, &t, &timeinc) == 4) {
				timemax = timemax * 60 * 1000 + t * 1000;
				timeinc *= 1000;
			}
			movesleft = movesmax;
			timeleft = timemax;
			continue;
		} else if (!strcmp("nopost", cmd)) {
		} else if (!strcmp("otim", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("ping", cmd)) {
			int n;
			sscanf(line, "ping %d", &n);
			printf("pong %d\n", n);
			continue;
		} else if (!strcmp("post", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("protver", cmd)) {
		} else if (!strcmp("st", cmd)) {
			sscanf(line, "st %d", &timemax);
			timemax *= 1000;
			timeleft = timemax;
			movesleft = movesmax = 1;
			timeinc = 0;
			continue;
		} else if (!strcmp("quit", cmd)) {
			break;
		} else if (!strcmp("random", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("result", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("time", cmd)) {
			sscanf(line, "time %d", &timeleft);
			timeleft *= 10; /* cs to ms */
			continue;
		} else if (!strcmp("undo", cmd)) {
			if (State->prev) {
				state *t = State->prev;
				freeGame(State->g);
				free(State);
				State = t;
			}
			continue;
		} else if (!strcmp("white", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("xboard", cmd)) {
			/* Ignore */
			continue;
		} else if (!strcmp("?", cmd)) {
		} else {
			__unused bool check;
			state *newst;
			/* It's likely a move, try to parse it */
			dbg("is it a move?\n");
			move m = parseMove(State->g, line);

			/* Couldn't parse it */
			if (m.move_type == MOVE_INVAL) {
				printf("Error (unknown command): %s\n", line);
				continue;
			}

			if (checkMove(State->g, m)) {
				printf("Error (illegal move): %s\n", line);
				continue;
			}

			newst = (state*) malloc(sizeof *newst);
			newst->g = copyGame(State->g);
			check = doMove(newst->g, m);
			assert(check);
			mark(newst->g);
			newst->prev = State;
			State = newst;
			curPlayer = flipTurn(curPlayer);
			continue;
		}

		syslog(LOG_ERR, "Error (unknown command): %s\n", cmd);
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

	case bench_eval:
		bench_eval_mode();
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
