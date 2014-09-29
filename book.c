#include "book.h"

struct book_entry {
	u64 hash;
	u8 r:4;
	u8 c:4;
	u8 R:4;
	u8 C:4;
	enum move_types move_type:3;
} __attribute__((packed));

/* Define the actual book */
#include "book.gen"

bool bookMove(move *m) {
	int i;
	int chance = 0;
	int match = -1;

	/* Exit gracefully if no book compiled in */
	if (booklen == 0)
		return false;

	for (i = 0; i < booklen; i++) {
		if (book[i].hash == G->zobrist) {
			if (!copts.shuffle) {
				match = i;
				break;
			}

			chance++;
			if (rand() % chance == 0)
				match = i;
		}
	}

	if (match != -1) {
		m->r = book[match].r;
		m->c = book[match].c;
		m->R = book[match].R;
		m->C = book[match].C;
		m->move_type = book[match].move_type;
		m->who = G->turn;
		m->promote = 0;
		return true;
	}

	return false;
}
