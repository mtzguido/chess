#ifndef __ADDON_LIST_H__
#define __ADDON_LIST_H__

#include "addon_cm.h"
#include "addon_killer.h"
#include "addon_trans.h"
#include "addon_trivial.h"

static inline void start_all_addons() {
	static int done;

	if (done)
		return;

	done = 1;

	if (copts.heur_trans)
		addon_trans_init();

	if (copts.heur_killer)
		addon_killer_init();

	if (copts.heur_cm)
		addon_cm_init();

	if (copts.heur_trivial)
		addon_trivial_init();
}

#endif
