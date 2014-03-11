#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "ai.h"
#include "board.h"
#include "pgn.h"

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

int main (int argc, char **argv) {
	game b = startingGame();

	if (argc > 1 && argv[1][0] == 'w')
		machineColor = WHITE;
	else
		machineColor = BLACK;

	int movenum = 1;

	FILE *game_log = fopen(machineColor == WHITE ? "gamelog_w" : "gamelog_b", "w");

	move m;
	char mbuf[500];
	struct pgn pp;

	/*
	   int i, j, n;
	   game *arr;
	   game *arr2;
	   int n2;

	   n = genSuccs(b, &arr);

	   printf("N=%i\n", n);
	   for (i=0; i<n; i++) {
		   printBoard(arr[i]);
		   if (isFinished(arr[i]) > 0 ) {
			   printf("!!!!!!!\n");
			   printf("!!!!!!!\n");
			   printf("!!!!!!!\n");
			   printf("!!!!!!!\n");
		   }
		   getchar();
	   }

	   return 0;
   */

	while(1) {
		int rc = isFinished(b);
		if (rc > 0) {
			if (rc == WIN(WHITE))
				printf("WHITE WON!\n");
			else if (rc == WIN(BLACK))
				printf("BLACK WON!\n");
			else if (rc == DRAW)
				printf("It's a draw!!\n");
			else
				assert(0);

			break;
		}

		game t = copyGame(b);
		printBoard(b);

		if (b->turn == machineColor) {
			game nb;

			/// printf("...\n");

			nb = machineMove(b);
			freeGame(b);
			b = nb;

			m = nb->lastmove;
			if (nb->lastmove.move_type == MOVE_REGULAR)
				printf("%c%c%c%c\n",
						nb->lastmove.c + 'a',
						'8'-nb->lastmove.r,
						nb->lastmove.C + 'a',
						'8'-nb->lastmove.R);
			else if (nb->lastmove.move_type == MOVE_KINGSIDE_CASTLE)
				//printf("z1z1\n");
				printf("e1g1\n");
			else
				//printf("z2z2\n");
				printf("e1c1\n");

		} else {
			int r, R;
			char c, C;
			char newpiece;
			int t;
			char *line = NULL;
			size_t crap = 0;

			m.who = flipTurn(machineColor);

//			printf("Your turn:\n");

			getline(&line, &crap, stdin);
			if (5 != (t=sscanf(line, "%c%i%c%i%c", &c, &r, &C, &R, &newpiece))
					&& (newpiece = 0) /* muuuuuuy chanta */
					&& (4 != (t=sscanf(line, "%c%i%c%i", &c, &r, &C, &R)))) {
				fprintf(stderr, "Could not parse move... try again\n");
				continue;
			}

			if (strcmp(line, "e8g8") == 0 && b->castle_king[m.who]) {
				m.move_type = MOVE_KINGSIDE_CASTLE;
				goto move;
			}
			if (strcmp(line, "e8c8") == 0 && b->castle_queen[m.who]) {
				m.move_type = MOVE_QUEENSIDE_CASTLE;
				goto move;
			}

			if (c=='x') break;
			if (c=='z') {
				m.move_type = r == 1? MOVE_KINGSIDE_CASTLE : MOVE_QUEENSIDE_CASTLE;
				// printf("trying to castle\n");
				goto move;
			}

			if (c >= 'a' && c <= 'z')
				c = c - 'a' + 'A';
			if (C >= 'a' && C <= 'z')
				C = C - 'a' + 'A';

			free(line);

			fprintf(stderr, "Your move: %c%i -> %c%i\n", c, r, C, R);


			if (c < 'A' || c > 'H' || C < 'A' || C > 'H' ||
					r < 0   || r > 8   || R < 0   || R > 8  ) {
				fprintf(stderr, "Out of bounds... try again\b");
				continue;
			}

			m.move_type = MOVE_REGULAR;
			m.r = 8-r;
			m.R = 8-R;
			m.c = c-'A';
			m.C = C-'A';
			m.promote = pieceOf(newpiece);

move:
			if (!doMove(b, m)) {
				fprintf(stderr, "Move is not legal... try again\n");
				continue;
			}
		}

		pp = toPGN(t, m);
		freeGame(t);
		stringPGN(mbuf, pp);
		fprintf(game_log, "%i. %s ", movenum, mbuf);
		if (movenum % 2 == 0)
			fprintf(game_log, "\n");

		movenum++;

		fflush(stdout);
	}

	freeGame(b);

	return 0;
}

