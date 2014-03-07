#include <stdio.h>
#include <assert.h>
#include "ai.h"
#include "board.h"

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

int main () {
	game b = startingGame();
	machineColor = BLACK;

	/*
	   int i, n;
	   game* arr;

	   n = genSuccs(b, &arr);

	   printf("N=%i\n", n);
	   for (i=0; i<n; i++)
	   printBoard(arr[i]);

	   return 0;
	   */

	printBoard(b);

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

		if (b->turn == machineColor) {
			game nb;
			printf("Machine turn:\n");

			nb = machineMove(b);
			freeGame(b);
			b = nb;

			printBoard(b);
			printf("(move was (REGULAR?) %c%i->%c%i)\n", b->lastmove.c+'A', 8-b->lastmove.r, b->lastmove.C+'A', 8-b->lastmove.R);
		} else {
			int r, R;
			char c, C;
			move m;
			char newpiece;
			int t;
			char *line = NULL;
			size_t crap = 0;

			m.who = flipTurn(machineColor);

			printf("Your turn:\n");

			getline(&line, &crap, stdin);
			if (5 != (t=sscanf(line, "%c%i%c%i%c", &c, &r, &C, &R, &newpiece))
					&& (newpiece = 0) /* muuuuuuy chanta */
					&& (4 != (t=sscanf(line, "%c%i%c%i", &c, &r, &C, &R)))) {
				fprintf(stderr, "Could not parse move... try again\n");
				continue;
			}

			if (c=='x') break;
			if (c=='z') {
				m.move_type = r == 1? MOVE_KINGSIDE_CASTLE : MOVE_QUEENSIDE_CASTLE;
				printf("trying to castle\n");
				goto move;
			}

			if (c >= 'a' && c <= 'z')
				c = c - 'a' + 'A';
			if (C >= 'a' && C <= 'z')
				C = C - 'a' + 'A';

			free(line);



			printf("Your move: %c%i -> %c%i\n", c, r, C, R);

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

			printf("promote=%i\n", m.promote);

move:
			if (!isLegalMove(b, m)) {
				fprintf(stderr, "Move is not legal... try again\n");
				continue;
			}

			doMove(b, m);
			printBoard(b);
		}
	}

	freeGame(b);

	return 0;
}

