#include "autoversion.h"
#include <stdio.h>

void print_version() {
	fprintf(stderr, "chess version %s\n", CHESS_VERSION);
	fprintf(stderr, "built on %s at %s\n", CHESS_BUILD_DATE,
			CHESS_BUILD_HOST);
}

static const char *shortv = "ICE 2014 " CHESS_VERSION;
const char *short_version() {
	return shortv;
}
