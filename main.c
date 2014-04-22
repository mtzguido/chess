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

struct player {
	void (*start)(int color);
	move (*getMove)(game);
	void (*notify)(game, move);
};

void checkMove(game g, move m) {
	game t = galloc();
	game ng = galloc();
	int i, nsucc;
	move *succs;

	*ng = *g;
	__maybe_unused int rc = doMove(ng, m);
	assert(rc);

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

__maybe_unused static void zobrist_test(game b, int d) {
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

int match(struct player pwhite, struct player pblack) {
	move m;
	game g = startingGame();

	pwhite.start(WHITE);
	pblack.start(BLACK);

	while(1) {
		int rc;

		mark(g);

		rc = isFinished(g);
		if (rc > 0) {
			if (rc == WIN(WHITE)) {
				fprintf(stderr, "RES: Win (checkmate)\n");
				return 12;
			} else if (rc == WIN(BLACK)) {
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
	fprintf(stderr, "RES: WHAT?");

	return 0;
}

enum play_mode {
	self,
	fairy,
	onemove,
	randplay,
};

struct {
	enum play_mode mode;
} behaviour;


int one_move() {
	game g;
	move m;

	start_all_addons();

	g = startingGame();

	mark(g);
	m = machineMove(g);
	doMove(g, m);
	printBoard(g);

	mark(g);
	m = machineMove(g);
	doMove(g, m);
	printBoard(g);

	return 0;
};

void parse_opt(int argc, char **argv) {
	behaviour.mode = fairy;
}

void printMove_wrap(game g, move m) {
	printMove(m);
}

void fairy_start(int col) {
	printf("new\n");
	if (col == WHITE) 
		printf("go\n");
}

struct player fairy_player =
{
	.getMove = playerMove,
	.notify = printMove_wrap,
	.start = fairy_start,
};

void ai_start() {
	start_all_addons();
}

void nothing() {
}

struct player ai_player =
{
	.start = ai_start,
	.getMove = machineMove,
	.notify = nothing,
};

move random_move(game g) {
	move *arr;
	int n = genSuccs(g, &arr);

	game ng = copyGame(g);

	do {
		int i = rand()%n;

		if (doMove(ng, arr[i])) {
			freeGame(ng);
			freeSuccs(arr, n);
			return arr[i];
		}

		*ng = *g;
	} while (1);
}

struct player random_player = {
	.start = nothing,
	.notify = nothing,
	.getMove = random_move,
};

int main(int argc, char **argv) {
	int rc;

	signal(SIGSEGV, handler);
	srand(time(NULL) + getpid());
	init_mem();

	parse_opt(argc, argv);

	switch (behaviour.mode) {
	case self:
		rc = match(ai_player, ai_player);
		break;
	case fairy:
		rc = match(ai_player, fairy_player);
		break;
	case onemove:
		rc = one_move();
		break;
	case randplay:
		rc = match(random_player, fairy_player);
		break;
	}

	fprintf(stderr, "Total nodes: %i\n", stats.totalopen);
	fprintf(stderr, "Total time: %ims\n", stats.totalms);
	fprintf(stderr, "program returned %i\n", rc);
	return 0;
}
