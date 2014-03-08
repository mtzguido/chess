#include "board.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "move.h"

static char charOf(int piece);
static int threatens(game g, int r, int c, int R, int C);

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
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, BQUEEN, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, BROOK, EMPTY },
		{ EMPTY, EMPTY, WKING, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY }
	},
	.turn = BLACK,
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
	return g;
}

game copyGame(game g) {
	game ret = malloc(sizeof (*ret));
	*ret = *g;
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

/* True if a change in (r,c) can never cause
 * a threat to (kr, kc). Having a piece type is useless
 * because something like
 * B . . .
 * . N . .
 * . . k .
 * . . . .
 * may occur. Where moving the knight could cause a check
 * even when it's not the piece that threatens the king */
static int safe (int r, int c, int kr, int kc) {
	int dx, dy;
	if (r == kr || c == kc) return 0; /* misma fila o columna */
	dx = abs(r - kr);
	dy = abs(c - kc);
	if (dx == dy) return 0; /* misma diagonal */
	if (dx + dy == 3 && dx != 0 && dy != 0) return 0; /* caballos */
	return 1;
}

static void doMove_impl(game g, move m) {
	int other = flipTurn(m.who);
	assert(g->turn == m.who);
	g->lastmove = m;

	switch (m.move_type) {
	case MOVE_REGULAR: {
		int8_t piece = g->board[m.r][m.c];
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
			g->board[m.r][m.C] = 0; /* sic */
		}

		/* Recalcular en passant */
		if (isPawn(piece) && abs(m.r - m.R) == 2) {
			assert (m.c == m.C);
			g->en_passant_x = (m.r+m.R)/2; /* je.. */
			g->en_passant_y = m.c;
		} else {
			g->en_passant_x = -1;
			g->en_passant_y = -1;
		}

		g->board[m.R][m.C] = g->board[m.r][m.c];
		g->board[m.r][m.c] = 0;

		/* Es un peón que promueve? */
		if (isPawn(piece)
				&& m.R == (m.who == WHITE ? 0 : 7)) {
			g->board[m.R][m.C] = m.who == WHITE ? m.promote : -m.promote;
		}

		/* Necesitamos también (posiblemente) dropear la nuestra,
		 * doMove_impl puede ser llamado con una movida no válida */
		if (!safe(m.r, m.c, g->kingx[m.who], g->kingy[m.who]) ||
			!safe(m.R, m.C, g->kingx[m.who], g->kingy[m.who]) ||
			isKing(piece))
			g->inCheck[m.who] = -1;

		/* Si es algún movimiento relevante al rey contrario
		 * dropeamos la cache */
		if (!safe(m.r, m.c, g->kingx[other], g->kingy[other]) ||
			!safe(m.R, m.C, g->kingx[other], g->kingy[other]))
			g->inCheck[other] = -1;

		break;
	}
	case MOVE_KINGSIDE_CASTLE:
		g->castle_king[m.who] = 0;

		/* Dropeamos la cache de jaque
		 * del oponente. Solo por simpleza,
		 * se podría llamar 4 veces a safe
		 * pero no se si es rentable */
		g->inCheck[other] = -1;

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
		break;
	case MOVE_QUEENSIDE_CASTLE:
		g->castle_queen[m.who] = 0;
		g->inCheck[other] = -1;

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
		break;

	default:
		assert("UNIMPLEMENTED" == NULL);
	}
}

void doMove(game g, move m) {
	assert(isLegalMove(g, m));
	doMove_impl(g, m);

	/* Sabemos con certeza que no podemos estar en jaque,
	 * ya que la movida es válida (isLegalMove debe haber
	 * retornado true para poder llamar a doMove). */
	g->inCheck[m.who] = 0;
	g->turn = flipTurn(g->turn);
}


int isFinished(game g) {
	if (hasNextGame(g) != 0)
		return -1;
	else if (inCheck(g, g->turn))
		return WIN(flipTurn(g->turn)); /* Current player is checkmated */
	else
		return DRAW; /* Stalemate */
}

int inCheck(game g, int who) {
	int kr, kc;
	int i, j;

	if (g->inCheck[who] != -1)
		return g->inCheck[who];

	kr = g->kingx[who];
	kc = g->kingy[who];

	if (threatens(g, kr-2, kc-1, kr, kc)) goto ret_true;
	if (threatens(g, kr+2, kc-1, kr, kc)) goto ret_true;
	if (threatens(g, kr-2, kc+1, kr, kc)) goto ret_true;
	if (threatens(g, kr+2, kc+1, kr, kc)) goto ret_true;
	if (threatens(g, kr-1, kc-2, kr, kc)) goto ret_true;
	if (threatens(g, kr+1, kc-2, kr, kc)) goto ret_true;
	if (threatens(g, kr-1, kc+2, kr, kc)) goto ret_true;
	if (threatens(g, kr+1, kc+2, kr, kc)) goto ret_true;
	for (i=kr+1; i<8; i++)
		if (threatens(g, i, kc, kr, kc)) goto ret_true;
		else if (g->board[i][kc] != 0) break;
	for (i=kr-1; i>=0; i--)
		if (threatens(g, i, kc, kr, kc)) goto ret_true;
		else if (g->board[i][kc] != 0) break;
	for (i=kc+1; i<8; i++)
		if (threatens(g, kr, i, kr, kc)) goto ret_true;
		else if (g->board[kr][i] != 0) break;
	for (i=kc-1; i>=0; i--)
		if (threatens(g, kr, i, kr, kc)) goto ret_true;
		else if (g->board[kr][i] != 0) break;

	for (i=kr-1, j=kc-1; i>=0 && j>=0; i--, j--)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;
	for (i=kr+1, j=kc+1; i<8 && j<8; i++, j++)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;
	for (i=kr+1, j=kc-1; i<8 && j>=0; i++, j--)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;
	for (i=kr-1, j=kc+1; i>=0 && j<8; i--, j++)
		if (threatens(g, i, j, kr, kc)) goto ret_true;
		else if (g->board[i][j] != 0) break;

	assert(g->inCheck[who] != 1);
	g->inCheck[who] = 0;
	return 0;

ret_true:

	assert(g->inCheck[who] != 0);
	g->inCheck[who] = 1;
	return 1;
}

static inline int threatens(game g, int r, int c, int R, int C) {
	return r >= 0 && r < 8 
	    && R >= 0 && R < 8 
	    && c >= 0 && c < 8 
	    && C >= 0 && C < 8 
	    && g->board[r][c]*g->board[R][C] < 0 
	    && canMove(g, r, c, R, C);
}

int isLegalMove(game g, move m) {
	game ng = copyGame(g);
	int ret;

	assert(m.who == g->turn);

	switch (m.move_type) {
	case MOVE_REGULAR:
		/* Siempre se mueve una pieza propia */
		if (g->board[m.r][m.c] == 0 || colorOf(g->board[m.r][m.c]) != g->turn) {
			ret = 0;
			goto out;
		}

		/* La pieza debe poder moverse al destino */
		if (!canMove(g, m.r, m.c, m.R, m.C)) {
			ret = 0;
			goto out;
		}

		/* Es un peón que promueve? */
		if (isPawn(g->board[m.r][m.c])
				&& m.R == (m.who == WHITE ? 0 : 7)) {
			if (m.promote == 0) {
				printf("Esa movida requiere una promoción!!!\n");
				ret = 0;
				goto out;
			}
		}

		break;
	case MOVE_KINGSIDE_CASTLE:
		if (m.who == WHITE) {
			if (!(g->castle_king[WHITE] && ! inCheck(g, WHITE)
			 && g->board[7][7] == WROOK && g->board[7][6] == EMPTY
			 && g->board[7][5] == EMPTY && g->board[7][4] == WKING)) {
				ret = 0;
				goto out;
			}
		} else {
			if (!(g->castle_king[BLACK] && ! inCheck(g, BLACK)
			 && g->board[0][7] == BROOK && g->board[0][6] == EMPTY
			 && g->board[0][5] == EMPTY && g->board[0][4] == BKING)) {
				ret = 0;
				goto out;
			}
		}
		break;
	case MOVE_QUEENSIDE_CASTLE:
		if (m.who == WHITE) {
			if (!(g->castle_queen[WHITE] && ! inCheck(g, WHITE)
			 && g->board[7][0] == WROOK && g->board[7][1] == EMPTY
			 && g->board[7][2] == EMPTY && g->board[7][3] == EMPTY
			 && g->board[7][4] == WKING)) {
				ret = 0;
				goto out;
			}
		} else {
			if (!(g->castle_queen[BLACK] && ! inCheck(g, BLACK)
			 && g->board[0][0] == BROOK && g->board[0][1] == EMPTY
			 && g->board[0][2] == EMPTY && g->board[0][3] == EMPTY
			 && g->board[0][4] == BKING)) {
				ret = 0;
				goto out;
			}
		}
		break;


	default:
		assert("UNIMPLEMENTED" == NULL);
	}

	/* Nunca podemos quedar en jaque */
	doMove_impl(ng, m);
	if (inCheck(ng, g->turn)) {
		ret = 0;
		goto out;
	}

	ret = 1;

out:
	freeGame(ng);
	return ret;
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

