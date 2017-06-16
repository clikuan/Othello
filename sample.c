#include "othello.h"
#include <getopt.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

static int width;
static int height;
static int cx = 3;
static int cy = 3;

int sockfd;
int start = 0;
int currentTurn = PLAYER1;
int isServer = -1;
char sendData[25];
int updateHeaderMsg = 0;
void
userPutPieceDone(int signo)
{	
	send(sockfd,sendData, sizeof(sendData),0);
}
void* 
conn(void *address)
{	char buffer[50];
	char *addr = (char *)address;
	struct sockaddr_in sAddr;
	char *IP = strtok(addr, ":");
    char *port = strtok (NULL, ":");   
    printf("%s %s\n",IP, port);
	inet_aton(IP, &sAddr.sin_addr);
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(atoi(port));
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(connect(sockfd, (struct sockaddr *)&sAddr, sizeof(sAddr)) == -1){
		if(errno == ECONNREFUSED)
			printf("ECONNREFUSED\n");
	}
	
	while(1){
		recv(sockfd, buffer, sizeof(buffer),0);
		if(strcmp(buffer,"start") == 0){
			start = 1;
		}
		else if(strcmp(buffer, "opponentSkip") == 0){
			currentTurn = PLAYER2;
		}
		else{
			char *row = strtok(buffer, ":");
			char *column = strtok(NULL, ":");
			int r = atoi(row);
			int c = atoi(column);
			placePiece(r,c,PLAYER1);
			board[r][c] = PLAYER1;
			currentTurn = PLAYER2;
		}
	}
}
void* 
serve(void *port)
{	
	char buffer[50];
	char *p = (char *)port;
 	struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(p));
    addr.sin_addr.s_addr = INADDR_ANY;
    printf("Waiting for a client on port %s\n", p);
    int serSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(serSockfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(serSockfd, 1);
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
	sockfd = accept(serSockfd, (struct sockaddr*)&client_addr, &addrlen);
	start = 1;
	/* Send start */
	strcpy(buffer,"start");
	send(sockfd, buffer, sizeof(buffer), 0);
	while(1){
		recv(sockfd, buffer, sizeof(buffer),0);
		if(strcmp(buffer, "opponentSkip") == 0){
			currentTurn = PLAYER1;
			continue;
		}
		char *row = strtok(buffer, ":");
		char *column = strtok(NULL, ":");
		int r = atoi(row);
		int c = atoi(column);
		placePiece(r,c,PLAYER2);
		board[r][c] = PLAYER2;
		currentTurn = PLAYER1;
	}
}
void 
printHeaderMsg()
{
	attron(A_BOLD);
	move(0, 0);	
	if(currentTurn == PLAYER1 && isServer == 1)
		printw("Player #1 It's my turn\n");
	else if(currentTurn == PLAYER2 && isServer == 0)
		printw("Player #2 It's my turn\n");
	else if(currentTurn == PLAYER1 && isServer == 0)
		printw("Player #2 Waiting for peer\n");
	else if(currentTurn == PLAYER2 && isServer == 1)
		printw("Player #1 Waiting for peer\n");
	attroff(A_BOLD);
}
int 
myTurn()
{
	if((currentTurn == PLAYER1 && isServer == 1) || (currentTurn == PLAYER2 && isServer == 0))
		return 1;
	else
		return 0;
}

int
main(int argc, char *argv[])
{	char option;
	char address[22];
	char port[6];
    while ((option = getopt(argc, argv,"c:s:")) != -1) {
        switch (option) {
            case 'c' :
            	if(isServer != -1){
            		fprintf(stderr, "can not specify -s and -c at the same time\n");
            	}
            	else{
            		isServer = 0;
            		strcpy(address,optarg);
            	}
                break;
            case 's' : 
            	if(isServer != -1){
            		fprintf(stderr, "can not specify -s and -c at the same time\n");
            	}
            	else{
            		isServer = 1;
            		strcpy(port,optarg);
            	}
            	break;
            default:
            	break;
        }
    }
    if(isServer == -1){
    	fprintf(stderr, "must specify argument\n");
    	return -1;
    }
    	//init_board();
        //placePiece(2, 4, PLAYER1);
        //return 1;
    // create a thread for communicate.
    pthread_t comThread;
    if(isServer)
    	pthread_create(&comThread, NULL, serve, port);
    else
    	pthread_create(&comThread, NULL, conn, address);
	
	// block main thread SIGUSR1 signal.
	sigset_t sigs_to_block;
	sigemptyset(&sigs_to_block);
	sigaddset(&sigs_to_block, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &sigs_to_block, NULL);
    signal(SIGUSR1, userPutPieceDone);

    // block untill game start.
    while(!start);

	initscr();			// start curses mode 
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
					// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);			// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

	init_colors();


	cx = cy = 3;
	init_board();
redraw:
	clear();
	printHeaderMsg();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();
	attron(A_BOLD);
	move(height-1, 0);	
	printw("Arrow keys: move; Space/Return: put; Q: quit");
	attroff(A_BOLD);
	while(true) {			// main loop
		if(!myTurn()){
			continue;
		}
		if(checkPlayerEnd(PLAYER1) && checkPlayerEnd(PLAYER2)){ //game over
			draw_board();
			draw_cursor(cx, cy, 1);
			draw_score();
			printGameResult(currentTurn);
			refresh();
			currentTurn = (currentTurn == PLAYER1) ? PLAYER2 : PLAYER1;
			continue;
		}
		else if((checkPlayerEnd(PLAYER1) && (currentTurn == PLAYER1) && (isServer)) || 
				(checkPlayerEnd(PLAYER2) && (currentTurn == PLAYER2) && (!isServer))){
			draw_board();
			draw_cursor(cx, cy, 1);
			draw_score();
			strcpy(sendData,"opponentSkip");
			refresh();
			kill(getpid(),SIGUSR1);
			currentTurn = (currentTurn == PLAYER1) ? PLAYER2 : PLAYER1;
			continue;
		}
		markGirdToPlacePiece(currentTurn);
		if(!updateHeaderMsg){
			updateHeaderMsg = 1;
			goto redraw;
		}
		int ch = getch();
		int moved = 0;
		switch(ch) {
			case ' ':
				if(board[cy][cx] != 0 )
					break;
				if(placePiece(cy, cx, currentTurn) == 0)
					break;
				board[cy][cx] = currentTurn;
				sprintf(sendData,"%d:%d",cy,cx);
				draw_cursor(cy, cx, 1);
				draw_score();
				refresh();
				currentTurn = (currentTurn == PLAYER1) ? PLAYER2 : PLAYER1;
				kill(getpid(),SIGUSR1);
				updateHeaderMsg = 0;
				goto redraw;
				break;
			case 0x0d:
			case 0x0a:
			case KEY_ENTER:
				//printw("%d %d\n",cx,cy);
				if(board[cy][cx] != 0)
					break;
				if(placePiece(cy, cx, currentTurn) == 0)
					break;
				board[cy][cx] = currentTurn;
				sprintf(sendData,"%d:%d",cy,cx);
				draw_cursor(cy, cx, 1);
				draw_score();
				refresh();
				kill(getpid(),SIGUSR1);
				updateHeaderMsg = 0;
				if(checkPlayerEnd(PLAYER1) && checkPlayerEnd(PLAYER2)){ //game over
					printGameResult(currentTurn);
					refresh();
					continue;
				}
				currentTurn = (currentTurn == PLAYER1) ? PLAYER2 : PLAYER1;
				goto redraw;
				break;
			case 'q':
			case 'Q':
				goto quit;
				break;
			case 'r':
			//case 'R':
			//	goto restart;
			//	break;
			case 'k':
			case KEY_UP:
				draw_cursor(cx, cy, 0);
				cy = (cy-1+BOARDSZ) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'j':
			case KEY_DOWN:
				draw_cursor(cx, cy, 0);
				cy = (cy+1) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'h':
			case KEY_LEFT:
				draw_cursor(cx, cy, 0);
				cx = (cx-1+BOARDSZ) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'l':
			case KEY_RIGHT:
				draw_cursor(cx, cy, 0);
				cx = (cx+1) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
		}

		if(moved) {
			refresh();
			moved = 0;
		}

		napms(1);		// sleep for 1ms
	}

quit:
	endwin();			// end curses mode


	return 0;
}
