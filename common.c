#include "common.h"
#include "board.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <syslog.h>
#include <time.h>

#ifndef FIXOPTS
struct opts copts;
#endif

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

	if (copts.verbosity <= 0)
		return;

	if (copts.syslog) {
		va_start(l, s);
		vsyslog(LOG_DEBUG, s, l);
		va_end(l);
	}

	va_start(l, s);
	fprintf(stderr, ">> ");
	vfprintf(stderr, s, l);
	fflush(stderr);
	va_end(l);
}

unsigned long getms() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

unsigned long getms_clock() {
	clock_t t = clock();

	return t * 1000 / CLOCKS_PER_SEC;
}
