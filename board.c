#include "board.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "move.h"

static char charOf(int piece);
static int threatens(game g, int r, int c, int R, int C);

static float scoreOf(int piece);

static int doMoveRegular(game g, move m);
static int doMoveKCastle(game g, move m);
static int doMoveQCastle(game g, move m);

inline static int sign(int a) {
	if (a > 0) return 1;
	if (a < 0) return -1;
	return 0;
}

#if 1
static const struct game_struct
init = {
	.board= {
		{ BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK },
		{ BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN },
		{ WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK }
	},
	.turn = WHITE,
	.lastmove = { 0 },
	.idlecount = 0,
	.castle_king = { 1, 1 },
	.castle_queen = { 1, 1 },
	.en_passant_x = -1,
	.en_passant_y = -1,
};
#else
static const struct game_struct
init = {
	.board= {
		{ BKING, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, BQUEEN, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, WKING, EMPTY, EMPTY, WROOK }
	},
	.turn = WHITE,
	.lastmove = { 0 },
	.idlecount = 0,
	.castle_king = { 1, 1 },
	.castle_queen = { 1, 1 },
	.en_passant_x = -1,
	.en_passant_y = -1,
};
#endif

static void fixKings(game g) {
	int i, j;
	int kw = 0, kb = 0;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			if (g->board[i][j] == WKING) {
				assert(kw == 0);
				g->kingx[WHITE] = i;
				g->kingy[WHITE] = j;
				kw = 1;
			} else if (g->board[i][j] == BKING) {
				assert(kb == 0);
				g->kingx[BLACK] = i;
				g->kingy[BLACK] = j;
				kb = 1;
			}
		}
	}
}

game startingGame() {
	game g = malloc(sizeof(*g));
	*g = init;
	fixKings(g);

	g->inCheck[BLACK] = -1;
	g->inCheck[WHITE] = -1;

	g->pieceScore = 0;
	return g;
}

game copyGame(game g) {
	game ret = malloc(sizeof (*ret));
	*ret = *g;

	assert(ret->turn == g->turn);
	return ret;
}

void freeGame(game g) {
	free(g);
}

void printBoard(game g) {
	int i, j;

	printf("(turn: %s)\n", g->turn == WHITE ? "WHITE" : "BLACK");
	for (i=0; i<8; i++) {
		printf("   ");
		for (j=0; j<8; j++) {
			if (g->en_passant_x == i && g->en_passant_y == j)
				putchar('!');
			else
				putchar(charOf(g->board[i][j]));
			if (g->en_passant_x == i && g->en_passant_y == j)
				putchar('!');
			else
				putchar(charOf(g->board[i][j]));
			if (g->en_passant_x == i && g->en_passant_y == j)
				putchar('!');
			else
				putchar(charOf(g->board[i][j]));
			putchar(' ');
			putchar(' ');
			putchar(' ');
		}
		putchar('\n');
		printf("%i  ", 8-i);
		for (j=0; j<8; j++) {
			if (g->en_passant_x == i && g->en_passant_y == j)
				putchar('!');
			else
				putchar(charOf(g->board[i][j]));
			if (g->en_passant_x == i && g->en_passant_y == j)
				putchar('!');
			else
				putchar(charOf(g->board[i][j]));
			if (g->en_passant_x == i && g->en_passant_y == j)
				putchar('!');
			else
				putchar(charOf(g->board[i][j]));
			putchar(' ');
			putchar(' ');
			putchar(' ');
		}
		putchar('\n');
		putchar('\n');
	}

	printf("\n    a     b     c     d     e     f     g     h\n");
	printf("[ castle_king = %i %i \n", g->castle_king[0], g->castle_king[1]);
	printf("  castle_queen = %i %i \n", g->castle_queen[0], g->castle_queen[1]);
	printf("  kingx = %i %i \n", g->kingx[0], g->kingx[1]);
	printf("  kingy = %i %i \n", g->kingy[0], g->kingy[1]);
	printf("  en_passant = %i %i \n", g->en_passant_x, g->en_passant_y);
	printf("  inCheck = %i %i \n", g->inCheck[0], g->inCheck[1]);
	printf("]\n");


	fflush(stdout);
}

static char charOf(int piece) {
	switch (piece) {
		case EMPTY:   return '.';
		case WPAWN:   return 'P';
		case BPAWN:   return 'p';
		case WROOK:   return 'R';
		case BROOK:   return 'r';
		case WKNIGHT: return 'N';
		case BKNIGHT: return 'n';
		case WBISHOP: return 'B';
		case BBISHOP: return 'b';
		case WQUEEN:  return 'Q';
		case BQUEEN:  return 'q';
		case WKING:	  return 'K';
		case BKING:	  return 'k';
		default:      assert(0);
	}
}

int isFinished(game g) {
	if (hasNextGame(g) != 0)
		return -1;
	else if (inCheck(g, g->turn))
		return WIN(flipTurn(g->turn)); /* Jaque mate al jugador actual */
	else
		return DRAW; /* Ahogado (Stalemate) */
}

int inCheck(game g, int who) {
	int kr, kc;
	int i, j;

	if (g->inCheck[who] != -1)
		return g->inCheck[who];

	kr = g->kingx[who];
	kc = g->kingy[who];

	/* Columna */
	j = kc;
	for (i=kr+1; i<8; i++)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	for (i=kr-1; i>=0; i--)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	/* Fila */
	i = kr;
	for (j=kc+1; j<8; j++)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	for (j=kc-1; j>=0; j--)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	/* Diagonales */
	for (i=kr-1, j=kc-1; i>=0 && j>=0; i--, j--)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	for (i=kr+1, j=kc+1; i<8 && j<8; i++, j++)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	for (i=kr+1, j=kc-1; i<8 && j>=0; i++, j--)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	for (i=kr-1, j=kc+1; i>=0 && j<8; i--, j++)
		if (g->board[i][j] != 0) {
			if (threatens(g, i, j, kr, kc))
				goto ret_true;
			else
				break;
		}

	/* Caballos */
	if (threatens(g, kr-2, kc-1, kr, kc)) goto ret_true;
	if (threatens(g, kr+2, kc-1, kr, kc)) goto ret_true;
	if (threatens(g, kr-2, kc+1, kr, kc)) goto ret_true;
	if (threatens(g, kr+2, kc+1, kr, kc)) goto ret_true;
	if (threatens(g, kr-1, kc-2, kr, kc)) goto ret_true;
	if (threatens(g, kr+1, kc-2, kr, kc)) goto ret_true;
	if (threatens(g, kr-1, kc+2, kr, kc)) goto ret_true;
	if (threatens(g, kr+1, kc+2, kr, kc)) goto ret_true;

	assert(g->inCheck[who] != 1);
	g->inCheck[who] = 0;
	return 0;

ret_true:

	assert(g->inCheck[who] != 0);
	g->inCheck[who] = 1;
	return 1;
}

/* Debe ser llamado con r,c,R,C VALIDOS */
static inline int threatens(game g, int r, int c, int R, int C) {
	return (g->board[r][c] ^ g->board[R][C]) < 0
	    && canMove(g, r, c, R, C);
}


void freeSuccs(game *arr, int len) {
	int i;

	for (i=0; i<len; i++)
		freeGame(arr[i]);

	free(arr);
}

int equalMove(move a, move b) {
	if (a.who != b.who) return 0;
	if (a.move_type != b.move_type) return 0;

	if (a.move_type != MOVE_REGULAR)
		return 1;

	return a.r == b.r
		&& a.c == b.c
		&& a.R == b.R
		&& a.C == b.C
		&& a.promote == b.promote;
}

/*
 * Devuelve verdadero si un cambio en (r,c)
 * nunca puede causar una amenaza a (kr, kc),
 * para el tablero dado en g.
 *
 * No es útil tener el tipo de pieza movida, ya 
 * que puede ocurrir algo como:
 *
 * B . . .
 * . N . .
 * . . k .
 * . . . .
 *
 * en donde mover el caballo causa un jaque aún
 * cuando no lo amenaza
 */
static int safe(game g, int r, int c, int kr, int kc) {
	/* Aparentemente, esto no ayuda */
//	return 0;


	int sr, sc; /* step */
	int dx, dy;
	int i, j;

	dx = abs(r - kr);
	dy = abs(c - kc);

	if (dx + dy == 3) {
		/* Caballos o diagonal */
		return 0; 
	} else if (r == kr || c == kc || dx == dy) {
		sr = kr > r ? 1 : kr == r ? 0 : -1;
		sc = kc > c ? 1 : kc == c ? 0 : -1;

		for (i=r+sr, j=c+sc; i != kr || j != kc ; i+=sr, j+=sc)
			if (g->board[i][j] != 0)
				return 1;
		 
		return 0;
	}

	return 1;
}

/*
 * 1 : Ok
 * 0 : Movida no válida, deja a g intacto
 */
int doMove(game g, move m) {
	assert(m.who == g->turn);

	game old_g = copyGame(g);

	switch (m.move_type) {
	case MOVE_REGULAR:
		if (!doMoveRegular(g, m))
			goto fail;

		break;
	case MOVE_KINGSIDE_CASTLE:
		if (!doMoveKCastle(g, m))
			goto fail;

		break;
	
	case MOVE_QUEENSIDE_CASTLE:
		if (!doMoveQCastle(g, m))
			goto fail;

		break;
	default:
		assert(0);
	}

	/* Nunca podemos quedar en jaque */
	if (g->inCheck[g->turn] != 0
		&& inCheck(g, g->turn))
		goto fail;

	freeGame(old_g);

	assert(g->inCheck[g->turn] == 0);
	g->turn = flipTurn(g->turn);

	return 1;

fail:
	*g = *old_g;
	freeGame(old_g);

	return 0;
}

static int doMoveRegular(game g, move m) {
	int piece = g->board[m.r][m.c];
	int other = flipTurn(g->turn);

	/* Siempre se mueve una pieza propia */
	if (piece == 0 || colorOf(piece) != g->turn)
		return 0;

	/* La pieza debe poder moverse al destino */
	if (!canMove(g, m.r, m.c, m.R, m.C))
		return 0;

	/* Es un peón que promueve? */
	if (isPawn(piece)
			&& m.R == (m.who == WHITE ? 0 : 7)) {
		if (m.promote == 0) {
			printf("Esa movida requiere una promoción!!!\n");
			return 0;
		}
	}

	/* Es válida */
	g->lastmove = m;

	if (isKing(piece)) {
		g->kingx[colorOf(piece)] = m.R;
		g->kingy[colorOf(piece)] = m.C;
		g->castle_king[m.who] = 0;
		g->castle_queen[m.who] = 0;
	}

	/* En vez de ver si se movió la torre
	 * correspondiente, nos fijamos en la
	 * casilla donde empieza la torre.
	 * Apenas hay un movimiento el enroque
	 * se invalida para siempre. */
	if (m.r == m.who*7) {
		if (m.c == 7)
			g->castle_king[m.who] = 0;
		else if (m.c == 0)
			g->castle_queen[m.who] = 0;
	}

	/* Es una captura al paso? */
	if (isPawn(piece)
			&& m.R == g->en_passant_x
			&& m.C == g->en_passant_y) {
		g->pieceScore -= scoreOf(g->board[m.r][m.C]);
		g->board[m.r][m.C] = 0;
	}

	/* Recalcular en passant */
	if (isPawn(piece) && abs(m.r - m.R) == 2) {
		assert (m.c == m.C);
		g->en_passant_x = (m.r+m.R)/2;
		g->en_passant_y = m.c;
	} else {
		g->en_passant_x = -1;
		g->en_passant_y = -1;
	}

	if (g->board[m.R][m.C] != 0)
		g->pieceScore -= scoreOf(g->board[m.R][m.C]);

	g->board[m.R][m.C] = g->board[m.r][m.c];
	g->board[m.r][m.c] = 0;

	/* Es un peón que promueve? */
	if (isPawn(piece)
			&& m.R == (m.who == WHITE ? 0 : 7)) {
		g->pieceScore -= scoreOf(g->board[m.R][m.C]);
		g->board[m.R][m.C] = m.who == WHITE ? m.promote : -m.promote;
		g->pieceScore += scoreOf(g->board[m.R][m.C]);
	}

	/* Si es algún movimiento relevante al rey contrario
	 * dropeamos la cache */
	if (!safe(g, m.r, m.c, g->kingx[other], g->kingy[other]) ||
		!safe(g, m.R, m.C, g->kingx[other], g->kingy[other]))
		g->inCheck[other] = -1;

	/* Necesitamos también (posiblemente) dropear la nuestra */
	if (isKing(piece) ||
		!safe(g, m.r, m.c, g->kingx[m.who], g->kingy[m.who]) ||
		!safe(g, m.R, m.C, g->kingx[m.who], g->kingy[m.who]))
		g->inCheck[m.who] = -1;

	return 1;
}

static int doMoveKCastle(game g, move m) {
	if (m.who == WHITE) {
		if (!(g->castle_king[WHITE] && ! inCheck(g, WHITE)
			&& g->board[7][7] == WROOK && g->board[7][6] == EMPTY
			&& g->board[7][5] == EMPTY && g->board[7][4] == WKING)) {
			return 0;
		}
	} else {
		if (!(g->castle_king[BLACK] && ! inCheck(g, BLACK)
			&& g->board[0][7] == BROOK && g->board[0][6] == EMPTY
			&& g->board[0][5] == EMPTY && g->board[0][4] == BKING)) {
			return 0;
		}
	}

	g->castle_king[m.who] = 0;

	/* Dropeamos la cache de jaque */
	g->inCheck[0] = -1;
	g->inCheck[1] = -1;

	if (m.who == WHITE) {
		g->board[7][4] = EMPTY;
		g->board[7][5] = WROOK;
		g->board[7][6] = WKING;
		g->board[7][7] = EMPTY;
		g->kingx[WHITE] = 7;
		g->kingy[WHITE] = 6;
	} else {
		g->board[0][4] = EMPTY;
		g->board[0][5] = BROOK;
		g->board[0][6] = BKING;
		g->board[0][7] = EMPTY;
		g->kingx[BLACK] = 0;
		g->kingy[BLACK] = 6;
	}

	return 1;
}

static int doMoveQCastle(game g, move m) {
	if (m.who == WHITE) {
		if (!(g->castle_queen[WHITE]
			&& g->board[7][0] == WROOK && g->board[7][1] == EMPTY
			&& g->board[7][2] == EMPTY && g->board[7][3] == EMPTY
			&& g->board[7][4] == WKING && ! inCheck(g, WHITE))) {
			return 0;
		}
	} else {
		if (!(g->castle_queen[BLACK] 
			&& g->board[0][0] == BROOK && g->board[0][1] == EMPTY
			&& g->board[0][2] == EMPTY && g->board[0][3] == EMPTY
			&& g->board[0][4] == BKING && ! inCheck(g, BLACK))) {
			return 0;
		}
	}

	g->castle_queen[m.who] = 0;

	g->inCheck[0] = -1;
	g->inCheck[1] = -1;

	if (m.who == WHITE) {
		g->board[7][0] = EMPTY;
		g->board[7][1] = EMPTY;
		g->board[7][2] = WKING;
		g->board[7][3] = WROOK;
		g->board[7][4] = EMPTY;
		g->kingx[WHITE] = 7;
		g->kingy[WHITE] = 2;
	} else {
		g->board[0][0] = EMPTY;
		g->board[0][1] = EMPTY;
		g->board[0][2] = BKING;
		g->board[0][3] = BROOK;
		g->board[0][4] = EMPTY;
		g->kingx[BLACK] = 0;
		g->kingy[BLACK] = 2;
	}

	return 1;
}

inline static float scoreOf(int piece) {
	switch(piece) {
		case WQUEEN:	return  50;
		case BQUEEN:	return -50;
		case WROOK:	return 10;
		case WBISHOP:	return 10;
		case WKNIGHT:	return 10;
		case BROOK:	return -10;
		case BBISHOP:	return -10;
		case BKNIGHT:	return -10;
		case WPAWN:	return  1;
		case BPAWN:	return -1;
		case WKING:	return 0;
		case BKING:	return 0;
		case 0:	return 0;
		default: assert(0);
	}
}


