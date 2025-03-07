CC= gcc
TARGET= snake_game
SERVER_TARGET= server
CLIENT_TARGET= client
ALL_TARGET= $(TARGET) $(SERVER_TARGET) $(CLIENT_TARGET) tags
CLIBS= -lncurses -lpthread
FLAGS= -g

all: clean game server client
	ctags -R *

game:
	$(CC) $(FLAGS) $(TARGET).c -o $(TARGET) $(CLIBS)

server:
	$(CC) $(FLAGS) $(SERVER_TARGET).c -o $(SERVER_TARGET) $(CLIBS)

client:
	$(CC) $(FLAGS) $(CLIENT_TARGET).c -o $(CLIENT_TARGET) $(CLIBS)

clean:
	rm -rf $(ALL_TARGET)
