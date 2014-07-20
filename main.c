#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ai.h"
#include "board.h"
#include "pgn.h"
#include "mem.h"
#include "ztable.h"
#include "addon_list.h"
#include "succs.h"
#include "user_input.h"
#include "common.h"

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

void checkMove(game g, move m) {
	game t = galloc();
	game ng = galloc();
	int i, nsucc;
	struct MS *succs;

	*ng = *g;
	__maybe_unused int rc = doMove(ng, m);
	assert(rc);

	nsucc = genSuccs(g, &succs);

	for (i=0; i<nsucc; i++) {
		*t = *g;
		if (!doMove(t, succs[i].m))
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

	if (m.move_type == MOVE_INVAL)
		return playerMove(g);

	free(line);
	return m;
}

__maybe_unused static void zobrist_test(game b, int d) {
	mark(b);
	struct MS *succs;
	int i;

	stats.totalopen++;

	if (rand() > rand())
		return;

	if (d >= copts.depth)
		return;

	int n = genSuccs(b, &succs);

	for (i=0; i<n; i++) {
		game t = copyGame(b);
		if (!doMove(t, succs[i].m))
			continue;

		zobrist_test(t, d+1);
		freeGame(t);
	}

	freeSuccs(succs, n);
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
			sleep(1);
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

int nmoves() {
	int i;
	game g;
	move m;

	start_all_addons();
	g = startingGame2();

	for (i=0; i<copts.nmoves; i++) {
		mark(g);

		if (isFinished(g) != -1)
			break;

		m = machineMove(g);
		doMove(g, m);
		printBoard(g);

		fprintf(stderr, "Moves %i/%i\n", i+1, copts.nmoves);
	}

	return 0;
};

void parse_opt(int argc, char **argv) {
	int c, idx;
	static const struct option long_opts[] = {
		{ "moves",	required_argument,	0, 'm'},
		{ "self",	no_argument,		0, 's'},
		{ "rand",	no_argument,		0, 'r'},
		{ "ai-vs-rand",	no_argument,		0, 0x2 },

		{ "black",	no_argument,		0, 'b' },
		{ "init",	required_argument,	0, 0xc },

		{ "depth",	required_argument,	0, 'd'},
		{ "shuffle",	no_argument,		0, 0x1 },
		{ "no-tt",	no_argument,		0, 0x3 },
		{ "reverse",	no_argument,		0, 0x4 },
		{ "no-ab",	no_argument,		0, 0x5 },
		{ "no-quiesce",	no_argument,		0, 0x6 },
		{ "no-book",	no_argument,		0, 0x7 },
		{ "seed",	required_argument,	0, 0x8},
		{ "no-null",	no_argument,		0, 0x9 },
		{ "no-iter",	no_argument,		0, 0xa },
		{ "no-lmr",	no_argument,		0, 0xb },
		{ "no-sort",	no_argument,		0, 0xd },
		{ 0,0,0,0 }
	};

	copts = defopts;
	while (c = getopt_long(argc, argv, "", long_opts, &idx), c != -1) {
		switch (c) {
		case 'b':
			copts.black = true;
			break;
		case 's':
			copts.mode = self;
			break;
		case 'm':
			copts.mode = moves;
			copts.nmoves = atoi(optarg);
			break;
		case 'r':
			copts.mode = randplay;
			break;
		case 'd':
			copts.depth = atoi(optarg);
			if (copts.depth < 0 || copts.depth > MAX_DEPTH) {
				fprintf(stderr, "Invalid depth\n");
				exit(1);
			}
			break;
		case 0x1:
			copts.shuffle = true;
			break;
		case 0x2:
			copts.mode = ai_vs_rand;
			break;
		case 0x3:
			copts.heur_trans = false;
			break;
		case 0x4:
			copts.reverse = true;
			break;
		case 0x5:
			copts.ab = false;
			break;
		case 0x6:
			copts.quiesce = false;
			break;
		case 0x7:
			copts.usebook = false;
			break;
		case 0x8:
			srand(atoi(optarg));
			break;
		case 0x9:
			copts.nullmove = false;
			break;
		case 0xa:
			copts.iter = false;
			break;
		case 0xb:
			copts.lmr = false;
			break;
		case 0xc:
			copts.custom_start = true;
			strcpy(copts.custom_start_str, optarg);
			break;
		case 0xd:
			copts.sort = false;
			break;
		case '?':
		default:
			exit(1);
		}
	}
}

void printMove_wrap(game g, move m) {
	printMove(m);
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
	start_all_addons();
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
	struct MS *arr;
	int n = genSuccs(g, &arr);

	game ng = copyGame(g);

	do {
		int i = rand()%n;

		if (doMove(ng, arr[i].m)) {
			struct MS temp = arr[i];
			freeGame(ng);
			freeSuccs(arr, n);
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

	srand(time(NULL) + getpid());
	init_mem();

	parse_opt(argc, argv);

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
	}

	fprintf(stderr, ">> Total nodes: %lld\n", stats.totalopen);
	fprintf(stderr, ">> Total time: %lldms\n", stats.totalms);
	fprintf(stderr, "program returned %i\n", rc);
	return 0;
}
