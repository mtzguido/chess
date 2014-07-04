#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "board.h"
#include "common.h"
#include "mem.h"

struct bookmove {
	u64 hash;
	move next;
};

struct bookmove book[20000];
int booklen = 0;

void add_one(u64 hash, move m) {
	book[booklen].hash = hash;
	book[booklen].next = m;
	booklen++;
}

void sort_book() {
	int i, j;
	struct bookmove sw;

	for (i=1; i<booklen; i++) {
		j = i-1;
		while (j >= 0 && book[j].hash > book[j+1].hash) {
			sw = book[j];
			book[j] = book[j+1];
			book[j+1] = sw;
			j--;
		}
	}
}

void print_book() {
	int i;

	printf("static struct book_entry book[] = {\n");
	for (i=0; i<booklen; i++) {
		move m = book[i].next;
		u64 hash = book[i].hash;

		printf("	[%i] = { .hash = 0x%.16" PRIx64 ",\n", i, hash);
		printf("		.r = %i,\n", m.r);
		printf("		.c = %i,\n", m.c);
		printf("		.R = %i,\n", m.R);
		printf("		.C = %i,\n", m.C);
		printf("		.move_type = %i,\n", m.move_type);
		printf("	},\n");

	}
	printf("};\n");
	printf("\n");
	printf("int booklen = %i;\n", booklen);
}


void add_rule(char *sequence) {
	game g = startingGame();
	int turn = WHITE;
	char c, C;
	int r, R;
	move m;

	while (sequence &&
		4 == sscanf(sequence, " %c%i%c%i ", &c, &r, &C, &R)) {
		m.move_type = MOVE_REGULAR;
		m.c = toupper(c) - 'A';
		m.r = 8 - r;
		m.C = toupper(C) - 'A';
		m.R = 8 - R;
		m.promote = 0;
		m.who = turn;

		if (m.r == 7 && m.c == 4 && m.R == 7 && m.C == 6
				&& g->turn == WHITE && g->castle_king[WHITE]) {
			m.move_type = MOVE_KINGSIDE_CASTLE;
		} else if (m.r == 7 && m.c == 4 && m.R == 7 && m.C == 2
				&& g->turn == WHITE && g->castle_queen[WHITE]) {
			m.move_type = MOVE_QUEENSIDE_CASTLE;
		} else if (m.r == 0 && m.c == 4 && m.R == 0 && m.C == 6
				&& g->turn == BLACK && g->castle_king[BLACK]) {
			m.move_type = MOVE_KINGSIDE_CASTLE;
		} else if (m.r == 0 && m.c == 4 && m.R == 0 && m.C == 2
				&& g->turn == BLACK && g->castle_queen[BLACK]) {
			m.move_type = MOVE_QUEENSIDE_CASTLE;
		}

		add_one(g->zobrist, m);
		bool rc __maybe_unused = doMove(g, m);
		assert(rc);

		turn = flipTurn(turn);
		sequence = strchr(sequence+1, ' ');
	}

	freeGame(g);
}

int main () {
	int line = 0;
	char buf[2000];
	int i, c=0;

	init_mem();

	while (c != EOF) {
		line++;

		i = 0;
		while ((c=getchar()) != '\n' && c != EOF)
			buf[i++] = c;
		buf[i] = 0;

		add_rule(buf);
	}

	sort_book();
	print_book();

	return 0;
}
