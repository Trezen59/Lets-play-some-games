#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <errno.h>

#define GAME_WIDTH     30
#define GAME_HEIGHT    20
#define MAX_PLAYERS    4
#define MAX_ASTEROIDS  5
#define MAX_BULLETS    5

#define WALL_BRICK    "#"
#define BULLET        "|"
#define ASTEROID      "O"
#define BLANK_SPACE   " "
#define SHOOTER       "^"

#define PLAYER_LEFT   'a'
#define PLAYER_RIGHT  'd'
#define PLAYER_FIRE   'w'

#define COMMAND_LEFT  'L'
#define COMMAND_RIGHT 'R'
#define COMMAND_FIRE  'F'

typedef struct {
	int x, y;
	int active;
	int owner;
} Bullet;

typedef struct {
	int x, y;
	int active;
} Asteroid;

typedef struct {
	int id;
	int x, y;
	int score;
	int alive;
	Bullet bullets[MAX_BULLETS];
} Player;

typedef struct {
	Player players[MAX_PLAYERS];
	Asteroid asteroids[MAX_ASTEROIDS];
} GameState;

GameState game_state;
int sock;

void draw_game()
{
	clear();
	for (int y = 0; y < GAME_HEIGHT; y++) {
		for (int x = 0; x < GAME_WIDTH; x++) {
			/* print boundry walls */
			if (   y == 0 || (y == GAME_HEIGHT - 1)
					|| x == 0 || (x == GAME_WIDTH - 1)
			   ){
				mvprintw(y, x, WALL_BRICK);
			} else {
				int drawn = 0;
				/* draw game player and bullets */
				for (int i = 0; i < MAX_PLAYERS; i++) {
					for (int j = 0; j < MAX_BULLETS; j++) {
						if (game_state.players[i].alive && game_state.players[i].x == x && game_state.players[i].y == GAME_HEIGHT - 2) {
							mvprintw(GAME_HEIGHT - 2, x, SHOOTER);
							drawn = 1;
						}
						/* draw bullets */
						if (game_state.players[i].bullets[j].active && game_state.players[i].bullets[j].x == x && game_state.players[i].bullets[j].y == y)
						{
							mvprintw(y, x, BULLET);
							drawn = 1;
						}
					}
				}
				/* draw asteroids */
				for (int i = 0; i < MAX_ASTEROIDS; i++) {
					if (game_state.asteroids[i].active && game_state.asteroids[i].x == x && game_state.asteroids[i].y == y) {
						mvprintw(y, x, ASTEROID);
						drawn = 1;
					}
				}
				/* draw blank space */
				if (!drawn) mvprintw(y, x, BLANK_SPACE);
			}
		}
	}
	/* draw player scores */
	mvprintw(GAME_HEIGHT + 1, 0, "Player scores: ");
	for (int i = 0; i < 4; i++) {
		mvprintw(GAME_HEIGHT + i + 2, 0, "Player%d: %d", i + 1, game_state.players[i].score);
	}
	refresh();
}

void* handle_input(void* arg)
{
	int ret = -1;
	char command;

	while (1) {
		int ch = getch();
		if      (ch == PLAYER_LEFT)  command = COMMAND_LEFT;
		else if (ch == PLAYER_RIGHT) command = COMMAND_RIGHT;
		else if (ch == PLAYER_FIRE)  command = COMMAND_FIRE;
		else continue;

		ret = send(sock, &command, sizeof(command), 0);
		if (ret < 0){
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
	WINDOW *win;

	sock = socket(AF_INET, SOCK_STREAM, 0);
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

	pthread_t input_thread;
	ret = pthread_create(&input_thread, NULL, handle_input, NULL);
	if (ret < 0){
		printf("Creating thread failed: %s (errno: %d)\n", strerror(errno), errno);
		goto END;
	}

	while(1)
	{
		ssize_t bytes_read = recv(sock, &game_state, sizeof(GameState), 0);
		if (bytes_read < 0)
		{
			printf("Creating thread failed: %s (errno: %d)\n", strerror(errno), errno);
			break;
		}
		else if (bytes_read == 0)
		{
			printf("Server disconnected.\n");
			break;
		}

		draw_game();
	}

END:
	close_socket(sock);
	endwin();
	return 0;
}
