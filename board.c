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
#include "moves.h"
#include "check.h"

static const char *init =
	"rnbqkbnr"
	"pppppppp"
	"........"
	"........"
	"........"
	"........"
	"PPPPPPPP"
	"RNBQKBNR"
	"W00011110000";

static void fix() {
	int i, j;

	G->idlecount = 0;
	G->pieceScore[BLACK] = 0;
	G->pieceScore[WHITE] = 0;
	G->zobrist = 0;
	G->piecemask[BLACK] = 0;
	G->piecemask[WHITE] = 0;

	for (i = 0; i < 10; i++) {
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
				G->piecemask[colorOf(piece)] |= posbit(i, j);
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
	for (i = 0; i < 8; i++) {
		l += sprintf(bbuf+l, "%i  ", 8-i);
		for (j = 0; j < 8; j++) {
			if (G->en_passant_x == i && G->en_passant_y == j)
				l += sprintf(bbuf + l, "!");
			else
				l += sprintf(bbuf + l, "%c", charOf(G->board[i][j]));

			l += sprintf(bbuf + l, " ");
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

/*
 * Doesn't use succesor info, so it's fast and
 * suitable to use in the searches
 */
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

	if (inCheck())
		return WIN(flipTurn(G->turn));
	else
		return DRAW_STALE;

not_finished:
	return -1;
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
	return scoreTab[toWhite(piece)];
}

void tostr(char *s) {
	int i, j;
	char buf[10];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			*s++ = charOf(G->board[i][j]);
	}

	*s++ = G->turn == WHITE ? 'W' : 'B';

	sprintf(buf, "%03d", G->idlecount);
	assert(strlen(buf) == 3);
	strcpy(s, buf);
	s += 3;

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
	char buf[4];

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++)
			G->board[i][j] = pieceOf(*s++);
	}

	G->turn = *s++ == 'W' ? WHITE : BLACK;

	buf[0] = *s++;
	buf[1] = *s++;
	buf[2] = *s++;
	buf[3] = 0;

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
