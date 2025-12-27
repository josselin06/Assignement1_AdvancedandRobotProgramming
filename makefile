PROGRAMS = Init B D I O T
CC = gcc
CFLAGS = -Wall

all:
	$(CC) $(CFLAGS) -o Init Init.c -lncurses
	$(CC) $(CFLAGS) -o B B.c -lncurses -lm
	$(CC) $(CFLAGS) -o D D.c -lncurses -lm
	$(CC) $(CFLAGS) -o I I.c -lncurses
	$(CC) $(CFLAGS) -o O O.c -lncurses
	$(CC) $(CFLAGS) -o T T.c -lncurses

clean:
	rm -f $(PROGRAMS)
