#ifndef __OTHELLO_H__
#define	__OTHELLO_H__

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#define	BOARDSZ		8
#define	PLAYER1		+1
#define	PLAYER2		-1

extern int board[BOARDSZ][BOARDSZ];

void init_board();
void init_colors();

void draw_message(const char *msg, int highlight);
void draw_cursor(int x, int y, int hide);
void draw_gird(int x, int y, int playerColor);
void draw_board();
void draw_score();

void markGirdToPlacePiece(int player);
int placePiece(int r, int c, int player);
void printGameResult(int player);
int checkPlayerEnd(int player);
#endif	/* __OTHELLO_H__ */
