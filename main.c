#include "ai.h"
#include "board.h"
#include "pgn.h"
#include "ztable.h"
#include "succs.h"
#include "user_input.h"
#include "common.h"
#include "opts.h"
#include "autoversion.h"
#include "eval.h"
#include "moves.h"
#include "check.h"

#include <syslog.h>
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

static char arg_string[1024] = "";

static void startingGame2() {
	if (copts.custom_start) {
		fromstr(copts.custom_start_str);
		printBoard();
	} else {
		startingGame();
	}
	mark();
}

static int nmoves() {
	int i;
	move m;

	startingGame2();

	for (i = 0; i < copts.nmoves; i++) {

		if (isFinished() != -1) {
			dbg("Game finished: %i\n", isFinished());
			break;
		}

		assert(ply == 0);
		m = machineMove(copts.timelimit);
		assert(ply == 0);
		doMove(m);
		ply = 0;
		printBoard();
		printMove(stdout, m);

		dbg("Moves %i/%i\n", i+1, copts.nmoves);
	}

	return 0;
};

static int board_eval_mode() {
	startingGame2();
	dbg("Board evaluation: %i\n", boardEval());
	return 0;
}

static int bench_eval_mode() {
	const int N = 1e6;
	int i;
	unsigned long long t1, t2;
	startingGame2();

	t1 = getms();
	for (i = 0; i < N; i++)
		boardEval();
	t2 = getms();

	printf("%i evals in %.3fs\n", N, (t2-t1)/1000.0);
	return 0;
}

static int checkMove(move m) {
	int i;

	/*
	 * Fix promotions to Knights or Queens,
	 * we don't care about not generating rook/bishop
	 * promotions. Fairymax doesn't even care about
	 * knights so we're being extra kind here.
	 */
	if (isPromotion(m) && m.promote != WKNIGHT)
		m.promote = WQUEEN;

	int rc = doMove(m);

	if (rc != true)
		return 1;

	undoMove();

	if (isCapture(m) || isPromotion(m))
		genCaps();
	else
		genSuccs();

	for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
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
static void logToBook(move m) {
	static FILE *game_log = NULL;
	static int movenum = 1;
	struct pgn pp;
	char mbuf[10];

	if (!copts.log)
		return;

	if (game_log == NULL)
		game_log = fopen("gamelog", "w");

	pp = toPGN(m);
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

static void xboard_main() {
	int curPlayer = WHITE;
	int ourPlayer = BLACK;
	char line[500];
	char cmd[500];

	/* Timing info */
	int movesmax = 40, movesleft = 40;
	int timeinc = 0, timemax = 300000, timeleft = 300000;

	/* Ignore SIGINT, because xboard is a crappy protocol */
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, sigterm);
	printf("feature sigint=0 sigterm=0 myname=\"%s%s\"\n", short_version(),
			arg_string);
	printf("feature colors=0 setboard=0 playother=0 reuse=0\n");
	printf("feature done=1\n");

	printf("tellics say dogui's chess engine\n");
	printf("tellics say Written by Guido Martínez, 2014\n");

	startingGame2();

	for (;;) {
		if (isFinished() == -1 && curPlayer == ourPlayer) {
			__unused bool check;
			unsigned long t1, t2;
			int maxms;

			t1 = getms();

			if (movesmax == 0)
				maxms = (timeleft - 500) / 50;
			else
				maxms = (timeleft - 500) / movesleft;

			move m = machineMove(maxms);

			check = doMove(m);
			assert(check);
			ply = 0;

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
		} else if (isFinished() != -1) {
			switch (isFinished()) {
			case DRAW_50MOVE:
				printf("1/2-1/2 {Draw by fifty move rule}\n");
				break;
			case DRAW_3FOLD:
				printf("1/2-1/2 {Draw by repetition}\n");
				break;
			case DRAW_STALE:
				printf("1/2-1/2 {Stalemate}\n");
				break;
			case WIN(WHITE):
				printf("1-0 {Checkmate}\n");
				break;
			case WIN(BLACK):
				printf("0-1 {Checkmate}\n");
				break;
			}

			ourPlayer = 2;
		}

		printBoard();
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
			while (hply)
				undoMove();

			unmark();

			startingGame2();
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
			if (hply) {
				ply = 1;
				undoMove();
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
			/* It's likely a move, try to parse it */
			dbg("is it a move?\n");
			move m = parseMove(line);

			/* Couldn't parse it */
			if (m.move_type == MOVE_INVAL) {
				printf("Error (unknown command): %s\n", line);
				continue;
			}

			if (checkMove(m)) {
				printf("Error (illegal move): %s\n", line);
				continue;
			}

			check = doMove(m);
			ply = 0;
			assert(check);
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

	if (argc > 1) {
		int i;

		strcat(arg_string, " args = [");

		for (i = 1; i < argc-1; i++) {
			strcat(arg_string, argv[i]);
			strcat(arg_string, ", ");
		}
		strcat(arg_string, argv[argc-1]);
		strcat(arg_string, "]");
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
