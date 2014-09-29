#include "autoversion.h"
#include <stdio.h>

void print_version() {
	fprintf(stderr, "ICE version %s\n", CHESS_VERSION);
	fprintf(stderr, "built on %s at %s\n", CHESS_BUILD_DATE,
			CHESS_BUILD_HOST);
}

static const char *shortv = "ICE " CHESS_VERSION;
const char *short_version() {
	return shortv;
}
