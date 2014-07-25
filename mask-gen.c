#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

char *name;
uint64_t mask[64];

void print_pre() {
	printf("/* Auto-generated file, do not edit */\n");
	printf("\n");
	printf("#include \"common.h\"\n");
	printf("\n");
}

void print() {
	int i;

	printf("const u64 %s_mask[64] = {\n", name);

	for (i=0; i<64; i++)
		printf("\t0x%.16" PRIx64 ",\n", mask[i]);

	printf("};\n");
	printf("\n");
}

void print_post() {
	printf("/* Auto-generated file, do not edit */\n");
}

uint64_t bit(int i) {
	return ((uint64_t)1) << i;
}

typedef bool (*genfun_t)(int r, int c, int R, int C);

void genMask(genfun_t f) {
	int r, c, R, C;

	for (r=0; r<8; r++) {
	for (c=0; c<8; c++) {
	for (R=0; R<8; R++) {
	for (C=0; C<8; C++) {
		if (f(r, c, R, C))
			mask[r*8+c] |= bit(R*8 + C);
		else
			mask[r*8+c] &= ~bit(R*8 + C);
	}
	}
	}
	}
}

bool knight(int r, int c, int R, int C) {
	name = "knight";

	return (abs(r-R) + abs(C-c) == 3) && r != R;
}

bool king(int r, int c, int R, int C) {
	name = "king";

	return abs(R-r) <= 1 && abs(C-c) <= 1 &&
		(r != R || c != C);
}

bool row_w(int r, int c, int R, int C) {
	name = "row_w";

	return (r == R) && (C < c);
}

bool row_e(int r, int c, int R, int C) {
	name = "row_e";

	return (r == R) && (C > c);
}

bool col_n(int r, int c, int R, int C) {
	name = "col_n";

	return (c == C) && (R < r);
}

bool col_s(int r, int c, int R, int C) {
	name = "col_s";

	return (c == C) && (R > r);
}

bool diag_ne(int r, int c, int R, int C) {
	name = "diag_ne";

	return (r+c == R+C) && (R < r);
}

bool diag_sw(int r, int c, int R, int C) {
	name = "diag_sw";

	return (r+c == R+C) && (R > r);
}

bool diag_se(int r, int c, int R, int C) {
	name = "diag_se";

	return (r-c == R-C) && (R > r);
}

bool diag_nw(int r, int c, int R, int C) {
	name = "diag_nw";

	return (r-c == R-C) && (R < r);
}

bool all(int r, int c, int R, int C) {
	bool t;

	t = diag_nw(r,c,R,C)
		|| diag_sw(r,c,R,C)
		|| diag_se(r,c,R,C)
		|| diag_ne(r,c,R,C)
		|| col_n(r,c,R,C)
		|| col_s(r,c,R,C)
		|| row_w(r,c,R,C)
		|| row_e(r,c,R,C)
		|| knight(r,c,R,C);

	/* ugh...*/
	name = "all";

	return t;
}

int main () {
	genfun_t genFuncs[] = {
		knight,
		king,
		row_w,
		row_e,
		col_n,
		col_s,
		diag_nw,
		diag_sw,
		diag_ne,
		diag_se,
		all,
	};
	unsigned i;

	print_pre();
	for (i=0; i<sizeof genFuncs / sizeof genFuncs[0]; i++) {
		genMask(genFuncs[i]);
		print();
	}
	print_post();

	return 0;
}
