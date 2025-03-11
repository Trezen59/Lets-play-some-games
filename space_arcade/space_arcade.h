#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>

#define GAME_HEIGHT 20
#define GAME_WIDTH  30

#define INPUT_WAIT_TIMEOUT_MS 100
#define GAME_UPDATE_SPEED_US 100000

#define WALL_BRICK "#"
#define TARGET     '*'
#define TANK_HEAD  '^'
#define BULLET     '|'

#define FIRE_KEY   'w'
#define LEFT_KEY   'a'
#define RIGHT_KEY  'd'
#define QUIT_KEY   'q'

typedef enum {
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
} Direction;

typedef struct{
	int x, y;
	int active;
} Position;

typedef struct {
		char shooter_head;
		int direction;
		Position shooter;
		int key_left, key_right, key_fire;
		char fire;
		bool alive;
		Position bullets[5];
		int score;
} Shooter;

