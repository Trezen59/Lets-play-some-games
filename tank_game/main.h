#include <iostream>
#include <cstdlib>
#include <string>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>

#define INPUT_WAIT_TIMEOUT_MS 100
#define GAME_UPDATE_SPEED_US 100000

#define GAME_HEIGHT 10
#define GAME_WIDTH  30

#define WALL_BRICK "#"
#define TARGET     "O"
#define TANK_HEAD  '^'
#define BULLET     '|'

#define MAX_BULLETS 5

#define LEFT_KEY  'a'
#define RIGHT_KEY 'd'
#define FIRE_KEY  'w'
#define QUIT_KEY  'q'

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

class Tank {
	public:
		char tank_head;
		int direction;
		Position tank;
		int key_left, key_right, key_fire;
		char fire;
		bool alive;
		Position bullets[MAX_BULLETS];
		int score;
};
