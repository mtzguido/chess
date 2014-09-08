#ifndef __PGN_H
#define __PGN_H

#include "board.h"
#include <stdbool.h>

struct pgn {
	char pieceChar;
	bool capture;

	char dest_file;
	char dest_rank;

	/* '\0' if not needed */
	char orig_file;
	char orig_rank;

	bool king_castle;
	bool queen_castle;

	/* '\0' if not a promotion */
	char promote_to;

	/*
	 * '\0' : none
	 * + : check
	 * # : checkmate
	 */
	char check_or_checkmate;
};

move fromPGN(struct pgn p);
struct pgn toPGN(move m);

void stringPGN(char *buf, struct pgn p);

#endif
