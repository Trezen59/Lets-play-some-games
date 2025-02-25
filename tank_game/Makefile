CC= g++
TARGET= main
CLIBS= -lncurses -lpthread
FLAGS= -g

all:
	$(CC) $(FLAGS) main.cpp -o $(TARGET) $(CLIBS)
	ctags -R *

clean:
	rm -rf $(TARGET) tags
