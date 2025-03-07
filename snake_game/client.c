#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <errno.h>

#define GAME_WIDTH 30
#define GAME_HEIGHT 20
#define MAX_PLAYERS 4
#define MAX_FOOD 5
#define MAXLEN 100

#define WALL_BRICK "#"
#define BLANK_SPACE " "
#define FOOD		"*"
#define SNAKE_BODY	"o"
#define SNAKE_HEAD	"O"

#define PLAYER_LEFT 'a'
#define PLAYER_RIGHT 'd'
#define PLAYER_UP 'w'
#define PLAYER_DOWN 's'

/* Direction Enum */
typedef enum {
	UP = 0,
	DOWN,
	LEFT,
	RIGHT
}DIRECTION;

typedef struct {
	int x, y;
	int active;
} Food;

typedef struct {
	int x, y;
} Body;

typedef struct {
	int id;
	int length;
	int direction;
	int score;
	int alive;
	Body body[MAXLEN];
} Player;

typedef struct {
	Player snakes[MAX_PLAYERS];
	Food food[MAX_FOOD];
} GameState;

GameState game_state;
int sock;

void draw_game()
{
	clear();

	for (int i = 0; i < MAX_PLAYERS; i++){
		for (int j = 0; j < GAME_WIDTH; j++) {
			mvprintw(0, j, WALL_BRICK);
			mvprintw(GAME_HEIGHT - 1,j, WALL_BRICK);
		}
		for (int j = 0; j < GAME_HEIGHT; j++) {
			mvprintw(j, 0, WALL_BRICK);
			mvprintw(j, GAME_WIDTH - 1, WALL_BRICK);
		}
	}

	for (int i = 0; i < MAX_PLAYERS; i++){
		for (int j = 0; j <  game_state.snakes[i].length; j++){
			if (game_state.snakes[i].alive && game_state.food->active)
				mvprintw(game_state.food->y, game_state.food->x, FOOD);
		}
	}

	for (int i = 0; i < MAX_PLAYERS; i++){
		if (game_state.snakes[i].alive){
			for (int j = 0; j < game_state.snakes[i].length; j++){
				mvprintw(game_state.snakes[i].body[j].y, game_state.snakes[i].body[j].x,
						(j == 0) ? SNAKE_HEAD : SNAKE_BODY);
			}
		}
	}

	/* draw player scores */
	mvprintw(GAME_HEIGHT + 1, 0, "Player scores: ");
	for (int i = 0; i < 4; i++) {
		mvprintw(GAME_HEIGHT + i + 2, 0, "Player%d: %d", i + 1, game_state.snakes[i].score);
	}

	refresh();
}

void* handle_input(void* arg)
{
	int ret = -1;
	char command;

	while (1) {
		int ch = getch();
		if (ch == PLAYER_LEFT)
			command = 'L';
		else if (ch == PLAYER_RIGHT)
			command = 'R';
		else if (ch == PLAYER_UP)
			command = 'U';
		else if (ch == PLAYER_DOWN)
			command = 'D';
		else continue;

		ssize_t bytes = send(sock, &command, sizeof(command), 0);
		if (bytes < 0){
			printf("send failed: %s (errno: %d)\n", strerror(errno), errno);
			pthread_exit((void*)-1);
		}
	}
	return NULL;
}

void close_socket(int fd)
{
	if (fd >= 0)
		close(fd);
}

int setup_screen(WINDOW *window)
{
	int ret = -1;

	window = initscr();
	if (window == NULL){
		printf("initscr failed: %s (errno: %d)\n", strerror(errno), errno);
		return -EXIT_FAILURE;
	}

	noecho();

	ret = curs_set(0);
	if (ret < 0){
		printf("curs_set failed: %s (errno: %d)\n", strerror(errno), errno);
		return -EXIT_FAILURE;
	}
}

int main()
{
	int ret = -1;
	int game_over = 0;
	WINDOW *win;
	pthread_t input_thread;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		printf("socket failed: %s (errno: %d)\n", strerror(errno), errno);
		goto END;
	}

	struct sockaddr_in server_addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};

	ret = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0){
		printf("connect failed: %s (errno: %d)\n", strerror(errno), errno);
		goto END;
	}

	ret = setup_screen(win);
	if (ret < 0){
		printf("setup screen failed: %s (errno: %d)\n", strerror(errno), errno);
		goto END;
	}

	ret = pthread_create(&input_thread, NULL, handle_input, NULL);
	if (ret < 0){
		printf("Creating thread failed: %s (errno: %d)\n", strerror(errno), errno);
		goto END;
	}

	while(1)
	{
		ssize_t bytes_read = recv(sock, &game_state, sizeof(GameState), 0);
		if (bytes_read < 0){
			printf("Creating thread failed: %s (errno: %d)\n", strerror(errno), errno);
			break;
		}
		else if (bytes_read == 0){
			printf("Server disconnected.\n");
			break;
		}

		draw_game();

		/* game exit condition
		   Exit if none of the clients are playing */
		int game_live = 0;
		for (int i = 0; i < MAX_PLAYERS; i++){
			if (game_state.snakes[i].alive)
				game_live = 1;
		}
		if (!game_live)
			break;
	}

END:
	endwin();
	close_socket(sock);
	printf("Game Over.\n");
	return 0;
}
