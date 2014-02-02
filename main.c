#include <stdio.h>
#include "ai.h"
#include "board.h"
#include "moves.h"

int machineColor;

int main () {
	game b = startingStatus();
	printBoard(b);
	machineColor = BLACK;

	while(1) {
		if (isFinished(b) != -1) {
			if (isFinished(b) == WHITE)
				printf("WHITE WON!\n");
			else 
				printf("BLACK WON!\n");

			break;
		}

		if (b.turn == machineColor) {
			printf("Machine turn:\n");
			b = machineMove(b);
			printBoard(b);
			printf("(move was %c%i->%c%i)\n", b.lastmove.c+'A', 8-b.lastmove.r, b.lastmove.C+'A', 8-b.lastmove.R);
		} else {
			int r, R;
			char c, C;
			int t;
			printf("Your turn:\n");
			
			if (4 != (t=scanf("%c%i%c%i", &c, &r, &C, &R))) {
				fprintf(stderr, "Could not parse move... try again\n");
				continue;
			}

			if (c >= 'a' && c <= 'z')
				c = c - 'a' + 'A';
			if (C >= 'a' && C <= 'z')
				C = C - 'a' + 'A';

			printf("Your move: %c%i -> %c%i\n", c, r, C, R);

			if (c < 'A' || c > 'H' || C < 'A' || C > 'H' ||
					r < 0 || r > 8 || R < 0 || R > 8) {
				fprintf(stderr, "Out of bounds... try again\b");
				continue;
			}

			if (!isLegalMove(b, 8-r, c-'A', 8-R, C-'A')) {
				fprintf(stderr, "Move is not legal... try again\n");
				continue;
			}

			b = doMove(b, 8-r, c-'A', 8-R, C-'A');
			//b.board[8-r][c-'A'] = b.board[8-R][C-'A'];
			//b.board[8-R][C-'A'] = 0;

			printBoard(b);
	
		}
	}


	return 0;
}

