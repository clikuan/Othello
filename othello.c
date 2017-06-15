#include "othello.h"

#define	PLAYER1SYM	('O')
#define	PLAYER2SYM	('X')

int board[BOARDSZ][BOARDSZ];
struct BoardPlace* bp[BOARDSZ][BOARDSZ];

enum PlaceType{
	row = 0, 
	col = 1, 
	leftTop2RightDown = 2, 
	rightTop2LeftDown = 3, 
};
struct BoardPlace{
	enum PlaceType type;
	int currentRow;
	int currentCol;
	int takeRow;
	int takeCol;
	struct BoardPlace *next;
};

static int const box_top = 1;
static int const box_left = 2;
static int const boxwidth = 3;

static int use_color = 0;
static int colorborder;
static int colorplayer1;
static int colorplayer2;
static int colorcursor;
static int colormsgwarn;
static int colormsgok;

void
init_board() {
	bzero(board, sizeof(board));
	board[3][3] = board[4][4] = PLAYER1;
	board[3][4] = board[4][3] = PLAYER2;
}

void
init_colors() {
	int coloridx = 0;	// color idx 0 is default color
	if(has_colors() == FALSE)
		return;
	start_color();
	//
	colorborder = ++coloridx;
	init_pair(colorborder, COLOR_WHITE, COLOR_BLACK);

	colorplayer1 = ++coloridx;
	init_pair(colorplayer1, COLOR_BLACK, COLOR_GREEN);

	colorplayer2 = ++coloridx;
	init_pair(colorplayer2, COLOR_BLACK, COLOR_MAGENTA);

	colorcursor = ++coloridx;
	init_pair(colorcursor, COLOR_YELLOW, COLOR_BLACK);

	colormsgwarn = ++coloridx;
	init_pair(colormsgwarn, COLOR_RED, COLOR_BLACK);

	colormsgok = ++coloridx;
	init_pair(colormsgok, COLOR_GREEN, COLOR_BLACK);
	//
	use_color = 1;
	return;
}

static chtype
BCH(int x, int y) {
	if(board[y][x] == PLAYER1) return PLAYER1SYM|COLOR_PAIR(colorplayer1);
	if(board[y][x] == PLAYER2) return PLAYER2SYM|COLOR_PAIR(colorplayer2);
	return ' ';
}

static void
draw_box(int x, int y, int ch, int color, int highlight) {
	int i;
	attron(highlight ? A_BOLD : A_NORMAL);
	attron(COLOR_PAIR(color));
	//
	move(box_top + y*2 + 0, box_left + x*(boxwidth+1));
	if(y == 0) addch(x == 0 ? ACS_ULCORNER : ACS_TTEE);
	else       addch(x == 0 ? ACS_LTEE : ACS_PLUS);
	for(i = 0; i < boxwidth; i++) addch(ACS_HLINE);
	if(y == 0) addch(x+1 == BOARDSZ ? ACS_URCORNER : ACS_TTEE);
	else       addch(x+1 == BOARDSZ ? ACS_RTEE : ACS_PLUS);
	//
	move(box_top + y*2 + 1, box_left + x*(boxwidth+1));
	addch(ACS_VLINE);
	for(i = 0; i < boxwidth/2; i++) addch(' ');
	addch(ch);
	for(i = 0; i < boxwidth/2; i++) addch(' ');
	addch(ACS_VLINE);
	//
	move(box_top + y*2 + 2, box_left + x*(boxwidth+1));
	if(y+1 == BOARDSZ) addch(x == 0 ? ACS_LLCORNER : ACS_BTEE);
	else               addch(x == 0 ? ACS_LTEE : ACS_PLUS);
	for(i = 0; i < boxwidth; i++) addch(ACS_HLINE);
	if(y+1 == BOARDSZ) addch(x+1 == BOARDSZ ? ACS_LRCORNER : ACS_BTEE);
	else               addch(x+1 == BOARDSZ ? ACS_RTEE : ACS_PLUS);
	//
	attroff(COLOR_PAIR(color));
	attroff(highlight ? A_BOLD : A_NORMAL);
}

void
draw_message(const char *msg, int highlight) {
	move(0, 0);
	attron(highlight ? A_BLINK : A_NORMAL);
	attron(COLOR_PAIR(highlight ? colormsgwarn : colormsgok));
	printw(msg);
	attroff(COLOR_PAIR(highlight ? colormsgwarn : colormsgok));
	attroff(highlight ? A_BLINK : A_NORMAL);
	return;
}

void
draw_cursor(int x, int y, int show) {
	draw_box(x, y, BCH(x, y), show ? colorcursor : colorborder, show);
	return;
}

void
draw_gird(int x, int y, int playerColor){
	draw_box(x, y, BCH(x, y), colormsgok, 1);
	return;
}

void
draw_board() {
	int i, j;
	for(i = 0; i < BOARDSZ; i++) {
		for(j = 0; j < BOARDSZ; j++) {
			draw_box(i, j, BCH(i, j), colorborder, 0);
		}
	}
	return;
}

void
draw_score() {
	int i, j;
	int black = 0, white = 0;
	for(i = 0; i < BOARDSZ; i++) {
		for(j = 0; j < BOARDSZ; j++) {
			if(board[i][j] == PLAYER1) white++;
			if(board[i][j] == PLAYER2) black++;
		}
	}
	attron(A_BOLD);
	move(box_top+3, box_left + 4*BOARDSZ + 10);
	printw("Player #1 ");
	addch(PLAYER1SYM|COLOR_PAIR(colorplayer1));
	printw(" : %d", white);
	move(box_top+5, box_left + 4*BOARDSZ + 10);
	printw("Player #2 ");
	addch(PLAYER2SYM|COLOR_PAIR(colorplayer2));
	printw(" : %d", black);
	attroff(A_BOLD);
	return;
}
void 
drawCol(int i, int j, int player)
{
	int left = -1;
	int right = -1;
	int k, l;
	for(k = j-1; k >= 0; k--){
		if(board[i][k] == 0){
			left = 0;
			break;
		}
		else if(board[i][k] == player){
			left = 1;
			break;
		}
	}
	for(l = j+1; l < BOARDSZ; l++){
		if(board[i][l] == 0){
			right = 0;
			break;
		}
		else if(board[i][l] == player){
			right = 1;
			break;
		}
	}
	if(left == 0 && right == 1){
		draw_gird(k, i, 1);
	}
	else if(left == 1 && right == 0){
		draw_gird(l, i, 1);
	}
}
void 
drawRow(int i, int j, int player)
{
	int up = -1;
	int down = -1;
	int k, l;
	for(k = i-1; k >= 0; k--){
		if(board[k][j] == 0){
			up = 0;
			break;
		}
		else if(board[k][j] == player){
			up = 1;
			break;
		}
	}
	for(l = i+1; l < BOARDSZ; l++){
		if(board[l][j] == 0){
			down = 0;
			break;
		}
		else if(board[l][j] == player){
			down = 1;
			break;
		}
	}
	if(up == 0 && down == 1){
		draw_gird(j, k, 1);
	}
	else if(up == 1 && down == 0){
		draw_gird(j, l, 1);
	}
}
void 
drawDiagonalLeftTop2RightBottom(i, j, player)
{
	int left = -1;
	int right = -1;
	int k1, k2, l1, l2;
	for(k1 = i-1, k2 = j-1; k1 >= 0 && k2 >= 0; k1--, k2--){
		if(board[k1][k2] == 0){
			left = 0;
			break;
		}
		else if(board[k1][k2] == player){
			left = 1;
			break;
		}
	}
	for(l1 = i+1, l2 = j+1; l1 < BOARDSZ && l2 < BOARDSZ; l1++, l2++){
		if(board[l1][l2] == 0){
			right = 0;
			break;
		}
		else if(board[l1][l2] == player){
			right = 1;
			break;
		}
	}
	if(left == 0 && right == 1){
		draw_gird(k2, k1, 1);
	}
	else if(left == 1 && right == 0){
		draw_gird(l2, l1, 1);
	}
}
void 
drawDiagonalRightTop2LeftBottom(i, j, player)
{
	int left = -1;
	int right = -1;
	int k1, k2, l1, l2;
	for(k1 = i+1, k2 = j-1; k1 < BOARDSZ && k2 >= 0; k1++, k2--){
		if(board[k1][k2] == 0){
			left = 0;
			break;
		}
		else if(board[k1][k2] == player){
			left = 1;
			break;
		}
	}
	for(l1 = i-1, l2 = j+1; l1 >= 0 && l2 < BOARDSZ; l1--, l2++){
		if(board[l1][l2] == 0){
			right = 0;
			break;
		}
		else if(board[l1][l2] == player){
			right = 1;
			break;
		}
	}
	if(left == 0 && right == 1){
		draw_gird(k2, k1, 1);
	}
	else if(left == 1 && right == 0){
		draw_gird(l2, l1, 1);
	}
}	

void
placeCol(int i, int j, int player)
{
	int left = -1;
	int right = -1;
	int k, l;
	for(k = j-1; k >= 0; k--){
		if(board[i][k] == 0){
			left = 0;
			break;
		}
		else if(board[i][k] == player){
			left = 1;
			break;
		}
	}
	for(l = j+1; l < BOARDSZ; l++){
		if(board[i][l] == 0){
			right = 0;
			break;
		}
		else if(board[i][l] == player){
			right = 1;
			break;
		}
	}
	struct BoardPlace *insertItem;

	if(left == 0 && right == 1){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = i;
		insertItem -> currentCol = k;
		insertItem -> takeRow = i;		
		insertItem -> takeCol = l;
		insertItem -> type = col;
		insertItem -> next = NULL;
		if(bp[i][k] == NULL){
			bp[i][k] = insertItem;
		}
		else{
			struct BoardPlace *head;
			for(head = bp[i][k]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else if(left == 1 && right == 0){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = i;
		insertItem -> currentCol = l;
		insertItem -> takeRow = i;		
		insertItem -> takeCol = k;
		insertItem -> type = col;
		insertItem -> next = NULL;
		if(bp[i][l] == NULL){
			bp[i][l] = insertItem;
			//printf("%d %d\n",i,l);
		}
		else{
			struct BoardPlace *head;
			for(head = bp[i][l]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else{
		return;
	}
}
void
placeRow(int i, int j, int player)
{
	int up = -1;
	int down = -1;
	int k, l;
	for(k = i-1; k >= 0; k--){
		if(board[k][j] == 0){
			up = 0;
			break;
		}
		else if(board[k][j] == player){
			up = 1;
			break;
		}
	}
	for(l = i+1; l < BOARDSZ; l++){
		if(board[l][j] == 0){
			down = 0;
			break;
		}
		else if(board[l][j] == player){
			down = 1;
			break;
		}
	}
	struct BoardPlace *insertItem;

	if(up == 0 && down == 1){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = k;
		insertItem -> currentCol = j;
		insertItem -> takeRow = l;		
		insertItem -> takeCol = j;
		insertItem -> type = row;
		insertItem -> next = NULL;
		if(bp[k][j] == NULL){
			bp[k][j] = insertItem;
		}
		else{
			struct BoardPlace *head;
			for(head = bp[k][j]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else if(up == 1 && down == 0){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = l;
		insertItem -> currentCol = j;
		insertItem -> takeRow = k;		
		insertItem -> takeCol = j;
		insertItem -> type = row;
		insertItem -> next = NULL;
		if(bp[l][j] == NULL){
			bp[l][j] = insertItem;
			//printf("%d %d\n",i,l);
		}
		else{
			struct BoardPlace *head;
			for(head = bp[l][j]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else{
		return;
	}
}
void
placeLeftTop2RightDown(int i, int j, int player)
{
	int left = -1;
	int right = -1;
	int k1, k2, l1, l2;
	for(k1 = i-1, k2 = j-1; k1 >= 0 && k2 >= 0; k1--, k2--){
		if(board[k1][k2] == 0){
			left = 0;
			break;
		}
		else if(board[k1][k2] == player){
			left = 1;
			break;
		}
	}
	for(l1 = i+1, l2 = j+1; l1 < BOARDSZ && l2 < BOARDSZ; l1++, l2++){
		if(board[l1][l2] == 0){
			right = 0;
			break;
		}
		else if(board[l1][l2] == player){
			right = 1;
			break;
		}
	}
	struct BoardPlace *insertItem;

	if(left == 0 && right == 1){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = k1;
		insertItem -> currentCol = k2;
		insertItem -> takeRow = l1;		
		insertItem -> takeCol = l2;
		insertItem -> type = leftTop2RightDown;
		insertItem -> next = NULL;
		if(bp[k1][k2] == NULL){
			bp[k1][k2] = insertItem;
		}
		else{
			struct BoardPlace *head;
			for(head = bp[k1][k2]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else if(left == 1 && right == 0){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = l1;
		insertItem -> currentCol = l2;
		insertItem -> takeRow = k1;		
		insertItem -> takeCol = k2;
		insertItem -> type = leftTop2RightDown;
		insertItem -> next = NULL;
		if(bp[l1][l2] == NULL){
			bp[l1][l2] = insertItem;
			//printf("%d %d\n",i,l);
		}
		else{
			struct BoardPlace *head;
			for(head = bp[l1][l2]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else{
		return;
	}
}
void
placeRightTop2LeftDown(int i, int j, int player)
{
	int left = -1;
	int right = -1;
	int k1, k2, l1, l2;
	for(k1 = i+1, k2 = j-1; k1 < BOARDSZ && k2 >= 0; k1++, k2--){
		if(board[k1][k2] == 0){
			left = 0;
			break;
		}
		else if(board[k1][k2] == player){
			left = 1;
			break;
		}
	}
	for(l1 = i-1, l2 = j+1; l1 >= 0 && l2 < BOARDSZ; l1--, l2++){
		if(board[l1][l2] == 0){
			right = 0;
			break;
		}
		else if(board[l1][l2] == player){
			right = 1;
			break;
		}
	}
	struct BoardPlace *insertItem;

	if(left == 0 && right == 1){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = k1;
		insertItem -> currentCol = k2;
		insertItem -> takeRow = l1;		
		insertItem -> takeCol = l2;
		insertItem -> type = rightTop2LeftDown;
		insertItem -> next = NULL;
		if(bp[k1][k2] == NULL){
			bp[k1][k2] = insertItem;
		}
		else{
			struct BoardPlace *head;
			for(head = bp[k1][k2]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else if(left == 1 && right == 0){
		insertItem = malloc(sizeof(struct BoardPlace));
		insertItem -> currentRow = l1;
		insertItem -> currentCol = l2;
		insertItem -> takeRow = k1;		
		insertItem -> takeCol = k2;
		insertItem -> type = rightTop2LeftDown;
		insertItem -> next = NULL;
		if(bp[l1][l2] == NULL){
			bp[l1][l2] = insertItem;
			//printf("%d %d\n",i,l);
		}
		else{
			struct BoardPlace *head;
			for(head = bp[l1][l2]; head -> next != NULL; head = head -> next);
			head -> next = insertItem;
		}
	}
	else{
		return;
	}
}
int 
placePiece(int r, int c, int player)
{
	int i, j;
	int oppiste = (player == PLAYER1) ? PLAYER2 : PLAYER1;
	
	for(i = 0; i < BOARDSZ; i++)
		for(j = 0; j < BOARDSZ; j++)
			bp[i][j] = NULL;

	for(i = 0; i < BOARDSZ; i++){
		for(j = 0; j < BOARDSZ; j++){
			if(board[i][j] == oppiste){
				placeCol(i, j, player);
				placeRow(i, j, player);
				placeLeftTop2RightDown(i, j, player);
				placeRightTop2LeftDown(i, j, player);
			}
		}
	}
	if(bp[r][c] == NULL)
		return 0;
	
	struct BoardPlace *head = bp[r][c];
	while(head != NULL){
		int k; 
		int e;
		int k1,k2,e1,e2;
		switch(head -> type){
			case row:
					k = (head -> currentRow >  head -> takeRow) ? 
						head -> takeRow : head -> currentRow;
					e = (head -> currentRow >  head -> takeRow) ? 
						head -> currentRow : head -> takeRow;
					for(k=k+1; k < e; k++){
						board[k][c] = player;
					}
					break;
			case col:	
					k = (head -> currentCol >  head -> takeCol) ? 
						head -> takeCol : head -> currentCol;
					e = (head -> currentCol >  head -> takeCol) ? 
						head -> currentCol : head -> takeCol;
					for(k=k+1; k < e; k++){
						board[r][k] = player;
					}
					break;
			case leftTop2RightDown:
					k1 = (head -> currentRow >  head -> takeRow) ? 
						head -> takeRow : head -> currentRow;
					k2 = (head -> currentCol >  head -> takeCol) ? 
						head -> takeCol : head -> currentCol;
					e1 = (head -> currentRow >  head -> takeRow) ? 
						head -> currentRow : head -> takeRow;
					e2 = (head -> currentCol >  head -> takeCol) ? 
						head -> currentCol : head -> takeCol;
					for(k1=k1+1,k2=k2+1; k1 < e1 && k2 < e2; k1++,k2++){
						board[k1][k2] = player;
					}
					break;
			case rightTop2LeftDown:
					k1 = (head -> currentRow >  head -> takeRow) ? 
						head -> takeRow : head -> currentRow;
					k2 = (head -> currentCol >  head -> takeCol) ? 
						head -> currentCol : head -> takeCol;
					e1 = (head -> currentRow >  head -> takeRow) ? 
						head -> currentRow : head -> takeRow;
					e2 = (head -> currentCol >  head -> takeCol) ? 
						head -> takeCol : head -> currentCol;
					for(k1=k1+1,k2=k2-1; k1 < e1 && k2 >= e2; k1++,k2--){
						board[k1][k2] = player;
					}
					break;
		}
		head = head -> next;
	}
	return 1;
}
void
markGirdToPlacePiece(int player)
{
	int i, j;
	int oppiste = (player == PLAYER1) ? PLAYER2 : PLAYER1;
	for(i = 0; i < BOARDSZ; i++){
		for(j = 0; j < BOARDSZ; j++){
			if(board[i][j] == oppiste){
				drawCol(i, j, player);
				drawRow(i, j, player);
				drawDiagonalLeftTop2RightBottom(i, j, player);
				drawDiagonalRightTop2LeftBottom(i, j, player);
			}
		}
	}
}

