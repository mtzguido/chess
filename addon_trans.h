#ifdef CFG_TRANSPOSITION
#include "board.h"

#define TTABLE_SIZE 131101

struct tt_entry {
	score v;
	game k;
	int d;
};

struct tt_entry ttable[TTABLE_SIZE] = { [0 ... TTABLE_SIZE-1] = { .k = NULL } };

static int gameHash(game g) {
	uint64_t x = 5381;
	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j+=2) {
			unsigned char c = ((g->board[i][j] & 0xF) ^ ((g->board[i][j+1] << 4) & 0xF0));
			x = c + (x*33);
		}

			//x = ((6+g->board[i][j]) + (x*9)) % TTABLE_SIZE;

	x = x % TTABLE_SIZE;

	//fprintf(stderr, "gameHash returns %u\n", x);
	return x;
}

static void trans_init() {
	int i;
	
	for (i=0; i<TTABLE_SIZE; i++) {
		if (ttable[i].k != NULL)
			freeGame(ttable[i].k);

		ttable[i].k = NULL;
	}
}

static void trans_notify_return(game g, score s, int depth) {
	int gh = gameHash(g);

	if (ttable[gh].k != NULL) {
		//fprintf(stderr, "trans: collision at %i\n", curDepth);
		freeGame(ttable[gh].k);
	}

	ttable[gh].k = copyGame(g);
	ttable[gh].v = s;
	ttable[gh].d = depth;
}

static bool trans_notify_entry(game g, int depth, score *ret) {
	int gh = gameHash(g);

	if (ttable[gh].k != NULL
		&& ttable[gh].d == depth
		//&& ttable[gh].d <= depth
		&& equalGame(g, ttable[gh].k)) {
		//fprintf(stderr, "trans: skipping node at %i\n", curDepth);

		*ret = ttable[gh].v;
		return true;
	} else {
		return false;
	}
}

#else

static void trans_init() { }
static void trans_notify_return(game g, score s, int depth) { }
static bool trans_notify_entry(game g, int depth, score *ret) { return false; }

#endif

