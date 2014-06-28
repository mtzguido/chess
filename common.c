#include "common.h"
#include "board.h"

struct opts copts;

int isPrefix(char *a, char *b) {
	while (*a && *a++ == *b++)
		;

	if (*a == 0)
		return 1;

	return 0;
}

char pieceOf(char c) {
	switch (c) {
		case 'P':   return WPAWN;
		case 'p':   return WPAWN;
		case 'R':   return WROOK;
		case 'r':   return WROOK;
		case 'N':   return WKNIGHT;
		case 'n':   return WKNIGHT;
		case 'B':   return WBISHOP;
		case 'b':   return WBISHOP;
		case 'Q':   return WQUEEN;
		case 'q':   return WQUEEN;
		case 'k':   return WKING;
		case 'K':   return WKING;
		default:    return EMPTY;
	}   
}

int on_bits(u64 x, u8 *arr) {
	int ret = 0;
	int i = 0;

	while (x) {
		while (!(x&0xffffffff)) {
			x >>= 32;
			i += 32;
		}

		while (!(x&0xff)) {
			x >>= 8;
			i += 8;
		}

		while (!(x&1)) {
			x >>= 1;
			i++;
		}

		arr[ret++] = i;
		x >>= 1;
		i++;
	}

	return ret;
}
