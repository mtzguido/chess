#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "ai.h"
#include "board.h"
#include "pgn.h"
#include "mem.h"
#include "ztable.h"
#include "addon_list.h"
#include "succs.h"
#include "user_input.h"

#include <execinfo.h>
#include <signal.h>

void checkMove(game g, move m) {
	game t = galloc();
	game ng = galloc();
	int i, nsucc;
	move *succs;

	*ng = *g;
	assert(doMove(ng, m));

	nsucc = genSuccs(g, &succs);

	for (i=0; i<nsucc; i++) {
		*t = *g;
		if (!doMove(t, succs[i]))
			continue;

		if (equalGame(t, ng))
			goto ok;
	}

	fprintf(stderr, "-----------------------------------\n");
	fprintf(stderr, "MOVIDA NO CONTEMPLADA!!!!\n");
	printBoard(g);
	fprintf(stderr, "%i %i %i %i %i\n", m.move_type, m.r, m.c, m.R, m.C);
	fprintf(stderr, "-----------------------------------\n");
	abort();

ok:
	freeGame(t);
	freeGame(ng);
	freeSuccs(succs, nsucc);
}

void logToBook(game g, move m) {
	static FILE *game_log = NULL;
	static int movenum = 0;
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

void handler(int sig) {
	void *array[100];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 100);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

move playerMove(game g) {
	char *line = NULL;
	size_t crap = 0;
	move m;

	getline(&line, &crap, stdin);
	line[strlen(line)-1] = 0;

	fprintf(stderr, "LINE= <%s>\n", line);

	if (isPrefix("1/2-1/2 {", line)
			|| isPrefix("0-1 {", line)
			|| isPrefix("1-0 {", line)) {
		fprintf(stderr, "CLAIMED RES: <%s>\n", line);
		exit(1);
	}

	if (!isPrefix("move ", line))
		return playerMove(g);

	m = parseMove(g, line+5);

	if (m.move_type == -1)
		return playerMove(g);

	free(line);
	return m;
}

static void zobrist_test(game b, int d) {
	mark(b);
	move *succs;
	int i;

	stats.totalopen++;

	if (rand() > rand())
		return;

	if (d >= CFG_DEPTH)
		return;

	int n = genSuccs(b, &succs);

	for (i=0; i<n; i++) {
		game t = copyGame(b);
		if (!doMove(t, succs[i]))
			continue;

		zobrist_test(t, d+1);
		freeGame(t);
	}

	freeSuccs(succs, n);
}

int fairy_play(int machineColor) {
	move m;
	game g = startingGame();

	start_all_addons();

	printf("new\n");
	if (machineColor != WHITE) 
		printf("go\n");

	while(1) {
		int rc;

		mark(g);

		rc = isFinished(g);
		if (rc > 0) {
			if (rc == WIN(machineColor)) {
				fprintf(stderr, "RES: Win (checkmate)\n");
				return 12;
			} else if (rc == WIN(flipTurn(machineColor))) {
				fprintf(stderr, "RES: Lose (checkmate)\n");
				return 10;
			} else if (rc == DRAW_3FOLD) {
				fprintf(stderr, "RES: Draw (threefold)\n");
				return 11;
			} else if (rc == DRAW_50MOVE) {
				fprintf(stderr, "RES: Draw (fifty move)\n");
				return 11;
			} else if (rc == DRAW_STALE) {
				fprintf(stderr, "RES: Draw (stalemate)\n");
				return 11;
			} else {
				assert(0);
			}

			break;
		}
		printBoard(g);

		game prev = copyGame(g);

		if (g->turn == machineColor) {
			m = machineMove(g);
			assert(doMove(g, m));
			printMove(g->lastmove);
		} else {
			m = playerMove(g);
			checkMove(g, m);
			assert(doMove(g, m));
		}

		logToBook(prev, g->lastmove);
		freeGame(prev);
	}

	printf("quit\n");

	freeGame(g);
	fprintf(stderr, "RES: WHAT?");

	return 0;
}

int main(int argc, char **argv) {
	int rc;

	signal(SIGSEGV, handler);
	srand(time(NULL) + getpid());
	init_mem();

	//self_play();
	rc = fairy_play(WHITE);
	//zobrist_test();
	//one_move();

	fprintf(stderr, "Total nodes: %i\n", stats.totalopen);
	fprintf(stderr, "Total time: %ims\n", stats.totalms);
	fprintf(stderr, "program returned %i\n", rc);
	return 0;
}
