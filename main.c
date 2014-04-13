#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "ai.h"
#include "board.h"
#include "pgn.h"
#include "mem.h"
#include "ztable.h"
#include "addon_list.h"
#include "succs.h"

#include <execinfo.h>
#include <signal.h>

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

/* a prefijo de b */
static int isPrefix(char *a, char *b) {
	while (*a && *a++ == *b++)
		;

	if (*a == 0)
		return 1;

	return 0;
}

static char pieceOf(char c) {
	switch (c) {
		case 'P':   return WPAWN;
		case 'p':   return WPAWN;
		case 'R':   return WROOK;
		case 'r':   return WROOK;
		case 'N':   return WKNIGHT;
		case 'n':   return WKNIGHT;
		case 'B':   return WBISHOP;
		case 'b':   return WBISHOP;
		case 'Q':   return WQUEEN;
		case 'q':   return WQUEEN;
		case 'k':   return WKING;
		case 'K':   return WKING;
		default:    return EMPTY;
	}   
}

static void zobrist_test(game b, int d) __attribute__((unused));
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

int main_trucho (int argc, char **argv) {
	init_mem();
	srand(time(NULL) + getpid());
	game b = startingGame();
	int machineColor;

	if (argc > 1 && argv[1][0] == 'w')
		machineColor = WHITE;
	else
		machineColor = BLACK;

	int movenum = 1;

	start_all_addons();

	FILE *game_log = fopen(machineColor == WHITE ? "gamelog_w" : "gamelog_b", "w");

#if 0
	{
		game *arr;
		int i, n;

		n = genSuccs(b, &arr);
		fprintf(stderr, "N=%i\n", n);

		for (i = 0; i < n; i++) {
			printBoard(arr[i]);
			fprintf(stderr, "move was: (%i,%i) -> (%i,%i)\n",
					arr[i]->lastmove.r,
					arr[i]->lastmove.c,
					arr[i]->lastmove.R,
					arr[i]->lastmove.C);

			getchar();
		}

		return 0;
	}
#endif

#if 0
	while (isFinished(b) == -1) {
		b = machineMove(b);
		move m = b->lastmove;
 
		if (m.move_type == MOVE_KINGSIDE_CASTLE) {
			if (m.who == BLACK) {
				printf("e8g8\n");
				fprintf(stderr, "e8g8\n");
			} else {
				printf("e1g1\n");
				fprintf(stderr, "e1g1\n");
			}
		} else if (m.move_type == MOVE_QUEENSIDE_CASTLE) {
			if (m.who == BLACK) {
				printf("e8c8\n");
				fprintf(stderr, "e8c8\n");
			} else {
				printf("e1c1\n");
				fprintf(stderr, "e1c1\n");
			}
		} else {
			assert(m.move_type == MOVE_REGULAR);

			if (m.was_promotion) {
				printf("%c%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R, tolower(charOf(m.promote)));
				fprintf(stderr, "%c%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R, tolower(charOf(m.promote)));
			} else {
				printf("%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R);
				fprintf(stderr, "%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R);
			}
		}

		printBoard(b);
	}
#endif

#if 0
	{
		printBoard(machineMove(b));
		return 0;
	}
#endif

#if 0
	zobrist_test(b, 0);
	return 0;
#endif

	move m;
	char mbuf[500];
	struct pgn pp;

	printf("new\n");
	if (machineColor != WHITE) 
		printf("go\n");

	mark(b);

	while(1) {

		int rc = isFinished(b);
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

		game t = copyGame(b);
		printBoard(b);

		fprintf(stderr, "turn=%i\n", b->turn);

		if (b->turn == machineColor) {
			game nb;

			nb = machineMove(b);
			assert(nb != NULL);
			freeGame(b);
			b = nb;

			m = b->lastmove;
			if (m.move_type == MOVE_KINGSIDE_CASTLE) {
				if (m.who == BLACK) {
					printf("e8g8\n");
					fprintf(stderr, "e8g8\n");
				} else {
					printf("e1g1\n");
					fprintf(stderr, "e1g1\n");
				}
			} else if (m.move_type == MOVE_QUEENSIDE_CASTLE) {
				if (m.who == BLACK) {
					printf("e8c8\n");
					fprintf(stderr, "e8c8\n");
				} else {
					printf("e1c1\n");
					fprintf(stderr, "e1c1\n");
				}
			} else {
				assert(m.move_type == MOVE_REGULAR);

				if (m.was_promotion) {
					printf("%c%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R, tolower(charOf(m.promote)));
					fprintf(stderr, "%c%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R, tolower(charOf(m.promote)));
				} else {
					printf("%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R);
					fprintf(stderr, "%c%c%c%c\n", m.c + 'a', '8'-m.r, m.C + 'a', '8'-m.R);
				}
			}
		} else {
			int r, R;
			char c, C;
			char newpiece;
			int t;
			char *line = NULL;
			size_t crap = 0;

			m.who = flipTurn(machineColor);

			getline(&line, &crap, stdin);
			line[strlen(line)-1] = 0;

			fprintf(stderr, "LINE= <%s>\n", line);

			if (isPrefix("1/2-1/2 {", line)
				|| isPrefix("0-1 {", line)
				|| isPrefix("1-0 {", line)) {
				fprintf(stderr, "RES: <%s>\n", line);
				return 101;
			}

			if (!isPrefix("move ", line)) {
				continue;
			}

			if (5 != (t=sscanf(line, "move %c%i%c%i%c", &c, &r, &C, &R, &newpiece))
					&& (newpiece = 0) /* muuuuuuy chanta */
					&& (4 != (t=sscanf(line, "move %c%i%c%i", &c, &r, &C, &R)))) {
				fprintf(stderr, "Ignoring line <%s>\n", line);
				continue;
			}

			free(line);

			c = tolower(c);
			C = tolower(C);

			fprintf(stderr, "Your move: %c%i -> %c%i\n", c, r, C, R);

			if (c < 'a' || c > 'h' || C < 'a' || C > 'h' ||
					r < 0   || r > 8   || R < 0   || R > 8  ) {
				fprintf(stderr, "Out of bounds... try again\b");
				continue;
			}

			if (m.who == BLACK
				&& c == 'e' && r == 8
				&& C == 'g' && R == 8
				&& b->castle_king[m.who]) {
				m.move_type = MOVE_KINGSIDE_CASTLE;
			} else if (m.who == BLACK
				&& c == 'e' && r == 8
				&& C == 'c' && R == 8
				&& b->castle_queen[m.who]) {
				m.move_type = MOVE_QUEENSIDE_CASTLE;
			} else if (m.who == WHITE
				&& c == 'e' && r == 1
				&& C == 'g' && R == 1
				&& b->castle_king[m.who]) {
				m.move_type = MOVE_KINGSIDE_CASTLE;
			} else if (m.who == WHITE
				&& c == 'e' && r == 1
				&& C == 'c' && R == 1
				&& b->castle_queen[m.who]) {
				m.move_type = MOVE_QUEENSIDE_CASTLE;
			} else {
				m.move_type = MOVE_REGULAR;
				m.r = 8-r;
				m.R = 8-R;
				m.c = c-'a';
				m.C = C-'a';
				m.promote = pieceOf(newpiece);
			}

			if (!doMove(b, m)) {
				fprintf(stderr, "Move is not legal... try again\n");
				printf("illegal move: %c%i%c%i\n", c, r, C, R);
				fprintf(stderr, "illegal move: %c%i%c%i\n", c, r, C, R);
				continue;
			}
		}

		mark(b);

		pp = toPGN(t, m);
		freeGame(t);
		stringPGN(mbuf, pp);
		fprintf(game_log, "%i. %s ", movenum, mbuf);
		if (movenum % 2 == 0)
			fprintf(game_log, "\n");

		movenum++;

		fflush(stdout);
	}

	printf("quit\n");

	freeGame(b);
	fprintf(stderr, "RES: WHAT?");

	return 0;
}

int main(int argc, char **argv) {
	signal(SIGSEGV, handler);

	int rc = main_trucho(argc, argv);

	fprintf(stderr, "Total nodes: %i\n", stats.totalopen);
	fprintf(stderr, "Total unique nodes: %i\n", NN);
	fprintf(stderr, "Total time: %ims\n", stats.totalms);

	printf("program returned %i\n", rc);
	return 0;
}
