#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "board.h"
#include "common.h"
#include "mem.h"

struct bookmove {
	u64 hash;
	move next;
	char strs[25][200];
	int nstrs;
};

struct bookmove book[20000];
int booklen = 0;

void add_one(u64 hash, move m,char *str) {
	book[booklen].hash = hash;
	book[booklen].next = m;
	strcpy(book[booklen].strs[0], str);
	book[booklen].nstrs = 1;
	booklen++;
}

#define cmp_field(a, b)					\
	do {						\
		if ((a) != (b))				\
			return (a) < (b) ? -1 : 1;	\
	} while (0)

int entry_cmp(struct bookmove a, struct bookmove b) {
	cmp_field(a.hash, b.hash);
	cmp_field(a.next.move_type, b.next.move_type);

	if (a.next.move_type != MOVE_REGULAR)
		return 0;

	cmp_field(a.next.r, b.next.r);
	cmp_field(a.next.c, b.next.c);
	cmp_field(a.next.R, b.next.R);
	cmp_field(a.next.C, b.next.C);

	return 0;
}
#undef cmp_field

void sort_book() {
	int i, j;
	struct bookmove sw;

	for (i=1; i<booklen; i++) {
		j = i-1;
		while (j >= 0 && entry_cmp(book[j], book[j+1]) > 0) {
			sw = book[j];
			book[j] = book[j+1];
			book[j+1] = sw;
			j--;
		}
	}
}

void copy_strs(int i1, int i2) {
	int i;

	for (i=0; i<book[i2].nstrs; i++) {
		strcpy(book[i1].strs[book[i1].nstrs], book[i2].strs[i]);
		book[i1].nstrs++;
	}
}

void nodup_book() {
	int i, j;

	i = 1;
	while (i < booklen) {
		if (!entry_cmp(book[i-1], book[i])) {
			copy_strs(i-1, i);
			for (j = i; j < booklen-1; j++)
				book[j] = book[j+1];

			booklen--;
		} else {
			i++;
		}
	}
}

char *movetype_str(int type) {
	switch (type) {
	case MOVE_REGULAR:
		return "MOVE_REGULAR";
	case MOVE_KINGSIDE_CASTLE:
		return "MOVE_KINGSIDE_CASTLE";
	case MOVE_QUEENSIDE_CASTLE:
		return "MOVE_QUEENSIDE_CASTLE";
	}
	assert(0);
	return NULL;
}

void print_book() {
	int i, j;

	printf("static struct book_entry book[] = {\n");
	for (i=0; i<booklen; i++) {
		move m = book[i].next;
		u64 hash = book[i].hash;

		printf("	{\n");
		printf("	/*\n");
		printf("	 * Entry for sequences:\n");
		for (j=0; j<book[i].nstrs; j++)
			printf("	 * [%s]\n", book[i].strs[j]);
		printf("	 */\n");
		printf("		.hash = 0x%.16" PRIx64 ",\n", hash);
		printf("		.move_type = %s,\n", movetype_str(m.move_type));
		if (m.move_type == MOVE_REGULAR) {
			printf("		.r = %i, .c = %i, .R = %i, .C = %i\n",
					m.r, m.c, m.R, m.C);
		}
		printf("	},\n");

	}
	printf("};\n");
	printf("\n");
	printf("int booklen = %i;\n", booklen);
}


void add_rule(char *sequence) {
	game g = startingGame();
	int turn = WHITE;
	char *seq_orig = sequence;
	char c, C;
	int r, R;
	move m;
	u64 hash = 0;
	__unused int ok = 0;

	G = g;
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

		hash = g->zobrist;
		__unused bool rc = doMove(m);
		assert(rc);
		ok = 1;

		turn = flipTurn(turn);
		sequence = strchr(sequence+1, ' ');
	}

	assert(ok);
	add_one(hash, m, seq_orig);

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

		if (i > 0)
			add_rule(buf);
	}

	sort_book();
	nodup_book();
	print_book();

	return 0;
}
