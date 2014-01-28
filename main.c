#include <stdio.h>
#include "board.h"
#include "ai.h"
#include "game.h"

int machineColor;

int main () {
	int turn;

	game b = startingStatus();
	printBoard(b);
	machineColor = BLACK;

	turn = WHITE;

	while(1) {
		if (turn == machineColor) {
			printf("Machine turn:\n");
			b = machineMove(b);
			printBoard(b);

			turn = turn == BLACK ? WHITE : BLACK;
		} else {
			char r1,r2;
			int c1,c2;
			int t;
			printf("Your turn:\n");
			
			if (4 != (t=scanf("%c%i%c%i", &r1, &c1, &r2, &c2))) {
				fprintf(stderr, "Could not parse move... try again\n");
				continue;
			}

			if (r1 >= 'a' && r1 <= 'z')
				r1 = r1 - 'a' + 'A';
			if (r2 >= 'a' && r2 <= 'z')
				r2 = r2 - 'a' + 'A';

			if (r1 < 'A' || r1 > 'H' || r2 < 'A' || r2 > 'H' ||
					c1 < 0 || c1 > 8 || c2 < 0 || c2 > 8) {
				fprintf(stderr, "Out of bounds... try again\b");
				continue;
			}

			if (!isLegalMove(b, r1-'A', c1-1, r2-'A', c2-1)) {
				fprintf(stderr, "Move is not legal... try again\n");
				continue;
			}

			b.board[r2-'A'][c2-1] = b.board[r1-'A'][c1-1];
			b.board[r1-'A'][c1-1] = 0;

			printBoard(b);
	
			turn = turn == BLACK ? WHITE : BLACK;
		}
	}


	return 0;
}

