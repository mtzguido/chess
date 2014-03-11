#include "pgn.h"
#include "board.h"
#include "move.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

move fromPGN(game g, struct pgn p) {
	move ret = {0};
	int piece;
	int i, j;

	if (p.king_castle) {
		ret.move_type = MOVE_KINGSIDE_CASTLE;
		return ret;
	}

	if (p.queen_castle) {
		ret.move_type = MOVE_QUEENSIDE_CASTLE;
		return ret;
	}

	p.dest_rank = tolower(p.dest_rank);
	p.orig_rank = tolower(p.orig_rank);
	p.dest_file = tolower(p.dest_file);
	p.orig_file = tolower(p.orig_file);

	switch (p.pieceChar) {
		case '\0': piece = WPAWN;   break;
		case 'Q':  piece = WQUEEN;  break;
		case 'K':  piece = WKING;   break;
		case 'N':  piece = WKNIGHT; break;
		case 'R':  piece = WROOK;   break;
		case 'B':  piece = WBISHOP; break;
		default:		abort();
	}

	if (g->turn == BLACK)
		piece = -piece;


	/* buscar una pieza que pueda moverse al destino */

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g->board[i][j] == piece
					&& canMove(g, i, j, '8' - p.dest_rank, p.dest_file - 'a')
					&& (p.orig_file == '\0' || p.orig_file - 'a' == j)
					&& (p.orig_rank == '\0' || '8' - p.orig_rank == i))
			{
				ret.r = i;
				ret.c = j;
				ret.R = '8' - p.dest_rank;
				ret.C = p.orig_file - 'a';

				if (g->board[ret.R][ret.C] != 0) {
					assert(p.capture);
					ret.was_capture = 1;
				}

				if (isPawn(piece) && ret.R == (g->turn == WHITE ? 0 : 7)) {
					assert(p.promote_to != '\0');

					switch (p.promote_to) {
						case '\0': ret.promote = WPAWN;   break;
						case 'Q':  ret.promote = WQUEEN;  break;
						case 'K':  ret.promote = WKING;   break;
						case 'N':  ret.promote = WKNIGHT; break;
						case 'R':  ret.promote = WROOK;   break;
						case 'B':  ret.promote = WBISHOP; break;
						default:		assert(0);
					}

					if (g->turn == BLACK)
						ret.promote = - ret.promote;

					ret.was_promotion = 1;

					return ret;
				}
			}
		}
	}

	abort();
}

struct pgn toPGN(game g, move m) {
	struct pgn ret = {0};
	int i, j;
	int ambiguous;

	if (m.move_type == MOVE_KINGSIDE_CASTLE) {
		ret.king_castle = 1;
		return ret;
	} 

	if (m.move_type == MOVE_QUEENSIDE_CASTLE) {
		ret.queen_castle = 1;
		return ret;
	} 

	if (isPawn(g->board[m.r][m.c]))
		ret.pieceChar = '\0';
	else
		ret.pieceChar = toupper(charOf(g->board[m.r][m.c]));

	ret.capture = g->board[m.R][m.C] != 0;

	ret.dest_file = m.C + 'a';
	ret.dest_rank = '8' - m.R;

	ambiguous = 0;
	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g->board[i][j] == g->board[m.r][m.c]
					&& canMove(g, i, j, m.R, m.C)
					&& (i != m.r || j != m.c))
				ambiguous =1;
		}
	}

	if (ambiguous) {
		ret.orig_file = m.c + 'a';

		for (i=0; i<8; i++) {
			if (g->board[i][m.c] == g->board[m.r][m.c]
					&& canMove(g, i, m.c, m.R, m.C)
					&& i != m.r)
				ambiguous = 1;
		}
	}

	if (ambiguous)
		ret.orig_rank = '8' - m.r;


	return ret;
}

void stringPGN(char *buf, struct pgn p) {
	int o = 0;

	if (p.king_castle) {
		sprintf(buf, "O-O");
		return;
	} else if (p.queen_castle) {
		sprintf(buf, "O-O-O");
		return;
	}

	if (p.pieceChar != '\0')
		o += sprintf(buf+o, "%c", p.pieceChar);

	if (p.orig_file != 0 && p.orig_rank != 0)
		o += sprintf(buf+o, "%c%c", p.orig_file, p.orig_rank);
	else if (p.orig_file != 0)
		o += sprintf(buf+o, "%c", p.orig_file);

	if (p.capture)
		o += sprintf(buf+o, "x");

	o += sprintf(buf+o, "%c%c", p.dest_file, p.dest_rank);
}

