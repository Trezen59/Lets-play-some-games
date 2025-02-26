CC= gcc
TARGET= space_arcade
CLIBS= -lncurses -lpthread
FLAGS= -g

all:
	$(CC) $(FLAGS) space_arcade.c -o $(TARGET) $(CLIBS)
	ctags -R *

clean:
	rm -rf $(TARGET) tags
