#include "user_input.h"
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

void printMove(FILE *stream, move m) {
	if (m.move_type == MOVE_KINGSIDE_CASTLE) {
		if (m.who == BLACK) {
			fprintf(stream, "move e8g8\n");
			dbg("move e8g8\n");
		} else {
			fprintf(stream, "move e1g1\n");
			dbg("move e1g1\n");
		}
	} else if (m.move_type == MOVE_QUEENSIDE_CASTLE) {
		if (m.who == BLACK) {
			fprintf(stream, "move e8c8\n");
			dbg("move e8c8\n");
		} else {
			fprintf(stream, "move e1c1\n");
			dbg("move e1c1\n");
		}
	} else {
		assert(m.move_type == MOVE_REGULAR);

		if (m.promote != 0) {
			fprintf(stream, "move %c%c%c%c%c\n",
					m.c + 'a',
					'8'-m.r,
					m.C + 'a',
					'8'-m.R,
					tolower(charOf(m.promote)));
			dbg("move %c%c%c%c%c\n",
					m.c + 'a',
					'8'-m.r,
					m.C + 'a',
					'8'-m.R,
					tolower(charOf(m.promote)));
		} else {
			fprintf(stream, "move %c%c%c%c\n",
					m.c + 'a',
					'8'-m.r,
					m.C + 'a',
					'8'-m.R);
			dbg("move %c%c%c%c\n", m.c + 'a',
					'8'-m.r,
					m.C + 'a',
					'8'-m.R);
		}
	}
}

move parseMove(char *line) {
	int t;
	char c, C;
	int r, R;
	char newpiece;
	move m;

	if (5 != (t=sscanf(line, "%c%i%c%i%c", &c, &r, &C, &R, &newpiece))
			&& (newpiece = 0) /* muuuuuuy chanta */
			&& (4 != (t=sscanf(line, "%c%i%c%i", &c, &r, &C, &R)))) {
		m.move_type = MOVE_INVAL;
		return m;
	}

	m.who = G->turn;

	c = tolower(c);
	C = tolower(C);

	dbg("Your move: %c%i -> %c%i\n", c, r, C, R);

	if (c < 'a' || c > 'h' || C < 'a' || C > 'h' ||
	    r < 0   || r > 8   || R < 0   || R > 8  ) {
		dbg("Out of bounds... try again\b");
		m.move_type = MOVE_INVAL;
		return m;
	}

	if (m.who == BLACK
		&& c == 'e' && r == 8
		&& C == 'g' && R == 8
		&& G->castle_king[m.who]) {
		m.move_type = MOVE_KINGSIDE_CASTLE;
	} else if (m.who == BLACK
		&& c == 'e' && r == 8
		&& C == 'c' && R == 8
		&& G->castle_queen[m.who]) {
		m.move_type = MOVE_QUEENSIDE_CASTLE;
	} else if (m.who == WHITE
		&& c == 'e' && r == 1
		&& C == 'g' && R == 1
		&& G->castle_king[m.who]) {
		m.move_type = MOVE_KINGSIDE_CASTLE;
	} else if (m.who == WHITE
		&& c == 'e' && r == 1
		&& C == 'c' && R == 1
		&& G->castle_queen[m.who]) {
		m.move_type = MOVE_QUEENSIDE_CASTLE;
	} else {
		m.move_type = MOVE_REGULAR;
		m.r = 8-r;
		m.R = 8-R;
		m.c = c-'a';
		m.C = C-'a';
		m.promote = pieceOf(newpiece) & 7;
	}

	return m;
}
