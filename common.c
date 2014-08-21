#include "common.h"
#include "board.h"
#include <stdarg.h>
#include <stdio.h>

struct opts copts;

unsigned seed;

int isPrefix(char *a, char *b) {
	while (*a && *a++ == *b++)
		;

	if (*a == 0)
		return 1;

	return 0;
}

char pieceOf(char c) {
	switch (c) {
		case 'P':	return WPAWN;
		case 'R':	return WROOK;
		case 'N':	return WKNIGHT;
		case 'B':	return WBISHOP;
		case 'Q':	return WQUEEN;
		case 'K':	return WKING;
		case 'p':	return BPAWN;
		case 'r':	return BROOK;
		case 'n':	return BKNIGHT;
		case 'b':	return BBISHOP;
		case 'q':	return BQUEEN;
		case 'k':	return BKING;
		default:	return EMPTY;
	}
}

void dbg(char *s, ...) {
	va_list l;

	if (!copts.debug)
		return;

	va_start(l, s);
	fprintf(stderr, ">> ");
	vfprintf(stderr, s, l);
	fflush(stderr);
	va_end(l);
}
