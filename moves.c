#include "zobrist.h"
#include "ztable.h"
#include "moves.h"
#include "board.h"
#include "masks.h"
#include "piece-square.h"
#include "legal.h"
#include "succs.h"
#include "check.h"
#include "common.h"

/* Leave a sentinel at the beginning */
static struct undo_info _hstack[MAX_HPLY+1] = {};
struct undo_info * const hstack = &_hstack[1];

int hply = 0;

static struct game_struct _G;
game const G = &_G;

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

static void set_castle_k(int who, bool val) {
	if (G->castle_king[who] != val)
		G->zobrist ^= ZOBR_CASTLE_K(who);

	G->castle_king[who] = val;
}

static void set_castle_q(int who, bool val) {
	if (G->castle_queen[who] != val)
		G->zobrist ^= ZOBR_CASTLE_Q(who);

	G->castle_queen[who] = val;
}

/* Auxiliares de en_passant */
static void set_ep(u8 r, u8 c) {
	G->zobrist ^= ZOBR_EP(G->en_passant_y);

	G->en_passant_x = r;
	G->en_passant_y = c;

	G->zobrist ^= ZOBR_EP(G->en_passant_y);
}

static bool isValid(const move * const m) {
	piece_t piece = G->board[m->r][m->c];

	/* Siempre se mueve una pieza propia */
	if (m->who != G->turn ||
	    piece == EMPTY ||
	    colorOf(piece) != G->turn)
		return false;

	/* La pieza debe poder moverse al destino */
	if (m->r < 0 || m->c < 0
	 || m->R < 0 || m->C < 0
	 || m->r > 7 || m->c > 7
	 || m->R > 7 || m->C > 7
	 || (m->r == m->R && m->c == m->C)
	 || !canMove(m->r, m->c, m->R, m->C))
		return false;

	/* Es un peón que promueve? */
	if (isPawn(piece)
	 && m->R == (m->who == WHITE ? 0 : 7)
	 && m->promote == 0) {
			return false;
	}

	return true;
}

static void updCastling(const move * const m) {
	/* En vez de ver si se movió la torre
	 * correspondiente, nos fijamos en la
	 * casilla donde empieza la torre.
	 * Apenas hay un movimiento el enroque
	 * se invalida para siempre. */
	if (m->r != (m->who == WHITE ? 7 : 0))
		return;

	if (m->c == 7)
		set_castle_k(m->who, false);
	else if (m->c == 0)
		set_castle_q(m->who, false);
}

static void recalcPawnRank(int col, int c) {
	int i;

	if (col == WHITE) {
		for (i = 6; i > 0; i--)
			if (G->board[i][c] == WPAWN)
				break;

		G->pawn_rank[WHITE][c+1] = i;
	} else {
		for (i = 1; i < 7; i++)
			if (G->board[i][c] == BPAWN)
				break;

		G->pawn_rank[BLACK][c+1] = i;
	}
}

static void setPiece(i8 r, i8 c, piece_t piece) {
	piece_t old_piece = G->board[r][c];
	u8 old_who = colorOf(old_piece);
	u8 who = colorOf(piece);

	if (old_piece) {
		assert(old_piece != WKING && old_piece != BKING);
		G->pieceScore[old_who]	-= scoreOf(old_piece);
		G->pps_O		-= piece_square_val_O(old_piece, r, c);
		G->pps_E		-= piece_square_val_E(old_piece, r, c);
		G->zobrist		^= ZOBR_PIECE(old_piece, r, c);
		G->piecemask[old_who]	^= ((u64)1) << (r*8 + c);
	}

	G->board[r][c] = piece;

	if (isPawn(piece)) {
		recalcPawnRank(who, c);
	} else if (isKing(piece)) {
		G->kingx[who] = r;
		G->kingy[who] = c;
	}

	if (isPawn(old_piece))
		recalcPawnRank(old_who, c);

	if (piece) {
		G->piecemask[who]	^= ((u64)1) << (r*8 + c);
		G->zobrist		^= ZOBR_PIECE(piece, r, c);
		G->pps_E		+= piece_square_val_E(piece, r, c);
		G->pps_O		+= piece_square_val_O(piece, r, c);
		G->pieceScore[who]	+= scoreOf(piece);
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
static void movePiece(i8 r, i8 c, i8 R, i8 C) {
	const piece_t from = G->board[r][c];
	const piece_t to   = G->board[R][C];
	const u8 who = G->turn;
	const u8 enemy = flipTurn(who);

	assert(from != EMPTY);
	assert(to != WKING);
	assert(to != BKING);

	G->pps_O +=
		piece_square_val_O(from, R, C) - piece_square_val_O(from, r, c);
	G->pps_E +=
		piece_square_val_E(from, R, C) - piece_square_val_E(from, r, c);
	G->zobrist ^=
		ZOBR_PIECE(from, r, c) ^ ZOBR_PIECE(from, R, C);
	G->piecemask[who] ^=
		(((u64)1) << (r*8 + c)) ^ (((u64)1) << (R*8 + C));

	G->board[r][c] = EMPTY;
	G->board[R][C] = from;

	if (isKing(from)) {
		G->kingx[who] = R;
		G->kingy[who] = C;
	}

	/* Si hubo captura */
	if (to) {
		assert(to != WKING && to != BKING);
		G->pieceScore[enemy]	-= scoreOf(to);
		G->pps_O		-= piece_square_val_O(to, R, C);
		G->pps_E		-= piece_square_val_E(to, R, C);
		G->zobrist		^= ZOBR_PIECE(to, R, C);
		G->piecemask[enemy]	^= ((u64)1) << (R*8 + C);
	}

	if (isPawn(from)) {
		if (c == C) {
			recalcPawnRank(who, c);
		} else {
			recalcPawnRank(who, c);
			recalcPawnRank(who, C);
		}
	}

	if (isPawn(to))
		recalcPawnRank(enemy, C);
}

static void epCapture(const move * const m) {
	if (m->R == G->en_passant_x && m->C == G->en_passant_y) {
		setPiece(m->r, m->C, 0);
		G->inCheck[WHITE] = -1;
		G->inCheck[BLACK] = -1;
		hstack[hply].was_ep = true;
	}
}

static void epCalc(const move * const m) {
	if (abs(m->r - m->R) == 2) {
		assert (m->c == m->C);
		set_ep((m->r+m->R)/2, m->c);
	} else {
		set_ep(-1, -1);
	}
}

static void calcPromotion(const move * const m) {
	if (m->R == (m->who == WHITE ? 0 : 7)) {
		piece_t new_piece = m->who == WHITE ? m->promote : (8 | m->promote);

		setPiece(m->r, m->c, new_piece);
		hstack[hply].was_promote = true;
	}
}

static bool doMoveRegular(const move * const m, bool check) {
	const piece_t piece = G->board[m->r][m->c];
	const u8 other = flipTurn(G->turn);

	if (check) {
		if (!isValid(m))
			return false;

		/* No pisar piezas propias */
		if (own_piece(m->R, m->C))
			return false;
	} else {
		assert(isValid(m));
		assert(!own_piece(m->R, m->C));
	}

	/* Es válida */
	G->idlecount++;

	if (isPawn(piece)) {
		/* Los peones no son reversibles */
		G->idlecount = 0;

		/* Actuar si es una captura al paso */
		epCapture(m);

		/* Recalcular en passant */
		epCalc(m);

		/* Es un peón que promueve? */
		calcPromotion(m);
	} else {
		if (isKing(piece)) {
			set_castle_k(m->who, false);
			set_castle_q(m->who, false);
		} else if (isRook(piece)) {
			updCastling(m);
		}

		set_ep(-1, -1);
	}

	if (enemy_piece(m->R, m->C)) {
		G->idlecount = 0;
		hstack[hply].capt = G->board[m->R][m->C];
	}

	/* Movemos */
	movePiece(m->r, m->c, m->R, m->C);

	/* Si es algún movimiento relevante al rey contrario
	 * dropeamos la cache */
	assert(G->inCheck[other] != 1);
	if (G->inCheck[other] == 0) {
		if (danger(m->r, m->c, G->kingx[other], G->kingy[other]) ||
		    danger(m->R, m->C, G->kingx[other], G->kingy[other]))
			G->inCheck[other] = -1;
	}

	/* Necesitamos también (posiblemente) dropear la nuestra */
	if (G->inCheck[m->who] == 1) {
		if (isKing(piece) ||
		    danger(m->R, m->C, G->kingx[m->who], G->kingy[m->who]))
			G->inCheck[m->who] = -1;
	} else if (G->inCheck[m->who] == 0) {
		if (isKing(piece) ||
		    danger(m->r, m->c, G->kingx[m->who], G->kingy[m->who]))
			G->inCheck[m->who] = -1;
	}

	return true;
}

static bool doMoveNull(const move * const m, bool check) {
	if (inCheck(G->turn))
		return false;

	return true;
}

static bool doMoveKCastle(const move * const m, bool check) {
	const u8 rank = m->who == WHITE ? 7 : 0;
	const piece_t kpiece = m->who == WHITE ? WKING : BKING;
	const piece_t rpiece = m->who == WHITE ? WROOK : BROOK;

	if (check) {
		if (!(G->castle_king[m->who]
			&& G->board[rank][7] == rpiece && G->board[rank][6] == EMPTY
			&& G->board[rank][5] == EMPTY  && G->board[rank][4] == kpiece)) {

			return false;
		}
	}

	if (inCheck(m->who))
		return false;

	{
		struct game_struct bak = *G;

		G->board[rank][4] = 0;
		G->board[rank][5] = kpiece;
		G->kingy[m->who] = 5;
		G->inCheck[m->who] = -1;

		if (inCheck(m->who)) {
			*G = bak;
			return false;
		}

		G->board[rank][5] = 0;
		G->board[rank][6] = kpiece;
		G->kingy[m->who] = 6;
		G->inCheck[m->who] = -1;

		if (inCheck(m->who)) {
			*G = bak;
			return false;
		}

		*G = bak;
	}

	set_castle_k(m->who, false);
	set_castle_q(m->who, false);

	/* Dropeamos la cache de jaque */
	G->inCheck[0] = -1;
	G->inCheck[1] = -1;

	/* Mover rey */
	movePiece(rank, 4, rank, 6);
	/* Mover torre */
	movePiece(rank, 7, rank, 5);

	return true;
}

static bool doMoveQCastle(const move * const m, bool check) {
	const u8 rank = m->who == WHITE ? 7 : 0;
	const piece_t kpiece = m->who == WHITE ? WKING : BKING;
	const piece_t rpiece = m->who == WHITE ? WROOK : BROOK;

	if (check) {
		if (!(G->castle_queen[m->who]
			&& G->board[rank][0] == rpiece && G->board[rank][1] == EMPTY
			&& G->board[rank][2] == EMPTY  && G->board[rank][3] == EMPTY
			&& G->board[rank][4] == kpiece)) {

			return false;
		}
	}

	if (inCheck(m->who))
		return false;

	{
		struct game_struct bak = *G;

		G->board[rank][4] = 0;
		G->board[rank][3] = kpiece;
		G->kingy[m->who] = 3;
		G->inCheck[m->who] = -1;

		if (inCheck(m->who)) {
			*G = bak;
			return false;
		}

		G->board[rank][3] = 0;
		G->board[rank][2] = kpiece;
		G->kingy[m->who] = 2;
		G->inCheck[m->who] = -1;

		if (inCheck(m->who)) {
			*G = bak;
			return false;
		}

		*G = bak;
	}

	set_castle_k(m->who, false);
	set_castle_q(m->who, false);

	/* Dropeamos la cache de jaque */
	G->inCheck[0] = -1;
	G->inCheck[1] = -1;

	/* Mover rey */
	movePiece(rank, 4, rank, 2);
	/* Mover torre */
	movePiece(rank, 0, rank, 3);

	return true;
}

static void _undoMove();

static bool __doMove(const move * const m, bool check) {
	hstack[hply].m = *m;
	hstack[hply].idlecount = G->idlecount;
	hstack[hply].capt = EMPTY;
	hstack[hply].was_promote = false;
	hstack[hply].was_ep = false;
	hstack[hply].castle_king[WHITE] = G->castle_king[WHITE];
	hstack[hply].castle_king[BLACK] = G->castle_king[BLACK];
	hstack[hply].castle_queen[WHITE] = G->castle_queen[WHITE];
	hstack[hply].castle_queen[BLACK] = G->castle_queen[BLACK];
	hstack[hply].castled[WHITE] = G->castled[WHITE];
	hstack[hply].castled[BLACK] = G->castled[BLACK];
	hstack[hply].en_passant_x = G->en_passant_x;
	hstack[hply].en_passant_y = G->en_passant_y;
	hstack[hply].inCheck[WHITE] = G->inCheck[WHITE];
	hstack[hply].inCheck[BLACK] = G->inCheck[BLACK];

	assert(m->who == G->turn);

	switch (m->move_type) {
	case MOVE_REGULAR:
		if (!doMoveRegular(m, check))
			goto fail;

		break;
	case MOVE_KINGSIDE_CASTLE:
		if (!doMoveKCastle(m, check))
			goto fail;

		set_ep(-1, -1);
		G->castled[m->who] = 1;

		break;

	case MOVE_QUEENSIDE_CASTLE:
		if (!doMoveQCastle(m, check))
			goto fail;

		set_ep(-1, -1);
		G->castled[m->who] = 1;

		break;

	case MOVE_NULL:
		assert(copts.null);
		if (!doMoveNull(m, check))
			goto fail;

		set_ep(-1, -1);

		break;

	default:
		assert(0);
		goto fail;
	}

	/* Nunca podemos quedar en jaque */
	if (G->inCheck[G->turn] == 1  ||
		(G->inCheck[G->turn] == -1 && inCheck(G->turn)))
		goto fail_undo;

	G->turn = flipTurn(G->turn);
	G->zobrist ^= ZOBR_BLACK();
	mark();
	ply++;
	hply++;

	first_succ[ply+1] = first_succ[ply];

	return true;

fail_undo:
	_undoMove();
fail:
	return false;
}

bool doMove(const move * const m) {
	return __doMove(m, true);
}

bool doMove_unchecked(const move * const m) {
	return __doMove(m, false);
}

void undoMoveRegular(const move * const m) {
	const piece_t pawn     = m->who == WHITE ? WPAWN : BPAWN;
	const piece_t opp_pawn = m->who == WHITE ? BPAWN : WPAWN;

	movePiece(m->R, m->C, m->r, m->c);

	if (hstack[hply].was_promote)
		setPiece(m->r, m->c, pawn);

	if (hstack[hply].capt != EMPTY)
		setPiece(m->R, m->C, hstack[hply].capt);
	else if (hstack[hply].was_ep)
		setPiece(m->r, m->C, opp_pawn);
}

void undoMoveKCastle() {
	const int who = G->turn;
	const u8 rank = who == WHITE ? 7 : 0;

	assert(G->kingx[who] == rank);
	assert(G->kingy[who] == 6);

	movePiece(rank, 6, rank, 4);
	movePiece(rank, 5, rank, 7);
}

void undoMoveQCastle() {
	const int who = G->turn;
	const u8 rank = who == WHITE ? 7 : 0;

	assert(G->kingx[who] == rank);
	assert(G->kingy[who] == 2);

	movePiece(rank, 2, rank, 4);
	movePiece(rank, 3, rank, 0);
}

void _undoMove() {
	const move * const m = &hstack[hply].m;

	assert(hply >= 0);

	assert(G->board[G->kingx[WHITE]][G->kingy[WHITE]] == WKING);
	assert(G->board[G->kingx[BLACK]][G->kingy[BLACK]] == BKING);

	switch (m->move_type) {
	case MOVE_REGULAR:
		undoMoveRegular(m);
		break;
	case MOVE_KINGSIDE_CASTLE:
		undoMoveKCastle();
		break;
	case MOVE_QUEENSIDE_CASTLE:
		undoMoveQCastle();
		break;
	case MOVE_NULL:
		break;
	}

	G->idlecount = hstack[hply].idlecount;
	set_castle_k(WHITE, hstack[hply].castle_king[WHITE]);
	set_castle_k(BLACK, hstack[hply].castle_king[BLACK]);
	set_castle_q(WHITE, hstack[hply].castle_queen[WHITE]);
	set_castle_q(BLACK, hstack[hply].castle_queen[BLACK]);
	G->castled[WHITE] = hstack[hply].castled[WHITE];
	G->castled[BLACK] = hstack[hply].castled[BLACK];
	set_ep(hstack[hply].en_passant_x, hstack[hply].en_passant_y);
	G->inCheck[WHITE] = hstack[hply].inCheck[WHITE];
	G->inCheck[BLACK] = hstack[hply].inCheck[BLACK];

	assert(G->board[G->kingx[WHITE]][G->kingy[WHITE]] == WKING);
	assert(G->board[G->kingx[BLACK]][G->kingy[BLACK]] == BKING);
}

void undoMove() {
	hply--;
	ply--;
	unmark();

	G->turn = flipTurn(G->turn);
	G->zobrist ^= ZOBR_BLACK();

	_undoMove();
}
