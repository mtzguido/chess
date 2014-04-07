#include "board.h"
#include "ztable.h"

#ifdef CFG_TRANSPOSITION

static void trans_init() { }
static void trans_notify_cut(game g, game next, int depth) { }
static void trans_sort(game g, game *succs, int nsucc, int depth) { }

#else

static void trans_init() { }
static void trans_notify_cut(game g, game next, int depth) { }
static void trans_sort(game g, game *succs, int nsucc, int depth) { }

#endif

