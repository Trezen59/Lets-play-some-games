CC=gcc
LIBS=-lncurses
BIN_NAME=snake_game
CFLAGS=-Wall -g

all:
	$(CC) $(CFLAGS) snake_game.c -o $(BIN_NAME) $(LIBS)

tags:
	ctags -R &

clean:
	rm -rf $(BIN_NAME)
