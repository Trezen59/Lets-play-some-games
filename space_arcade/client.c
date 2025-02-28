#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define GAME_WIDTH 30
#define GAME_HEIGHT 20
#define MAX_PLAYERS 4
#define MAX_ASTEROIDS 5
#define MAX_BULLETS 5

#define PLAYER_1_LEFT 'a'
#define PLAYER_1_RIGHT 'd'
#define PLAYER_1_FIRE 'w'

#define PLAYER_2_LEFT 'j'
#define PLAYER_2_RIGHT 'l'
#define PLAYER_2_FIRE 'i'

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
    Bullet bullets[5];
} Player;

typedef struct {
    Player players[4];
    Asteroid asteroids[5];
} GameState;

GameState game_state;
int sock;

void draw_game() {
    clear();
    for (int y = 0; y < GAME_HEIGHT; y++) {
        for (int x = 0; x < GAME_WIDTH; x++) {
			/* print boundry walls */
			if (   y == 0 || (y == GAME_HEIGHT - 1)
					|| x == 0 || (x == GAME_WIDTH - 1)
			   ){
				mvprintw(y, x, "%c", '#');
			} else {
				int drawn = 0;
				for (int i = 0; i < MAX_PLAYERS; i++) {
					for (int j = 0; j < MAX_BULLETS; j++) {
						if (game_state.players[i].alive && game_state.players[i].x == x && game_state.players[i].y == GAME_HEIGHT - 2) {
							mvprintw(GAME_HEIGHT - 2, x, "^");
							drawn = 1;
						}
						if (game_state.players[i].bullets[j].active && game_state.players[i].bullets[j].x == x && game_state.players[i].bullets[j].y == y)
						{
							mvprintw(y, x, "|");
							drawn = 1;
						}
					}
				}
				for (int i = 0; i < MAX_ASTEROIDS; i++) {
					if (game_state.asteroids[i].active && game_state.asteroids[i].x == x && game_state.asteroids[i].y == y) {
						mvprintw(y, x, "O");
						drawn = 1;
					}
				}
				if (!drawn) mvprintw(y, x, " ");
			}
		}
    }
	mvprintw(GAME_HEIGHT + 1, 0, "Player scores: ");
	for (int i = 0; i < 4; i++) {
		mvprintw(GAME_HEIGHT + i + 2, 0, "Player%d: %d", i + 1, game_state.players[i].score);
	}
    refresh();
}

void* handle_input(void* arg) {
    char command;
    while (1) {
        int ch = getch();
        if      (ch == PLAYER_1_LEFT) command = 'L';
		else if (ch == PLAYER_1_RIGHT) command = 'R';
		else if (ch == PLAYER_1_FIRE) command = 'F';

		else if (ch == PLAYER_2_LEFT) command = 'L';
		else if (ch == PLAYER_2_RIGHT) command = 'R';
		else if (ch == PLAYER_2_FIRE) command = 'F';

		else continue;
		//printw("%c", command);

        send(sock, &command, sizeof(command), 0);
    }
    return NULL;
}

int main() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {AF_INET, htons(8080), inet_addr("127.0.0.1")};
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    initscr();
    noecho();
    curs_set(0);

    pthread_t input_thread;
    pthread_create(&input_thread, NULL, handle_input, NULL);

    while(1)
	{
		recv(sock, &game_state, sizeof(GameState), 0);
        draw_game();
    }

    endwin();
    return 0;
}

