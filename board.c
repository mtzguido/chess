#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "move.h"
#include "masks.h"
#include "piece-square.h"
#include "mem.h"
#include "board.h"
#include "zobrist.h"
#include "ztable.h"
#include "succs.h"

char charOf(int piece);

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
	.castled = { 0, 0 },
};

static void fix(game g) {
	int i, j;

	g->idlecount = 0;
	g->pieceScore[BLACK] = 0;
	g->pieceScore[WHITE] = 0;
	g->zobrist = 0;
	g->piecemask[BLACK] = 0;
	g->piecemask[WHITE] = 0;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			int piece = g->board[i][j];

			if (isKing(piece)) {
				g->kingx[colorOf(piece)] = i;
				g->kingy[colorOf(piece)] = j;
			}

			g->pieceScore[colorOf(piece)] += scoreOf(piece);

			if (piece) {
				g->zobrist ^= ZOBR_PIECE(piece, i, j);
				g->piecemask[colorOf(piece)]
					|= ((u64)1) <<(i*8 + j);
			}
		}
	}

	if (g->turn == BLACK) g->zobrist ^= ZOBR_BLACK();

	if (g->castle_king[WHITE])  g->zobrist ^= ZOBR_CASTLE_K(WHITE);
	if (g->castle_king[BLACK])  g->zobrist ^= ZOBR_CASTLE_K(BLACK);
	if (g->castle_queen[WHITE]) g->zobrist ^= ZOBR_CASTLE_Q(WHITE);
	if (g->castle_queen[BLACK]) g->zobrist ^= ZOBR_CASTLE_Q(BLACK);

	g->inCheck[BLACK] = -1;
	g->inCheck[WHITE] = -1;
	g->en_passant_x = -1;
	g->en_passant_y = -1;

	g->castled[WHITE] = 0;
	g->castled[BLACK] = 0;

	piecePosFullRecalc(g);
}

game startingGame() {
	game g = galloc();
	*g = init;
	fix(g);
	return g;
}

game copyGame(game g) {
	game ret = galloc();
	memcpy(ret, g, sizeof *ret);

	return ret;
}

void freeGame(game g) {
	gfree(g);
}

void printBoard(game g) {
	int i, j;

	char bbuf[200];

	fprintf(stderr, "(turn: %s)\n", g->turn == WHITE ? "WHITE" : "BLACK");
	for (i=0; i<8; i++) {
		fprintf(stderr, "%i  ", 8-i);
		for (j=0; j<8; j++) {
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			fputc(' ', stderr);
		}
		fputc('\n', stderr);
	}

	fprintf(stderr, "\n   a b c d e f g h\n");
	fprintf(stderr, "[ castle_king = %i %i \n", g->castle_king[0], g->castle_king[1]);
	fprintf(stderr, "[ castle_queen = %i %i \n", g->castle_queen[0], g->castle_queen[1]);
	fprintf(stderr, "[ kingx = %i %i \n", g->kingx[0], g->kingx[1]);
	fprintf(stderr, "[ kingy = %i %i \n", g->kingy[0], g->kingy[1]);
	fprintf(stderr, "[ en_passant = %i %i \n", g->en_passant_x, g->en_passant_y);
	fprintf(stderr, "[ inCheck = %i %i \n", g->inCheck[0], g->inCheck[1]);
	fprintf(stderr, "[ scores = %i %i\n", g->pieceScore[WHITE],
					      g->pieceScore[BLACK]);
	fprintf(stderr, "[ pps o e = %i %i\n", g->pps_O, g->pps_E);
	fprintf(stderr, "[ zobrist = 0x%" PRIx64 "\n", g->zobrist);
	fprintf(stderr, "[ idlecount = %i\n", g->idlecount);
	fprintf(stderr, "[ piecemask[W] = 0x%.16" PRIx64 "\n", g->piecemask[WHITE]);
	fprintf(stderr, "[ piecemask[B] = 0x%.16" PRIx64 "\n", g->piecemask[BLACK]);
	tostr(g, bbuf);
	fprintf(stderr, "[ tostr = <%s>\n", bbuf);


	fflush(stdout);
}

char charOf(int piece) {
	switch (piece) {
	case EMPTY:	return '.';
	case WPAWN:	return 'P';
	case BPAWN:	return 'p';
	case WROOK:	return 'R';
	case BROOK:	return 'r';
	case WKNIGHT:	return 'N';
	case BKNIGHT:	return 'n';
	case WBISHOP:	return 'B';
	case BBISHOP:	return 'b';
	case WQUEEN:	return 'Q';
	case BQUEEN:	return 'q';
	case WKING:	return 'K';
	case BKING:	return 'k';
	default:	assert(0);
	}

	return 'x';
}

/* No usa info de sucesores */
bool isDraw(game g) {
	int r = reps(g);

	assert(r > 0);

	return r >= 3 || g->idlecount >= 100;
}

int isFinished(game g) {
	int i, r = reps(g);
	game ng;

	assert(r > 0);
	assert(r <= 3);

	if (r == 3)
		return DRAW_3FOLD;
	else if (g->idlecount >= 100)
		return DRAW_50MOVE;

	assert(ply == 0);
	genSuccs(g);
	ng = copyGame(g);

	for (i=first_succ[ply]; i<first_succ[ply+1]; i++) {
		/*
		 * Si hay un sucesor válido,
		 * el juego no terminó
		 */
		if (doMove_unchecked(ng, gsuccs[i].m))
			goto not_finished;
	}

	freeGame(ng);

	if (inCheck(g, g->turn))
		return WIN(flipTurn(g->turn));
	else
		return DRAW_STALE;

not_finished:
	freeGame(ng);
	return -1;
}

/*
 * Rompemos la simetría en el caso de los reyes,
 * para optimizar un poco.
 */
static bool inCheck_diag_sw(game g, int kr, int kc, int who);
static bool inCheck_diag_se(game g, int kr, int kc, int who);
static bool inCheck_diag_nw(game g, int kr, int kc, int who);
static bool inCheck_diag_ne(game g, int kr, int kc, int who);
static bool inCheck_row_w(game g, int kr, int kc, int who);
static bool inCheck_row_e(game g, int kr, int kc, int who);
static bool inCheck_col_n(game g, int kr, int kc, int who);
static bool inCheck_col_s(game g, int kr, int kc, int who);
static bool inCheck_knig(game g, int kr, int kc, int who);
static bool inCheck_pawn(game g, int kr, int kc, int who);
static bool inCheck_king(game g);

bool inCheck(game g, int who) {
	u8 kr, kc;

	if (g->inCheck[who] != -1)
		return g->inCheck[who];

	kr = g->kingx[who];
	kc = g->kingy[who];

	g->inCheck[who] =  inCheck_diag_sw(g, kr, kc, who)
			|| inCheck_diag_se(g, kr, kc, who)
			|| inCheck_diag_nw(g, kr, kc, who)
			|| inCheck_diag_ne(g, kr, kc, who)
			|| inCheck_row_w(g, kr, kc, who)
			|| inCheck_row_e(g, kr, kc, who)
			|| inCheck_col_n(g, kr, kc, who)
			|| inCheck_col_s(g, kr, kc, who)
			|| inCheck_knig(g, kr, kc, who)
			|| inCheck_pawn(g, kr, kc, who)
			|| inCheck_king(g);

	return g->inCheck[who];
}

static bool inCheck_row_e(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(g->piecemask[flipTurn(who)] & row_e_mask[kr*8+kc]))
		return false;

	i = kr;
	for (j=kc+1; j<8; j++) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_row_w(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(g->piecemask[flipTurn(who)] & row_w_mask[kr*8+kc]))
		return false;

	i = kr;
	for (j=kc-1; j>=0; j--) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_col_s(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(g->piecemask[flipTurn(who)] & col_s_mask[kr*8+kc]))
		return 0;

	j = kc;
	for (i=kr+1; i<8; i++) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_col_n(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(g->piecemask[flipTurn(who)] & col_n_mask[kr*8+kc]))
		return 0;

	j = kc;
	for (i=kr-1; i>=0; i--) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_sw(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(g->piecemask[flipTurn(who)] & diag_sw_mask[kr*8+kc]))
		return false;

	j = kc;
	for (i=kr+1, j=kc-1; i<8 && j>=0; i++, j--) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_nw(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(g->piecemask[flipTurn(who)] & diag_nw_mask[kr*8+kc]))
		return false;

	for (i=kr-1, j=kc-1; i>=0 && j>=0; i--, j--) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_se(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(g->piecemask[flipTurn(who)] & diag_se_mask[kr*8+kc]))
		return false;

	for (i=kr+1, j=kc+1; i<8 && j<8; i++, j++) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_ne(game g, int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(g->piecemask[flipTurn(who)] & diag_ne_mask[kr*8+kc]))
		return false;

	for (i=kr-1, j=kc+1; i>=0 && j<8; i--, j++) {
		if (any_piece(g, i, j)) {
			if (g->board[i][j] == enemy_q
					|| g->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_knig(game g, int kr, int kc, int who) {
	const piece_t enemy_kn = who == WHITE ? BKNIGHT : WKNIGHT;

	if (!(g->piecemask[flipTurn(who)] & knight_mask[kr*8+kc]))
		return false;

	/* Caballos */
	if (kr >= 2 && kc >= 1 && g->board[kr-2][kc-1] == enemy_kn) return true; 
	if (kr <= 5 && kc >= 1 && g->board[kr+2][kc-1] == enemy_kn) return true; 
	if (kr >= 2 && kc <= 6 && g->board[kr-2][kc+1] == enemy_kn) return true; 
	if (kr <= 5 && kc <= 6 && g->board[kr+2][kc+1] == enemy_kn) return true; 
	if (kr >= 1 && kc >= 2 && g->board[kr-1][kc-2] == enemy_kn) return true; 
	if (kr <= 6 && kc >= 2 && g->board[kr+1][kc-2] == enemy_kn) return true; 
	if (kr >= 1 && kc <= 5 && g->board[kr-1][kc+2] == enemy_kn) return true; 
	if (kr <= 6 && kc <= 5 && g->board[kr+1][kc+2] == enemy_kn) return true; 

	return false;
}

static bool inCheck_pawn(game g, int kr, int kc, int who) {
	if (who == WHITE) {
		if (kr > 0) {
			if (kc > 0 && g->board[kr-1][kc-1] == BPAWN)
				return true;
			if (kc < 7 && g->board[kr-1][kc+1] == BPAWN)
				return true;
		}

		return 0;
	} else {
		if (kr < 7) {
			if (kc > 0 && g->board[kr+1][kc-1] == WPAWN)
				return true;
			if (kc < 7 && g->board[kr+1][kc+1] == WPAWN)
				return true;
		}

		return false;
	}
}

static bool inCheck_king(game g) {
	/* Simplemente viendo la distancia */

	return     abs(g->kingx[0] - g->kingx[1]) <= 1
		&& abs(g->kingy[0] - g->kingy[1]) <= 1;
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
static bool danger(u8 r, u8 c, u8 kr, u8 kc) {
	return all_mask[8*kr + kc] & ((u64)1 << (r*8 + c));
}

/* 
 * Auxiliares que deshabilitan el enroque,
 * no existen sus inversas ya que nunca se
 * habilita
 */
static void disable_castle_k(game g, int who) {
	if (g->castle_king[who])
		g->zobrist ^= ZOBR_CASTLE_K(who);

	g->castle_king[who] = 0;
}

static void disable_castle_q(game g, int who) {
	if (g->castle_queen[who])
		g->zobrist ^= ZOBR_CASTLE_Q(who);

	g->castle_queen[who] = 0;
}

/* Auxiliares de en_passant */
static void set_ep(game g, u8 r, u8 c) {
	g->zobrist ^= ZOBR_EP(g->en_passant_x);

	g->en_passant_x = r;
	g->en_passant_y = c;

	g->zobrist ^= ZOBR_EP(g->en_passant_x);
}

static bool doMoveRegular(game g, move m, bool check);
static bool doMoveKCastle(game g, move m, bool check);
static bool doMoveQCastle(game g, move m, bool check);
static bool doMoveNull(game g, move m, bool check);

/*
 * 1 : Ok
 * 0 : Movida no válida, deja a g intacto
 */
static bool __doMove(game g, move m, bool check) {
	assert(m.who == g->turn);

	game old_g = copyGame(g);

	switch (m.move_type) {
	case MOVE_REGULAR:
		if (!doMoveRegular(g, m, check))
			goto fail;

		break;
	case MOVE_KINGSIDE_CASTLE:
		if (!doMoveKCastle(g, m, check))
			goto fail;

		set_ep(g, -1, -1);
		g->lastmove = m;
		g->castled[m.who] = 1;

		break;

	case MOVE_QUEENSIDE_CASTLE:
		if (!doMoveQCastle(g, m, check))
			goto fail;

		set_ep(g, -1, -1);
		g->lastmove = m;
		g->castled[m.who] = 1;

		break;

	case MOVE_NULL:
		assert(copts.nullmove);
		if (!doMoveNull(g, m, check))
			goto fail;

		set_ep(g, -1, -1);
		g->lastmove = m;

		break;

	default:
		assert(0);
		goto fail;
	}

	/* Nunca podemos quedar en jaque */
	if (g->inCheck[g->turn] == 1  ||
		(g->inCheck[g->turn] == -1 && inCheck(g, g->turn)))
		goto fail;

	freeGame(old_g);

	g->turn = flipTurn(g->turn);
	g->zobrist ^= ZOBR_BLACK();

	return true;

fail:
	memcpy(g, old_g, sizeof *g);
	freeGame(old_g);

	return false;
}

bool doMove(game g, move m) {
	return __doMove(g, m, true);
}

bool doMove_unchecked(game g, move m) {
	return __doMove(g, m, false);
}

/* Auxiliares de doMoveRegular */
static inline void setPiece(game g, i8 r, i8 c, piece_t piece);
static bool isValid(game g, move m);
static inline void updKing(game g, move m);
static inline void updCastling(game g, move m);
static inline void epCapture(game g, move m);
static inline void epCalc(game g, move m);
static inline void calcPromotion(game g, move m);

static void setPiece(game g, i8 r, i8 c, piece_t piece) {
	piece_t old_piece = g->board[r][c];
	u8 old_who = colorOf(old_piece);
	u8 who = colorOf(piece);

	if (old_piece) {
		assert(old_piece != WKING && old_piece != BKING);
		g->pieceScore[old_who]	-= scoreOf(old_piece);
		g->pps_O		-= piece_square_val_O(old_piece, r, c);
		g->pps_E		-= piece_square_val_E(old_piece, r, c);
		g->zobrist		^= ZOBR_PIECE(old_piece, r, c);
		g->piecemask[old_who]	^= ((u64)1) << (r*8 + c);
	}

	g->board[r][c] = piece;

	if (piece) {
		g->piecemask[who]	^= ((u64)1) << (r*8 + c);
		g->zobrist		^= ZOBR_PIECE(piece, r, c);
		g->pps_E		+= piece_square_val_E(piece, r, c);
		g->pps_O		+= piece_square_val_O(piece, r, c);
		g->pieceScore[who]	+= scoreOf(piece);
	}
}

/*
 * movePiece(g, r, c, R, C) es equivalente a :
 *   setPiece(g, R, C, g->board[r][c]);
 *   setPiece(g, r, c, 0);
 *
 * pero ahorra llamadas innecesarias a scoreOf y 
 * anda mas rápido
 */
static void movePiece(game g, i8 r, i8 c, i8 R, i8 C) {
	const piece_t from = g->board[r][c];
	const piece_t to   = g->board[R][C];
	const u8 who = g->turn;
	const u8 enemy = flipTurn(who);

	assert(from != EMPTY);

	g->pps_O +=
		piece_square_val_O(from, R, C) - piece_square_val_O(from, r, c);
	g->pps_E +=
		piece_square_val_E(from, R, C) - piece_square_val_E(from, r, c);
	g->zobrist ^=
		ZOBR_PIECE(from, r, c) ^ ZOBR_PIECE(from, R, C);
	g->piecemask[who] ^=
		(((u64)1) << (r*8 + c)) ^ (((u64)1) << (R*8 + C));

	g->board[r][c] = EMPTY;
	g->board[R][C] = from;

	/* Si hubo captura */
	if (to) {
		assert(to != WKING && to != BKING);
		g->pieceScore[enemy]	-= scoreOf(to);
		g->pps_O		-= piece_square_val_O(to, R, C);
		g->pps_E		-= piece_square_val_E(to, R, C);
		g->zobrist		^= ZOBR_PIECE(to, R, C);
		g->piecemask[enemy]	^= ((u64)1) << (R*8 + C);
	}
}

static bool doMoveRegular(game g, move m, bool check) {
	const piece_t piece = g->board[m.r][m.c];
	const u8 other = flipTurn(g->turn);

	if (check) {
		if (!isValid(g, m))
			return false;

		/* No pisar piezas propias */
		if (own_piece(g, m.R, m.C))
			return false;
	}

	/* Es válida */
	g->lastmove = m;
	g->idlecount++;

	if (isPawn(piece)) {
		/* Los peones no son reversibles */
		g->idlecount = 0;

		/* Actuar si es una captura al paso */
		epCapture(g, m);

		/* Recalcular en passant */
		epCalc(g, m);

		/* Es un peón que promueve? */
		calcPromotion(g, m);
	} else {
		if (isKing(piece))
			updKing(g, m);
		else if (isRook(piece))
			updCastling(g, m);

		set_ep(g, -1, -1);
	}

	if (enemy_piece(g, m.R, m.C))
		g->idlecount = 0;

	/* Movemos */
	movePiece(g, m.r, m.c, m.R, m.C);

	/* Si es algún movimiento relevante al rey contrario
	 * dropeamos la cache */
	assert(g->inCheck[other] != 1);
	if (g->inCheck[other] == 0) {
		if (danger(m.r, m.c, g->kingx[other], g->kingy[other]) ||
		    danger(m.R, m.C, g->kingx[other], g->kingy[other]))
			g->inCheck[other] = -1;
	}

	/* Necesitamos también (posiblemente) dropear la nuestra */
	if (g->inCheck[m.who] == 1) {
		if (isKing(piece) ||
		    danger(m.R, m.C, g->kingx[m.who], g->kingy[m.who]))
			g->inCheck[m.who] = -1;
	} else if (g->inCheck[m.who] == 0) {
		if (isKing(piece) ||
		    danger(m.r, m.c, g->kingx[m.who], g->kingy[m.who]))
			g->inCheck[m.who] = -1;
	}

	return true;
}

static bool doMoveNull(game g, move m, bool check) {
	if (inCheck(g, g->turn))
		return false;

	return true;
}

static bool isValid(game g, move m) {
	piece_t piece = g->board[m.r][m.c];

	/* Siempre se mueve una pieza propia */
	if (m.who != g->turn ||
	    piece == EMPTY ||
	    colorOf(piece) != g->turn)
		return false;

	/* La pieza debe poder moverse al destino */
	if (m.r < 0 || m.c < 0
	 || m.R < 0 || m.C < 0
	 || m.r > 7 || m.c > 7
	 || m.R > 7 || m.C > 7
	 || (m.r == m.R && m.c == m.C)
	 || !canMove(g, m.r, m.c, m.R, m.C))
		return false;

	/* Es un peón que promueve? */
	if (isPawn(piece)
	 && m.R == (m.who == WHITE ? 0 : 7)
	 && m.promote == 0) {
			return false;
	}

	return true;
}

static void updKing(game g, move m) {
	g->kingx[m.who] = m.R;
	g->kingy[m.who] = m.C;

	disable_castle_k(g, m.who);
	disable_castle_q(g, m.who);
}

static void updCastling(game g, move m) {
	/* En vez de ver si se movió la torre
	 * correspondiente, nos fijamos en la
	 * casilla donde empieza la torre.
	 * Apenas hay un movimiento el enroque
	 * se invalida para siempre. */
	if (m.r != (m.who == WHITE ? 7 : 0))
		return;

	if (m.c == 7)
		disable_castle_k(g, m.who);
	else if (m.c == 0)
		disable_castle_q(g, m.who);
}

static void epCapture(game g, move m) {
	if (m.R == g->en_passant_x && m.C == g->en_passant_y) {
		setPiece(g, m.r, m.C, 0);
		g->inCheck[WHITE] = -1;
		g->inCheck[BLACK] = -1;
	}
}

static void epCalc(game g, move m) {
	if (abs(m.r - m.R) == 2) {
		assert (m.c == m.C);
		set_ep(g, (m.r+m.R)/2, m.c);
	} else {
		set_ep(g, -1, -1);
	}
}

static void calcPromotion(game g, move m) {
	if (m.R == (m.who == WHITE ? 0 : 7)) {
		piece_t new_piece = m.who == WHITE ? m.promote : (8 | m.promote);

		setPiece(g, m.r, m.c, new_piece);
	}
}

static bool doMoveKCastle(game g, move m, bool check) {
	const u8 rank = m.who == WHITE ? 7 : 0;
	const piece_t kpiece = m.who == WHITE ? WKING : BKING;
	const piece_t rpiece = m.who == WHITE ? WROOK : BROOK;

	if (check) {
		if (!(g->castle_king[m.who]
			&& g->board[rank][7] == rpiece && g->board[rank][6] == EMPTY
			&& g->board[rank][5] == EMPTY  && g->board[rank][4] == kpiece)) {

			return false;
		}
	}

	if (inCheck(g, m.who))
		return false;

	{
		game tg;

		tg = copyGame(g);
		tg->board[rank][4] = 0;
		tg->board[rank][5] = kpiece;
		tg->kingy[m.who] = 5;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return false;
		}

		tg->board[rank][5] = 0;
		tg->board[rank][6] = kpiece;
		tg->kingy[m.who] = 6;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return false;
		}

		freeGame(tg);
	}

	disable_castle_k(g, m.who);
	disable_castle_q(g, m.who);

	/* Dropeamos la cache de jaque */
	g->inCheck[0] = -1;
	g->inCheck[1] = -1;

	/* Mover rey */
	movePiece(g, rank, 4, rank, 6);
	/* Mover torre */
	movePiece(g, rank, 7, rank, 5);

	g->kingx[m.who] = rank;
	g->kingy[m.who] = 6;

	return true;
}

static bool doMoveQCastle(game g, move m, bool check) {
	const u8 rank = m.who == WHITE ? 7 : 0;
	const piece_t kpiece = m.who == WHITE ? WKING : BKING;
	const piece_t rpiece = m.who == WHITE ? WROOK : BROOK;

	if (check) {
		if (!(g->castle_queen[m.who]
			&& g->board[rank][0] == rpiece && g->board[rank][1] == EMPTY
			&& g->board[rank][2] == EMPTY  && g->board[rank][3] == EMPTY
			&& g->board[rank][4] == kpiece)) {

			return false;
		}
	}

	if (inCheck(g, m.who))
		return false;

	{
		game tg;

		tg = copyGame(g);
		tg->board[rank][4] = 0;
		tg->board[rank][3] = kpiece;
		tg->kingy[m.who] = 3;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return false;
		}

		tg->board[rank][3] = 0;
		tg->board[rank][2] = kpiece;
		tg->kingy[m.who] = 2;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return false;
		}

		freeGame(tg);
	}

	disable_castle_k(g, m.who);
	disable_castle_q(g, m.who);

	/* Dropeamos la cache de jaque */
	g->inCheck[0] = -1;
	g->inCheck[1] = -1;

	/* Mover rey */
	movePiece(g, rank, 4, rank, 2);
	/* Mover torre */
	movePiece(g, rank, 0, rank, 3);

	g->kingx[m.who] = rank;
	g->kingy[m.who] = 2;

	return true;
}

static const int scoreTab[] = {
	[EMPTY]		= 0,
	[WPAWN]		= PAWN_SCORE,
	[WKNIGHT]	= KNIGHT_SCORE,
	[WBISHOP]	= BISHOP_SCORE,
	[WROOK]		= ROOK_SCORE,
	[WQUEEN]	= QUEEN_SCORE,
	[WKING]		= 0,
};

int scoreOf(int piece) {
	return scoreTab[piece & 7];
}

bool equalGame(game a, game b) {
	return a->zobrist == b->zobrist;
}

void tostr(game g, char *s) {
	int i, j;
	char buf[10];

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++)
			*s++ = charOf(g->board[i][j]);
	}

	*s++ = g->turn == WHITE ? 'W' : 'B';

	sprintf(buf, "%02d", g->idlecount);
	assert(strlen(buf) == 2);
	strcpy(s, buf);
	s += 2;

	*s++ = g->castle_king[WHITE] ? '1' : '0';
	*s++ = g->castle_king[BLACK] ? '1' : '0';
	*s++ = g->castle_queen[WHITE] ? '1' : '0';
	*s++ = g->castle_queen[BLACK] ? '1' : '0';
	*s++ = g->castled[WHITE] ? '1' : '0';
	*s++ = g->castled[BLACK] ? '1' : '0';
	*s++ = g->en_passant_x + '1';
	*s++ = g->en_passant_y + '1';

	*s = 0;
}

game fromstr(char *s) {
	int i, j;
	game ret = galloc();
	char buf[3];

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++)
			ret->board[i][j] = pieceOf(*s++);
	}

	ret->turn = *s++ == 'W' ? WHITE : BLACK;

	buf[0] = *s++;
	buf[1] = *s++;
	buf[2] = 0;

	ret->idlecount = atoi(buf);

	ret->castle_king[WHITE]  = *s++ == '1';
	ret->castle_king[BLACK]  = *s++ == '1';
	ret->castle_queen[WHITE] = *s++ == '1';
	ret->castle_queen[BLACK] = *s++ == '1';
	ret->castled[WHITE]      = *s++ == '1';
	ret->castled[BLACK]      = *s++ == '1';
	ret->en_passant_x        = *s++ - '1';
	ret->en_passant_y        = *s++ - '1';

	fix(ret);
	return ret;
}
