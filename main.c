#include <stdio.h>
#include <assert.h>
#include "ai.h"
#include "board.h"

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
		if (isFinished(b) > 0) {
			if (isFinished(b) == WIN(WHITE))
				printf("WHITE WON!\n");
			else 
				printf("BLACK WON!\n");

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
			int t;
			char *line = NULL;
			size_t crap = 0;

			m.who = WHITE;


			printf("Your turn:\n");
			
			getline(&line, &crap, stdin);
			if (4 != (t=sscanf(line, "%c%i%c%i", &c, &r, &C, &R))) {
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

move:
			if (!isLegalMove(b, m)) {
				fprintf(stderr, "Move is not legal... try again\n");
				continue;
			}

			assert(doMove(b, m) == 0);
			printBoard(b);
		}
	}

	freeGame(b);

	return 0;
}

