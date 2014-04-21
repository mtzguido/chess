#include "addon.h"
#include "addon_trivial.h"

#include <stdio.h>

static void trivial_score_succs(game g __maybe_unused,
				const move *succs, score *vals,
				int nsucc, int depth) {
	int i;

	for (i=0; i<nsucc; i++) {
		/* Captures? */

		if (succs[i].promote != 0)
			vals[i] += 1;
	}

}
		
static struct addon trivial_addon __maybe_unused =
{
	.score_succs = trivial_score_succs,
};

void addon_trivial_init() {
#ifdef CFG_TRIVIAL
	addon_register(trivial_addon);
#endif
}
