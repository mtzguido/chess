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

int on_bits(u64 x, u8 *arr) {
	int ret = 0;

	if (x & (B(0) | B(1) | B(2) | B(3))) {
		if (x & B(0)) {
			if (x & b(0)) arr[ret++] = 0;
			if (x & b(1)) arr[ret++] = 1;
			if (x & b(2)) arr[ret++] = 2;
			if (x & b(3)) arr[ret++] = 3;
			if (x & b(4)) arr[ret++] = 4;
			if (x & b(5)) arr[ret++] = 5;
			if (x & b(6)) arr[ret++] = 6;
			if (x & b(7)) arr[ret++] = 7;
		}
		if (x & B(1)) {
			if (x & b(8)) arr[ret++] = 8;
			if (x & b(9)) arr[ret++] = 9;
			if (x & b(10)) arr[ret++] = 10;
			if (x & b(11)) arr[ret++] = 11;
			if (x & b(12)) arr[ret++] = 12;
			if (x & b(13)) arr[ret++] = 13;
			if (x & b(14)) arr[ret++] = 14;
			if (x & b(15)) arr[ret++] = 15;
		}
		if (x & B(2)) {
			if (x & b(16)) arr[ret++] = 16;
			if (x & b(17)) arr[ret++] = 17;
			if (x & b(18)) arr[ret++] = 18;
			if (x & b(19)) arr[ret++] = 19;
			if (x & b(20)) arr[ret++] = 20;
			if (x & b(21)) arr[ret++] = 21;
			if (x & b(22)) arr[ret++] = 22;
			if (x & b(23)) arr[ret++] = 23;
		}
		if (x & B(3)) {
			if (x & b(24)) arr[ret++] = 24;
			if (x & b(25)) arr[ret++] = 25;
			if (x & b(26)) arr[ret++] = 26;
			if (x & b(27)) arr[ret++] = 27;
			if (x & b(28)) arr[ret++] = 28;
			if (x & b(29)) arr[ret++] = 29;
			if (x & b(30)) arr[ret++] = 30;
			if (x & b(31)) arr[ret++] = 31;
		}
	}
	if (x & (B(4) | B(5) | B(6) | B(7))) {
		if (x & B(4)) {
			if (x & b(32)) arr[ret++] = 32;
			if (x & b(33)) arr[ret++] = 33;
			if (x & b(34)) arr[ret++] = 34;
			if (x & b(35)) arr[ret++] = 35;
			if (x & b(36)) arr[ret++] = 36;
			if (x & b(37)) arr[ret++] = 37;
			if (x & b(38)) arr[ret++] = 38;
			if (x & b(39)) arr[ret++] = 39;
		}
		if (x & B(5)) {
			if (x & b(40)) arr[ret++] = 40;
			if (x & b(41)) arr[ret++] = 41;
			if (x & b(42)) arr[ret++] = 42;
			if (x & b(43)) arr[ret++] = 43;
			if (x & b(44)) arr[ret++] = 44;
			if (x & b(45)) arr[ret++] = 45;
			if (x & b(46)) arr[ret++] = 46;
			if (x & b(47)) arr[ret++] = 47;
		}
		if (x & B(6)) {
			if (x & b(48)) arr[ret++] = 48;
			if (x & b(49)) arr[ret++] = 49;
			if (x & b(50)) arr[ret++] = 50;
			if (x & b(51)) arr[ret++] = 51;
			if (x & b(52)) arr[ret++] = 52;
			if (x & b(53)) arr[ret++] = 53;
			if (x & b(54)) arr[ret++] = 54;
			if (x & b(55)) arr[ret++] = 55;
		}
		if (x & B(7)) {
			if (x & b(56)) arr[ret++] = 56;
			if (x & b(57)) arr[ret++] = 57;
			if (x & b(58)) arr[ret++] = 58;
			if (x & b(59)) arr[ret++] = 59;
			if (x & b(60)) arr[ret++] = 60;
			if (x & b(61)) arr[ret++] = 61;
			if (x & b(62)) arr[ret++] = 62;
			if (x & b(63)) arr[ret++] = 63;
		}
	}

	return ret;
}
