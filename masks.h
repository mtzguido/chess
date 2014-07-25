#ifndef __MASKS_H
#define __MASKS_H

#include "common.h"

/* Geometry masks */
extern const u64 row_w_mask[64];
extern const u64 row_e_mask[64];
extern const u64 col_n_mask[64];
extern const u64 col_s_mask[64];
extern const u64 diag_sw_mask[64];
extern const u64 diag_se_mask[64];
extern const u64 diag_nw_mask[64];
extern const u64 diag_ne_mask[64];

/* Piece masks */
extern const u64 knight_mask[64];
extern const u64 king_mask[64];

/* Union of all the piece masks, for check validation */
extern const u64 all_mask[64];

#endif
