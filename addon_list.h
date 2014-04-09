#ifndef __ADDON_LIST_H__
#define __ADDON_LIST_H__

#include "addon_cm.h"
#include "addon_killer.h"
#include "addon_trans.h"

static inline void start_all_addons() {
	addon_trans_init();
	addon_killer_init();
	addon_cm_init();
}

#endif
