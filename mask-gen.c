#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

uint64_t mask[64];

void print_pre() {
	printf("/* Auto-generated file, do not edit */\n");
	printf("\n");
	printf("#include \"common.h\"\n");
	printf("\n");
}

void print(char *name) {
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
#ifdef FLIPBIT
	return ((uint64_t)1) << (63 - i);
#else
	return ((uint64_t)1) << i;
#endif
}

typedef bool (*genfun_t)(int r, int c, int R, int C);

void genMask(genfun_t f) {
	int i, j;

	for (i = 0; i < 64; i++) {
		for (j = 0; j < 64; j++) {
			if (f(i/8, i%8, j/8, j%8))
				mask[i] |= bit(j);
			else
				mask[i] &= ~bit(j);
		}
	}
}

bool knight(int r, int c, int R, int C) {
	return (abs(r-R) + abs(C-c) == 3) && r != R;
}

bool king(int r, int c, int R, int C) {
	return abs(R-r) <= 1 && abs(C-c) <= 1 &&
		(r != R || c != C);
}

bool row_w(int r, int c, int R, int C) {
	return (r == R) && (C < c);
}

bool row_e(int r, int c, int R, int C) {
	return (r == R) && (C > c);
}

bool col_n(int r, int c, int R, int C) {
	return (c == C) && (R < r);
}

bool col_s(int r, int c, int R, int C) {
	return (c == C) && (R > r);
}

bool diag_ne(int r, int c, int R, int C) {
	return (r+c == R+C) && (R < r);
}

bool diag_sw(int r, int c, int R, int C) {
	return (r+c == R+C) && (R > r);
}

bool diag_se(int r, int c, int R, int C) {
	return (r-c == R-C) && (R > r);
}

bool diag_nw(int r, int c, int R, int C) {
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

	return t;
}
struct gen_info {
	genfun_t f;
	char *name;
};

static struct gen_info genFuncs[] = {
	{ knight,	"knight"	},
	{ king,		"king"		},
	{ row_w,	"row_w"		},
	{ row_e,	"row_e"		},
	{ col_n,	"col_n"		},
	{ col_s,	"col_s"		},
	{ diag_nw,	"diag_nw",	},
	{ diag_sw,	"diag_sw",	},
	{ diag_ne,	"diag_ne",	},
	{ diag_se,	"diag_se",	},
	{ all,		"all"		},
};

int main () {
	unsigned i;

	print_pre();
	for (i=0; i<sizeof genFuncs / sizeof genFuncs[0]; i++) {
		genMask(genFuncs[i].f);
		print(genFuncs[i].name);
	}
	print_post();

	return 0;
}
