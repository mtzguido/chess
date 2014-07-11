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
		case 'P':	return WPAWN;
		case 'p':	return WPAWN;
		case 'R':	return WROOK;
		case 'r':	return WROOK;
		case 'N':	return WKNIGHT;
		case 'n':	return WKNIGHT;
		case 'B':	return WBISHOP;
		case 'b':	return WBISHOP;
		case 'Q':	return WQUEEN;
		case 'q':	return WQUEEN;
		case 'k':	return WKING;
		case 'K':	return WKING;
		default:	return EMPTY;
	}
}

#define B(n)	(((u64)0xff)<<((8*n)))
#define b(n)	(((u64)1<<(n)))

int on_bits(u64 x, u8 *rows, u8 *cols) {
	int ret = 0;

	if (x & (B(0) | B(1) | B(2) | B(3))) {
		if (x & B(0)) {
			if (x & b(0))  { rows[ret] = 0; cols[ret++] = 0; }
			if (x & b(1))  { rows[ret] = 0; cols[ret++] = 1; }
			if (x & b(2))  { rows[ret] = 0; cols[ret++] = 2; }
			if (x & b(3))  { rows[ret] = 0; cols[ret++] = 3; }
			if (x & b(4))  { rows[ret] = 0; cols[ret++] = 4; }
			if (x & b(5))  { rows[ret] = 0; cols[ret++] = 5; }
			if (x & b(6))  { rows[ret] = 0; cols[ret++] = 6; }
			if (x & b(7))  { rows[ret] = 0; cols[ret++] = 7; }
		}
		if (x & B(1)) {
			if (x & b(8))  { rows[ret] = 1; cols[ret++] = 0; }
			if (x & b(9))  { rows[ret] = 1; cols[ret++] = 1; }
			if (x & b(10)) { rows[ret] = 1; cols[ret++] = 2; }
			if (x & b(11)) { rows[ret] = 1; cols[ret++] = 3; }
			if (x & b(12)) { rows[ret] = 1; cols[ret++] = 4; }
			if (x & b(13)) { rows[ret] = 1; cols[ret++] = 5; }
			if (x & b(14)) { rows[ret] = 1; cols[ret++] = 6; }
			if (x & b(15)) { rows[ret] = 1; cols[ret++] = 7; }
		}
		if (x & B(2)) {
			if (x & b(16)) { rows[ret] = 2; cols[ret++] = 0; }
			if (x & b(17)) { rows[ret] = 2; cols[ret++] = 1; }
			if (x & b(18)) { rows[ret] = 2; cols[ret++] = 2; }
			if (x & b(19)) { rows[ret] = 2; cols[ret++] = 3; }
			if (x & b(20)) { rows[ret] = 2; cols[ret++] = 4; }
			if (x & b(21)) { rows[ret] = 2; cols[ret++] = 5; }
			if (x & b(22)) { rows[ret] = 2; cols[ret++] = 6; }
			if (x & b(23)) { rows[ret] = 2; cols[ret++] = 7; }
		}
		if (x & B(3)) {
			if (x & b(24)) { rows[ret] = 3; cols[ret++] = 0; }
			if (x & b(25)) { rows[ret] = 3; cols[ret++] = 1; }
			if (x & b(26)) { rows[ret] = 3; cols[ret++] = 2; }
			if (x & b(27)) { rows[ret] = 3; cols[ret++] = 3; }
			if (x & b(28)) { rows[ret] = 3; cols[ret++] = 4; }
			if (x & b(29)) { rows[ret] = 3; cols[ret++] = 5; }
			if (x & b(30)) { rows[ret] = 3; cols[ret++] = 6; }
			if (x & b(31)) { rows[ret] = 3; cols[ret++] = 7; }
		}
	}
	if (x & (B(4) | B(5) | B(6) | B(7))) {
		if (x & B(4)) {
			if (x & b(32)) { rows[ret] = 4; cols[ret++] = 0; }
			if (x & b(33)) { rows[ret] = 4; cols[ret++] = 1; }
			if (x & b(34)) { rows[ret] = 4; cols[ret++] = 2; }
			if (x & b(35)) { rows[ret] = 4; cols[ret++] = 3; }
			if (x & b(36)) { rows[ret] = 4; cols[ret++] = 4; }
			if (x & b(37)) { rows[ret] = 4; cols[ret++] = 5; }
			if (x & b(38)) { rows[ret] = 4; cols[ret++] = 6; }
			if (x & b(39)) { rows[ret] = 4; cols[ret++] = 7; }
		}
		if (x & B(5)) {
			if (x & b(40)) { rows[ret] = 5; cols[ret++] = 0; }
			if (x & b(41)) { rows[ret] = 5; cols[ret++] = 1; }
			if (x & b(42)) { rows[ret] = 5; cols[ret++] = 2; }
			if (x & b(43)) { rows[ret] = 5; cols[ret++] = 3; }
			if (x & b(44)) { rows[ret] = 5; cols[ret++] = 4; }
			if (x & b(45)) { rows[ret] = 5; cols[ret++] = 5; }
			if (x & b(46)) { rows[ret] = 5; cols[ret++] = 6; }
			if (x & b(47)) { rows[ret] = 5; cols[ret++] = 7; }
		}
		if (x & B(6)) {
			if (x & b(48)) { rows[ret] = 6; cols[ret++] = 0; }
			if (x & b(49)) { rows[ret] = 6; cols[ret++] = 1; }
			if (x & b(50)) { rows[ret] = 6; cols[ret++] = 2; }
			if (x & b(51)) { rows[ret] = 6; cols[ret++] = 3; }
			if (x & b(52)) { rows[ret] = 6; cols[ret++] = 4; }
			if (x & b(53)) { rows[ret] = 6; cols[ret++] = 5; }
			if (x & b(54)) { rows[ret] = 6; cols[ret++] = 6; }
			if (x & b(55)) { rows[ret] = 6; cols[ret++] = 7; }
		}
		if (x & B(7)) {
			if (x & b(56)) { rows[ret] = 7; cols[ret++] = 0; }
			if (x & b(57)) { rows[ret] = 7; cols[ret++] = 1; }
			if (x & b(58)) { rows[ret] = 7; cols[ret++] = 2; }
			if (x & b(59)) { rows[ret] = 7; cols[ret++] = 3; }
			if (x & b(60)) { rows[ret] = 7; cols[ret++] = 4; }
			if (x & b(61)) { rows[ret] = 7; cols[ret++] = 5; }
			if (x & b(62)) { rows[ret] = 7; cols[ret++] = 6; }
			if (x & b(63)) { rows[ret] = 7; cols[ret++] = 7; }
		}
	}

	/* A lo sumo hay 32 piezas en el tablero */
	assert(ret <= 32);
	return ret;
}
