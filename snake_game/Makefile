CC= gcc
TARGET= snake_game
MULTI_PLAYER_DIR=multi-player-game/
SERVER_TARGET= server
CLIENT_TARGET= client
BIN_FOLDER=bin/
ALL_TARGET= $(BIN_FOLDER)$(TARGET) \
			$(BIN_FOLDER)$(SERVER_TARGET) \
			$(BIN_FOLDER)$(CLIENT_TARGET) \
			tags
CLIBS= -lncurses -lpthread
FLAGS= -g

all: clean game server client
	ctags -R *

game:
	$(CC) $(FLAGS) $(TARGET).c -o $(BIN_FOLDER)$(TARGET) $(CLIBS)

server:
	$(CC) $(FLAGS) $(MULTI_PLAYER_DIR)$(SERVER_TARGET).c -o $(BIN_FOLDER)$(SERVER_TARGET) $(CLIBS)

client:
	$(CC) $(FLAGS) $(MULTI_PLAYER_DIR)$(CLIENT_TARGET).c -o $(BIN_FOLDER)$(CLIENT_TARGET) $(CLIBS)

clean:
	rm -rf $(ALL_TARGET)
