CC=gcc
LIBS=-lncurses -lpthread
CFLAGS=-Wall -g
SNAKE_GAME_SOURCE=snake_game.c
SNAKE_BIN_NAME=snake

all:
	$(CC) $(CFLAGS) $(SNAKE_GAME_SOURCE) -o $(SNAKE_BIN_NAME) $(LIBS)
	ctags -R &

tags:
	ctags -R &

clean:
	rm -rf $(BIN_NAME) tags
