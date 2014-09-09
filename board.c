#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ai.h"
#include "legal.h"
#include "masks.h"
#include "piece-square.h"
#include "board.h"
#include "zobrist.h"
#include "ztable.h"
#include "succs.h"

static const char *init =
	"rnbqkbnr"
	"pppppppp"
	"........"
	"........"
	"........"
	"........"
	"PPPPPPPP"
	"RNBQKBNR"
	"W0011110000";

static void fix() {
	int i, j;

	G->idlecount = 0;
	G->pieceScore[BLACK] = 0;
	G->pieceScore[WHITE] = 0;
	G->zobrist = 0;
	G->piecemask[BLACK] = 0;
	G->piecemask[WHITE] = 0;

	for (i=0; i<10; i++) {
		G->pawn_rank[WHITE][i] = 0;
		G->pawn_rank[BLACK][i] = 7;
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			int piece = G->board[i][j];

			if (isKing(piece)) {
				G->kingx[colorOf(piece)] = i;
				G->kingy[colorOf(piece)] = j;
			}

			if (piece != EMPTY) {
				if (piece != WKING && piece != BKING)
					G->pieceScore[colorOf(piece)] += scoreOf(piece);
				G->zobrist ^= ZOBR_PIECE(piece, i, j);
				G->piecemask[colorOf(piece)] |=
					((u64)1) <<(i*8 + j);
			}

			if (piece == WPAWN) {
				if (i > G->pawn_rank[WHITE][j+1])
					G->pawn_rank[WHITE][j+1] = i;
			} else if (piece == BPAWN) {
				if (i < G->pawn_rank[BLACK][j+1])
					G->pawn_rank[BLACK][j+1] = i;
			}
		}
	}

	if (G->turn == BLACK) G->zobrist ^= ZOBR_BLACK();

	if (G->castle_king[WHITE])  G->zobrist ^= ZOBR_CASTLE_K(WHITE);
	if (G->castle_king[BLACK])  G->zobrist ^= ZOBR_CASTLE_K(BLACK);
	if (G->castle_queen[WHITE]) G->zobrist ^= ZOBR_CASTLE_Q(WHITE);
	if (G->castle_queen[BLACK]) G->zobrist ^= ZOBR_CASTLE_Q(BLACK);

	G->inCheck[BLACK] = -1;
	G->inCheck[WHITE] = -1;
	G->en_passant_x = -1;
	G->en_passant_y = -1;

	G->castled[WHITE] = 0;
	G->castled[BLACK] = 0;

	G->was_capture = false;
	G->was_promote = false;

	piecePosFullRecalc();
}

void startingGame() {
	ply = 0;
	fromstr(init);
}

void printBoard() {
	int i, j;
	char bbuf[200];
	int l = 0;

	dbg("(turn: %s)\n", G->turn == WHITE ? "WHITE" : "BLACK");
	for (i=0; i<8; i++) {
		l += sprintf(bbuf+l, "%i  ", 8-i);
		for (j=0; j<8; j++) {
			if (G->en_passant_x == i && G->en_passant_y == j)
				l += sprintf(bbuf+l, "!");
			else
				l += sprintf(bbuf+l, "%c", charOf(G->board[i][j]));

			l += sprintf(bbuf+l, " ");
		}
		dbg("%s\n", bbuf);
		l = 0;
	}
	dbg("\n");
	dbg("   a b c d e f g h\n");

	dbg("[ castle_king = %i %i \n", G->castle_king[WHITE], G->castle_king[BLACK]);
	dbg("[ castle_queen = %i %i \n", G->castle_queen[WHITE], G->castle_queen[BLACK]);
	dbg("[ kingx = %i %i \n", G->kingx[WHITE], G->kingx[BLACK]);
	dbg("[ kingy = %i %i \n", G->kingy[WHITE], G->kingy[BLACK]);
	dbg("[ en_passant = %i %i \n", G->en_passant_x, G->en_passant_y);
	dbg("[ inCheck = %i %i \n", G->inCheck[WHITE], G->inCheck[BLACK]);
	dbg("[ scores = %i %i\n", G->pieceScore[WHITE], G->pieceScore[BLACK]);
	dbg("[ pps o e = %i %i\n", G->pps_O, G->pps_E);
	dbg("[ zobrist = 0x%" PRIx64 "\n", G->zobrist);
	dbg("[ idlecount = %i\n", G->idlecount);
	dbg("[ piecemask[W] = 0x%.16" PRIx64 "\n", G->piecemask[WHITE]);
	dbg("[ piecemask[B] = 0x%.16" PRIx64 "\n", G->piecemask[BLACK]);
	tostr(bbuf);
	dbg("[ tostr = <%s>\n", bbuf);
	dbg("[ reps = %i\n", reps());

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
bool isDraw() {
	int r = reps();

	assert(r > 0);
	return r >= 3 || G->idlecount >= 100;
}

int isFinished() {
	int i, r = reps();

	assert(r > 0);
	assert(r <= 3);

	if (r == 3)
		return DRAW_3FOLD;
	else if (G->idlecount >= 100)
		return DRAW_50MOVE;

	assert(ply == 0);
	genSuccs();

	for (i = first_succ[ply]; i < first_succ[ply+1]; i++) {
		/*
		 * Si hay un sucesor válido,
		 * el juego no terminó
		 */
		if (doMove_unchecked(gsuccs[i].m)) {
			undoMove();
			goto not_finished;
		}
	}

	if (inCheck(G->turn))
		return WIN(flipTurn(G->turn));
	else
		return DRAW_STALE;

not_finished:
	return -1;
}

/*
 * Rompemos la simetría en el caso de los reyes,
 * para optimizar un poco.
 */
static bool inCheck_diag_sw(int kr, int kc, int who);
static bool inCheck_diag_se(int kr, int kc, int who);
static bool inCheck_diag_nw(int kr, int kc, int who);
static bool inCheck_diag_ne(int kr, int kc, int who);
static bool inCheck_row_w(int kr, int kc, int who);
static bool inCheck_row_e(int kr, int kc, int who);
static bool inCheck_col_n(int kr, int kc, int who);
static bool inCheck_col_s(int kr, int kc, int who);
static bool inCheck_knig(int kr, int kc, int who);
static bool inCheck_pawn(int kr, int kc, int who);
static bool inCheck_king();

bool inCheck(int who) {
	u8 kr, kc;

	if (G->inCheck[who] != -1)
		return G->inCheck[who];

	if (!(G->piecemask[flipTurn(who)] & all_mask[8*G->kingx[who] + G->kingy[who]])) {
		G->inCheck[who] = 0;
		return 0;
	}

	kr = G->kingx[who];
	kc = G->kingy[who];

	G->inCheck[who] =  inCheck_diag_sw(kr, kc, who)
			|| inCheck_diag_se(kr, kc, who)
			|| inCheck_diag_nw(kr, kc, who)
			|| inCheck_diag_ne(kr, kc, who)
			|| inCheck_row_w(kr, kc, who)
			|| inCheck_row_e(kr, kc, who)
			|| inCheck_col_n(kr, kc, who)
			|| inCheck_col_s(kr, kc, who)
			|| inCheck_knig(kr, kc, who)
			|| inCheck_pawn(kr, kc, who)
			|| inCheck_king();

	return G->inCheck[who];
}

static bool inCheck_row_e(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(G->piecemask[flipTurn(who)] & row_e_mask[kr*8+kc]))
		return false;

	i = kr;
	for (j=kc+1; j<8; j++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_row_w(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(G->piecemask[flipTurn(who)] & row_w_mask[kr*8+kc]))
		return false;

	i = kr;
	for (j=kc-1; j>=0; j--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_col_s(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(G->piecemask[flipTurn(who)] & col_s_mask[kr*8+kc]))
		return 0;

	j = kc;
	for (i=kr+1; i<8; i++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_col_n(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_r = who == WHITE ? BROOK : WROOK;

	if (!(G->piecemask[flipTurn(who)] & col_n_mask[kr*8+kc]))
		return 0;

	j = kc;
	for (i=kr-1; i>=0; i--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_r)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_sw(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(G->piecemask[flipTurn(who)] & diag_sw_mask[kr*8+kc]))
		return false;

	j = kc;
	for (i=kr+1, j=kc-1; i<8 && j>=0; i++, j--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_nw(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(G->piecemask[flipTurn(who)] & diag_nw_mask[kr*8+kc]))
		return false;

	for (i=kr-1, j=kc-1; i>=0 && j>=0; i--, j--) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_se(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(G->piecemask[flipTurn(who)] & diag_se_mask[kr*8+kc]))
		return false;

	for (i=kr+1, j=kc+1; i<8 && j<8; i++, j++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_diag_ne(int kr, int kc, int who) {
	int i, j;
	const piece_t enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const piece_t enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	if (!(G->piecemask[flipTurn(who)] & diag_ne_mask[kr*8+kc]))
		return false;

	for (i=kr-1, j=kc+1; i>=0 && j<8; i--, j++) {
		if (any_piece(i, j)) {
			if (G->board[i][j] == enemy_q
					|| G->board[i][j] == enemy_b)
				return true;

			break;
		}
	}

	return false;
}

static bool inCheck_knig(int kr, int kc, int who) {
	const piece_t enemy_kn = who == WHITE ? BKNIGHT : WKNIGHT;

	if (!(G->piecemask[flipTurn(who)] & knight_mask[kr*8+kc]))
		return false;

	/* Caballos */
	if (kr >= 2 && kc >= 1 && G->board[kr-2][kc-1] == enemy_kn) return true;
	if (kr <= 5 && kc >= 1 && G->board[kr+2][kc-1] == enemy_kn) return true;
	if (kr >= 2 && kc <= 6 && G->board[kr-2][kc+1] == enemy_kn) return true;
	if (kr <= 5 && kc <= 6 && G->board[kr+2][kc+1] == enemy_kn) return true;
	if (kr >= 1 && kc >= 2 && G->board[kr-1][kc-2] == enemy_kn) return true;
	if (kr <= 6 && kc >= 2 && G->board[kr+1][kc-2] == enemy_kn) return true;
	if (kr >= 1 && kc <= 5 && G->board[kr-1][kc+2] == enemy_kn) return true;
	if (kr <= 6 && kc <= 5 && G->board[kr+1][kc+2] == enemy_kn) return true;

	return false;
}

static bool inCheck_pawn(int kr, int kc, int who) {
	if (who == WHITE) {
		if (kr > 0) {
			if (kc > 0 && G->board[kr-1][kc-1] == BPAWN)
				return true;
			if (kc < 7 && G->board[kr-1][kc+1] == BPAWN)
				return true;
		}

		return 0;
	} else {
		if (kr < 7) {
			if (kc > 0 && G->board[kr+1][kc-1] == WPAWN)
				return true;
			if (kc < 7 && G->board[kr+1][kc+1] == WPAWN)
				return true;
		}

		return false;
	}
}

static bool inCheck_king(game g) {
	/* Simplemente viendo la distancia */

	return     abs(G->kingx[0] - G->kingx[1]) <= 1
		&& abs(G->kingy[0] - G->kingy[1]) <= 1;
}

static const int scoreTab[] = {
	[WPAWN]		= PAWN_SCORE,
	[WKNIGHT]	= KNIGHT_SCORE,
	[WBISHOP]	= BISHOP_SCORE,
	[WROOK]		= ROOK_SCORE,
	[WQUEEN]	= QUEEN_SCORE,
};

int scoreOf(int piece) {
	assert(piece != WKING);
	assert(piece != BKING);
	assert(piece != EMPTY);
	return scoreTab[piece & 7];
}

void tostr(char *s) {
	int i, j;
	char buf[10];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			*s++ = charOf(G->board[i][j]);
	}

	*s++ = G->turn == WHITE ? 'W' : 'B';

	sprintf(buf, "%02d", G->idlecount);
	assert(strlen(buf) == 2);
	strcpy(s, buf);
	s += 2;

	*s++ = G->castle_king[WHITE] ? '1' : '0';
	*s++ = G->castle_king[BLACK] ? '1' : '0';
	*s++ = G->castle_queen[WHITE] ? '1' : '0';
	*s++ = G->castle_queen[BLACK] ? '1' : '0';
	*s++ = G->castled[WHITE] ? '1' : '0';
	*s++ = G->castled[BLACK] ? '1' : '0';
	*s++ = G->en_passant_x + '1';
	*s++ = G->en_passant_y + '1';

	*s = 0;
}

void fromstr(const char *s) {
	int i, j;
	char buf[3];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			G->board[i][j] = pieceOf(*s++);
	}

	G->turn = *s++ == 'W' ? WHITE : BLACK;

	buf[0] = *s++;
	buf[1] = *s++;
	buf[2] = 0;

	G->idlecount = atoi(buf);

	G->castle_king[WHITE]  = *s++ == '1';
	G->castle_king[BLACK]  = *s++ == '1';
	G->castle_queen[WHITE] = *s++ == '1';
	G->castle_queen[BLACK] = *s++ == '1';
	G->castled[WHITE]      = *s++ == '1';
	G->castled[BLACK]      = *s++ == '1';
	G->en_passant_x        = *s++ - '1';
	G->en_passant_y        = *s++ - '1';

	fix();
}
