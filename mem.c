#include "mem.h"
#include "addon.h"

#include <stdlib.h>

#ifdef CFG_OWNMEM

#include <assert.h>
#define MEMSZ CFG_MEMSZ

struct game_struct boards[MEMSZ];
int slab_list[MEMSZ];
int nfree = -1;

void init_mem() {
	int i;

	for (i=0; i<MEMSZ; i++)
		slab_list[i] = i;

	nfree = MEMSZ;
}

game galloc() {
	int t;

#ifdef CFG_TRANSPOSITION
	if (nfree == 0) {
		addon_free_mem();
	}
#endif

	assert(nfree > 0);
	nfree--;
	t = slab_list[nfree];

	return &boards[t];
}

void gfree(game g) {
	int t = g - &boards[0];

	/* redondeo... */ 
	assert(&boards[t] == g);

	slab_list[nfree] = t;
	nfree++;
}

#else

void init_mem() {
}

game galloc () {
	return malloc(sizeof (struct game_struct));
}

void gfree(game g) {
	free(g);
}

#endif
