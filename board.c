#include "board.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "move.h"
#include "piece-square.h"
#include "mem.h"

#include "ai.h" // BORRAR!!

char charOf(int piece);

static int scoreOf(int piece);
static int absoluteScoreOf(int piece);

static int doMoveRegular(game g, move m);
static int doMoveKCastle(game g, move m);
static int doMoveQCastle(game g, move m);

void pr_board(game g) {
	int i;
	fprintf(stderr, "BOARD: [");
	for(i=0; i<64; i++)
		fprintf(stderr, "%i,", g->board[i/8][i%8]);
	fprintf(stderr, "]\n");
}

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
};
#else
static const struct game_struct
init = {
	.board= {
		{ EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, BKING },
		{ EMPTY, EMPTY, BPAWN, EMPTY, EMPTY, BPAWN, EMPTY, EMPTY },
		{ EMPTY, EMPTY, EMPTY, BBISHOP, EMPTY, EMPTY, EMPTY, BPAWN },
		{ BPAWN, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY },
		{ WPAWN, EMPTY, BPAWN, EMPTY, EMPTY, EMPTY, WROOK, WKING },
		{ EMPTY, EMPTY, WPAWN, BBISHOP, EMPTY, BROOK, EMPTY, WPAWN },
		{ EMPTY, WPAWN, EMPTY, EMPTY, BROOK, EMPTY, EMPTY, EMPTY },
		{ WROOK, EMPTY, WBISHOP, WKNIGHT, EMPTY, EMPTY, EMPTY, EMPTY }
	},
	.turn = WHITE,
	.lastmove = { 0 },
	.idlecount = 0,
	.castle_king = { 1, 1 },
	.castle_queen = { 1, 1 },
};
#endif

static void fix(game g) {
	int i, j;

	g->pieceScore = 0;
	g->totalScore = 0;

	for (i=0; i<8; i++) {
		for (j=0; j<8; j++) {
			int piece = g->board[i][j];

			if (isKing(piece)) {
				g->kingx[colorOf(piece)] = i;
				g->kingy[colorOf(piece)] = j;
			}

			g->pieceScore += scoreOf(piece);
			g->totalScore += absoluteScoreOf(piece);
		}
	}

	g->inCheck[BLACK] = -1;
	g->inCheck[WHITE] = -1;
	g->en_passant_x = -1;
	g->en_passant_y = -1;
	
	g->hasNext = -1;
	g->nSucc = -1;

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

	fprintf(stderr, "(turn: %s)\n", g->turn == WHITE ? "WHITE" : "BLACK");
	for (i=0; i<8; i++) {
		fprintf(stderr, "   ");
		for (j=0; j<8; j++) {
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			fputc(' ', stderr);
			fputc(' ', stderr);
			fputc(' ', stderr);
		}
		fputc('\n', stderr);
		fprintf(stderr, "%i  ", 8-i);
		for (j=0; j<8; j++) {
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			if (g->en_passant_x == i && g->en_passant_y == j)
				fputc('!', stderr);
			else
				fputc(charOf(g->board[i][j]), stderr);
			fputc(' ', stderr);
			fputc(' ', stderr);
			fputc(' ', stderr);
		}
		fputc('\n', stderr);
		fputc('\n', stderr);
	}

	fprintf(stderr, "\n    a     b     c     d     e     f     g     h\n");

	fprintf(stderr, "[ castle_king = %i %i \n", g->castle_king[0], g->castle_king[1]);
	fprintf(stderr, "  castle_queen = %i %i \n", g->castle_queen[0], g->castle_queen[1]);
	fprintf(stderr, "  kingx = %i %i \n", g->kingx[0], g->kingx[1]);
	fprintf(stderr, "  kingy = %i %i \n", g->kingy[0], g->kingy[1]);
	fprintf(stderr, "  en_passant = %i %i \n", g->en_passant_x, g->en_passant_y);
	fprintf(stderr, "  inCheck = %i %i \n", g->inCheck[0], g->inCheck[1]);
	fprintf(stderr, "  scores = %i %i\n", g->pieceScore, g->totalScore);
	fprintf(stderr, "  pps o e = %i %i\n", g->pps_O, g->pps_E);
	
	fprintf(stderr," heur is = %i\n", heur(g));
	fprintf(stderr, "]\n");


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

int isFinished(game g) {
	if (g->idlecount >= 50)
		return DRAW;
	else if (hasNextGame(g))
		return -1;
	else if (inCheck(g, g->turn))
		return WIN(flipTurn(g->turn)); /* Jaque mate al jugador actual */
	else
		return DRAW; /* Ahogado (Stalemate) */
}

static int inCheck_diag(game g, int kr, int kc, int who);
static int inCheck_line(game g, int kr, int kc, int who);
static int inCheck_knig(game g, int kr, int kc, int who);
static int inCheck_pawn(game g, int kr, int kc, int who);
static int inCheck_king(game g, int kr, int kc, int who);

int inCheck(game g, int who) {
	int kr, kc;

	if (g->inCheck[who] != -1)
		return g->inCheck[who];

	kr = g->kingx[who];
	kc = g->kingy[who];

	g->inCheck[who] = inCheck_diag(g, kr, kc, who)
	                || inCheck_line(g, kr, kc, who)
			|| inCheck_knig(g, kr, kc, who)
			|| inCheck_pawn(g, kr, kc, who)
			|| inCheck_king(g, kr, kc, who);

	return g->inCheck[who];
}

static int inCheck_line(game g, int kr, int kc, int who) {
	int i, j;
	const int enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const int enemy_r = who == WHITE ? BROOK : WROOK;

	/* Columna */
	j = kc;
	for (i=kr+1; i<8; i++)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_r)
					return 1;

			break;
		}

	for (i=kr-1; i>=0; i--)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_r)
					return 1;

			break;
		}

	/* Fila */
	i = kr;
	for (j=kc+1; j<8; j++)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_r)
					return 1;

			break;
		}

	for (j=kc-1; j>=0; j--)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_r)
					return 1;

			break;
		}

	return 0;
}

static int inCheck_diag(game g, int kr, int kc, int who) {
	int i, j;
	const int enemy_q = who == WHITE ? BQUEEN : WQUEEN;
	const int enemy_b = who == WHITE ? BBISHOP : WBISHOP;

	/* Diagonales */
	for (i=kr-1, j=kc-1; i>=0 && j>=0; i--, j--)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_b)
					return 1;

			break;
		}

	for (i=kr+1, j=kc+1; i<8 && j<8; i++, j++)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_b)
					return 1;

			break;
		}

	for (i=kr+1, j=kc-1; i<8 && j>=0; i++, j--)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_b)
					return 1;

			break;
		}

	for (i=kr-1, j=kc+1; i>=0 && j<8; i--, j++)
		if (g->board[i][j] != 0) {
			if (colorOf(g->board[i][j]) != who)
				if (g->board[i][j] == enemy_q || g->board[i][j] == enemy_b)
					return 1;

			break;
		}

	return 0;
}

static int inCheck_knig(game g, int kr, int kc, int who) {
	const int enemy_kn = who == WHITE ? BKNIGHT : WKNIGHT;

	/* Caballos */
	if (kr >= 2 && kc >= 1 && g->board[kr-2][kc-1] == enemy_kn) return 1;
	if (kr <= 5 && kc >= 1 && g->board[kr+2][kc-1] == enemy_kn) return 1;
	if (kr >= 2 && kc <= 6 && g->board[kr-2][kc+1] == enemy_kn) return 1;
	if (kr <= 5 && kc <= 6 && g->board[kr+2][kc+1] == enemy_kn) return 1;
	if (kr >= 1 && kc >= 2 && g->board[kr-1][kc-2] == enemy_kn) return 1;
	if (kr <= 6 && kc >= 2 && g->board[kr+1][kc-2] == enemy_kn) return 1;
	if (kr >= 1 && kc <= 5 && g->board[kr-1][kc+2] == enemy_kn) return 1;
	if (kr <= 6 && kc <= 5 && g->board[kr+1][kc+2] == enemy_kn) return 1;

	return 0;
}

static int inCheck_pawn(game g, int kr, int kc, int who) {
	if (who == WHITE) {
		if (kr > 0) {
			if (kc > 0 && g->board[kr-1][kc-1] == BPAWN)
				return 1;
			if (kc < 7 && g->board[kr-1][kc+1] == BPAWN)
				return 1;
		}

		return 0;
	} else {
		if (kr < 7) {
			if (kc > 0 && g->board[kr+1][kc-1] == WPAWN)
				return 1;
			if (kc < 7 && g->board[kr+1][kc+1] == WPAWN)
				return 1;
		}

		return 0;
	}
}

static int inCheck_king(game g, int kr, int kc, int who) {
	const int dr[] = {  1, 1, 1, 0, -1, -1, -1,  0 };
	const int dc[] = { -1, 0, 1, 1,  1,  0, -1, -1 };
	const int enemy_k = who == WHITE ? BKING : WKING;

	int i;

	for (i = 0; i < sizeof dr / sizeof dr[0]; i++) {
		int R = kr + dr[i];
		int C = kc + dc[i];

		if (R >= 0 && R < 8 && C >= 0 && C < 8)
			if (g->board[R][C] == enemy_k)
				return 1;
	}

	return 0;
}

void freeSuccs(game *arr, int len) {
	int i;

	for (i=0; i<len; i++)
		freeGame(arr[i]);

	free(arr);
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
	if (r == kr || c == kc) {
		return 0;
	} else {
		int dx, dy;

		dx = abs(r-kr);
		dy = abs(c-kc);

		if (dx == dy || dx + dy == 3)
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

		g->en_passant_x = -1;
		g->en_passant_y = -1;
		g->idlecount = 0;
		g->lastmove = m;

		break;
	
	case MOVE_QUEENSIDE_CASTLE:
		if (!doMoveQCastle(g, m))
			goto fail;

		g->en_passant_x = -1;
		g->en_passant_y = -1;
		g->idlecount = 0;
		g->lastmove = m;

		break;
	default:
		assert(0);
	}

	/* Nunca podemos quedar en jaque */
	if (g->inCheck[g->turn] == 1  ||
		(g->inCheck[g->turn] == -1 && inCheck(g, g->turn)))
		goto fail;

	freeGame(old_g);

	g->turn = flipTurn(g->turn);
	g->hasNext = -1;
	g->nSucc = -1;

	return 1;

fail:
	memcpy(g, old_g, sizeof *g);
	freeGame(old_g);

	return 0;
}

/* Auxiliares de doMoveRegular */
static void setPiece(game g, int r, int c, int piece);
static int isValid(game g, move m);
static void updKing(game g, move m);
static void updCastling(game g, move m);
static void epCapture(game g, move m);
static void epCalc(game g, move m);
static void calcPromotion(game g, move m);

static void setPiece(game g, int r, int c, int piece) {
	int old_piece = g->board[r][c];
	if (old_piece != 0) {
		g->pieceScore -= scoreOf(old_piece);
		g->totalScore -= absoluteScoreOf(old_piece);
		g->pps_O      -= piece_square_val_O(old_piece, r, c);
		g->pps_E      -= piece_square_val_E(old_piece, r, c);
	}

	g->board[r][c] = piece;

	if (piece) {
		g->pps_E      += piece_square_val_E(piece, r, c);
		g->pps_O      += piece_square_val_O(piece, r, c);
		g->totalScore += absoluteScoreOf(piece);
		g->pieceScore += scoreOf(piece);
	}
}

static int doMoveRegular(game g, move m) {
	int piece = g->board[m.r][m.c];
	int other = flipTurn(g->turn);

	if (!isValid(g, m))
		return 0;

	/* Es válida */
	memcpy(&g->lastmove, &m, sizeof m);
	g->idlecount++;

	updKing(g, m);
	updCastling(g, m);

	/* Los peones no son reversibles */
	if (isPawn(piece))
		g->idlecount = 0;

	/* Actuar si es una captura al paso */
	epCapture(g, m);

	/* Recalcular en passant */
	epCalc(g, m);

	if (g->board[m.R][m.C] != 0) {
		g->idlecount = 0;
		g->lastmove.was_capture = 1;
	} else {
		g->lastmove.was_capture = 0;
	}

	setPiece(g, m.R, m.C, piece);
	setPiece(g, m.r, m.c, 0);

	calcPromotion(g, m);

	/* Si es algún movimiento relevante al rey contrario
	 * dropeamos la cache */
	if (g->inCheck[other] != -1)
		if (!safe(g, m.r, m.c, g->kingx[other], g->kingy[other]) ||
		    !safe(g, m.R, m.C, g->kingx[other], g->kingy[other]))
			g->inCheck[other] = -1;

	/* Necesitamos también (posiblemente) dropear la nuestra */
	if (g->inCheck[m.who] != -1)
		if (isKing(piece) ||
		    !safe(g, m.r, m.c, g->kingx[m.who], g->kingy[m.who]) ||
		    !safe(g, m.R, m.C, g->kingx[m.who], g->kingy[m.who]))
			g->inCheck[m.who] = -1;

	return 1;
}

static int isValid(game g, move m) {
	int piece = g->board[m.r][m.c];

	/* Siempre se mueve una pieza propia */
	if (piece == 0 || colorOf(piece) != g->turn)
		return 0;

	/* La pieza debe poder moverse al destino */
	if (m.r < 0 || m.r > 7
		|| m.c < 0 || m.c > 7
		|| m.R < 0 || m.R > 7
		|| m.C < 0 || m.C > 7
		|| (m.r == m.R && m.c == m.C)
		|| !canMove(g, m.r, m.c, m.R, m.C))
		return 0;

	/* Es un peón que promueve? */
	if (isPawn(piece)
			&& m.R == (m.who == WHITE ? 0 : 7)) {
		if (m.promote == 0) {
			fprintf(stderr, "Esa movida requiere una promoción!!!\n");
			fflush(NULL);
			abort();
			return 0;
		}
	}

	return 1;
}

static void updKing(game g, move m) {
	int piece = g->board[m.r][m.c];

	if (isKing(piece)) {
		g->kingx[colorOf(piece)] = m.R;
		g->kingy[colorOf(piece)] = m.C;
		g->castle_king[m.who] = 0;
		g->castle_queen[m.who] = 0;
	}
}

static void updCastling(game g, move m) {
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
}

static void epCapture(game g, move m) {
	int piece = g->board[m.r][m.c];

	if (isPawn(piece)
			&& m.R == g->en_passant_x
			&& m.C == g->en_passant_y) {
		setPiece(g, m.r, m.C, 0);
		g->inCheck[WHITE] = -1;
		g->inCheck[BLACK] = -1;
	}
}

static void epCalc(game g, move m) {
	int piece = g->board[m.r][m.c];

	if (isPawn(piece) && abs(m.r - m.R) == 2) {
		assert (m.c == m.C);
		g->en_passant_x = (m.r+m.R)/2;
		g->en_passant_y = m.c;
	} else {
		g->en_passant_x = -1;
		g->en_passant_y = -1;
	}
}

static void calcPromotion(game g, move m) {
	/* se llama a calcPromotion LUEGO de hacer la movida,
	 * por lo tanto usamos m.R y m.C */
	int piece = g->board[m.R][m.C];

	/* Es un peón que promueve? */
	if (isPawn(piece)
			&& m.R == (m.who == WHITE ? 0 : 7)) {
		int new_piece = m.who == WHITE ? m.promote : -m.promote;

		setPiece(g, m.R, m.C, new_piece);
		g->lastmove.was_promotion = 1;
	} else {
		g->lastmove.was_promotion = 0;
	}
}

static int doMoveKCastle(game g, move m) {
	const int rank = m.who == WHITE ? 7 : 0;
	const int kpiece = m.who == WHITE ? WKING : BKING;
	const int rpiece = m.who == WHITE ? WROOK : BROOK;

	if (!(g->castle_king[m.who] && ! inCheck(g, m.who)
		&& g->board[rank][7] == rpiece && g->board[rank][6] == EMPTY
		&& g->board[rank][5] == EMPTY  && g->board[rank][4] == kpiece)) {

		return 0;
	}

	{
		game tg;

		tg = copyGame(g);
		tg->board[rank][4] = 0;
		tg->board[rank][5] = kpiece;
		tg->kingy[m.who] = 5;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return 0;
		}

		tg->board[rank][5] = 0;
		tg->board[rank][6] = kpiece;
		tg->kingy[m.who] = 6;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return 0;
		}

		freeGame(tg);
	}

	g->castle_king[m.who] = 0;
	g->castle_queen[m.who] = 0;

	/* Dropeamos la cache de jaque */
	g->inCheck[0] = -1;
	g->inCheck[1] = -1;

	setPiece(g, rank, 4, 0);
	setPiece(g, rank, 5, rpiece);
	setPiece(g, rank, 6, kpiece);
	setPiece(g, rank, 7, 0);

	g->kingx[m.who] = rank;
	g->kingy[m.who] = 6;

	return 1;
}

static int doMoveQCastle(game g, move m) {
	const int rank = m.who == WHITE ? 7 : 0;
	const int kpiece = m.who == WHITE ? WKING : BKING;
	const int rpiece = m.who == WHITE ? WROOK : BROOK;

	if (!(g->castle_queen[m.who] && ! inCheck(g, m.who)
		&& g->board[rank][0] == rpiece && g->board[rank][1] == EMPTY
		&& g->board[rank][2] == EMPTY  && g->board[rank][3] == EMPTY
		&& g->board[rank][4] == kpiece)) {

		return 0;
	}

	{
		game tg;

		tg = copyGame(g);
		tg->board[rank][4] = 0;
		tg->board[rank][3] = kpiece;
		tg->kingy[m.who] = 3;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return 0;
		}

		tg->board[rank][3] = 0;
		tg->board[rank][2] = kpiece;
		tg->kingy[m.who] = 2;
		tg->inCheck[m.who] = -1;

		if (inCheck(tg, m.who)) {
			freeGame(tg);
			return 0;
		}

		freeGame(tg);
	}

	g->castle_queen[m.who] = 0;
	g->castle_king[m.who] = 0;

	/* Dropeamos la cache de jaque */
	g->inCheck[0] = -1;
	g->inCheck[1] = -1;

	setPiece(g, rank, 0, 0);
	setPiece(g, rank, 1, 0);
	setPiece(g, rank, 2, kpiece);
	setPiece(g, rank, 3, rpiece);
	setPiece(g, rank, 4, 0);
			
	g->kingx[m.who] = rank;
	g->kingy[m.who] = 2;

	return 1;
}

static const int __scoreTab[] =
{
	[6+BKING] = -20000,
	[6+BQUEEN] = -900,
	[6+BROOK] = -500,
	[6+BBISHOP] = -330,
	[6+BKNIGHT] = -320,
	[6+BPAWN] = -100,
	[6+0] = 0,
	[6+WKING] = 20000,
	[6+WQUEEN] = 900,
	[6+WROOK] = 500,
	[6+WBISHOP] = 330,
	[6+WKNIGHT] = 320,
	[6+WPAWN] = 100
};

static const int * scoreTab = &__scoreTab[6];

static int scoreOf(int piece) {
	return scoreTab[piece];
}

static int absoluteScoreOf(int piece) {
	return abs(scoreTab[piece]);
}
