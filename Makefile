
CC	= gcc 
CFLAGS	= -Wall -g -I/usr/include/ncurses
LDFLAGS	= -lncurses -pthread 
PROGS	= game

all: $(PROGS)

game: game.o othello.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o *~ $(PROGS)

