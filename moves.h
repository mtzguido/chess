#ifndef __MOVES_H
#define __MOVES_H

bool doMove(move m);
bool doMove_unchecked(move m);
void undoMove(void);

#define MAX_HPLY 2000
extern int hply;

struct undo_info {
	move m;
	piece_t capt;
	bool was_promote;
	bool was_ep;

	u8 idlecount;
	bool castle_king[2];
	bool castle_queen[2];
	i8 en_passant_x:4;
	i8 en_passant_y:4;
	bool castled[2];
	i8 inCheck[2];

	u64 hash;
};

extern struct undo_info * const hstack;
#endif
