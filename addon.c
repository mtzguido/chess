#include "addon_trans.h"
#include "addon_cm.h"
#include "addon_killer.h"
#include "addon_trivial.h"

#include "addon.h"
#include "ai.h"
#include "board.h"
#include <assert.h>


#define addon_for(name, fun, ...)	\
	name##_##fun(__VA_ARGS__)

#define addon_for_each(fun, ...)			\
	do {						\
		addon_for(trans, fun, __VA_ARGS__);	\
		addon_for(killer, fun, __VA_ARGS__);	\
		addon_for(cm, fun, __VA_ARGS__);	\
		addon_for(trivial, fun, __VA_ARGS__);	\
	} while (0)

void addon_reset() {
	addon_for_each(reset);
}

void addon_notify_return(game g, move m, int depth, score score, flag_t flag) {
	addon_for_each(notify_return, g, m, depth, score, flag);
}

void addon_notify_entry(game g, int depth, score *alpha, score *beta) {
	addon_for_each(notify_entry, g, depth, alpha, beta);
}

void addon_notify_cut(game g, move m, int depth) {
	addon_for_each(notify_cut, g, m, depth);
}

void addon_score_succs(game g, int depth) {
	addon_for_each(score_succs, g, depth);
}

int addon_suggest(game g, move **arr, int depth) {
	return 0;
}

void addon_free_mem() {
}
