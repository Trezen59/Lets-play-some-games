#include "main.h"
using namespace std;

#define GAME_HEIGHT 10
#define GAME_WIDTH 30

#define TANK_LEN 2
#define MAX_LEN 100
#define MAX_PLAYERS 4

#define WALL_BRICK "#"
#define TARGET "O"
#define TANK_HEAD '^'
#define BULLET "|"

typedef enum {
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
	FIRE,
} Direction;

typedef struct{
	int x, y;
	int active;
	int direction;
} Position;

class Tank {
	public:
		char tank_head;
		int direction;
		Position tank;
		int key_up, key_down, key_left, key_right, key_fire;
		char fire;
		bool alive;
};

Tank tank_player_1;
Position enemy;
Position bullets[5];
int game_over = 0;
int frameCounter = 0;
int score = 0;

void draw_enemy() {
	enemy.x = rand() % (GAME_WIDTH - 2) + 1;
	enemy.y = rand() % (GAME_HEIGHT - 2) + 1;
}

void init_game()
{
	tank_player_1.tank_head = TANK_HEAD;
	tank_player_1.direction = RIGHT;

	tank_player_1.tank.x = 3;
	tank_player_1.tank.y = 2;

	tank_player_1.key_up = UP;
	tank_player_1.key_down = DOWN;
	tank_player_1.key_left = LEFT;
	tank_player_1.key_right = RIGHT;
	tank_player_1.key_fire = FIRE;

	tank_player_1.fire = '-';
	tank_player_1.alive = true;

	for (int i = 0; i < 5; i++){
		bullets[i].active = 0;
		bullets[i].direction = RIGHT;
	}

	draw_enemy();
}

void update_bullets(){
	for (int i = 0; i < 5; i++)
	{
		if (bullets[i].active)
		{
			if (bullets[i].direction == UP)
			{
				bullets[i].y--;
				if (bullets[i].y < 1 || bullets[i].x == enemy.x && bullets[i].y == enemy.y)
				{
					if (bullets[i].x == enemy.x && bullets[i].y == enemy.y){
						draw_enemy();
					}
					bullets[i].active = 0;
					break;
				}
			}
			if (bullets[i].direction == DOWN)
			{
				bullets[i].y++;
				if (bullets[i].y > GAME_HEIGHT - 2 || bullets[i].x == enemy.x && bullets[i].y == enemy.y)
				{
					if (bullets[i].x == enemy.x && bullets[i].y == enemy.y){
						draw_enemy();
					}
					bullets[i].active = 0;
					break;
				}
			}
			if (bullets[i].direction == LEFT)
			{
				bullets[i].x--;
				if (bullets[i].x < 1 || bullets[i].x == enemy.x && bullets[i].y == enemy.y)
				{
					if (bullets[i].x == enemy.x && bullets[i].y == enemy.y){
						draw_enemy();
					}
					bullets[i].active = 0;
					break;
				}
			}
			if (bullets[i].direction == RIGHT)
			{
				bullets[i].x++;
				if (bullets[i].x > GAME_WIDTH - 2 || bullets[i].x == enemy.x && bullets[i].y == enemy.y)
				{
					if (bullets[i].x == enemy.x && bullets[i].y == enemy.y){
						draw_enemy();
					}
					bullets[i].active = 0;
					break;
				}
			}
		}
	}	
}

void fire_bullet(){
	for (int i = 0; i < 5; i++)
	{
		if ( !bullets[i].active ){
			bullets[i].x = tank_player_1.tank.x;
			bullets[i].y = tank_player_1.tank.y;
			bullets[i].direction = tank_player_1.tank.direction;
			bullets[i].active = 1;
			break;
		}
	}
}

void draw_game()
{
	clear();
	for (int y = 0; y < GAME_HEIGHT; y++)
	{
		for (int x = 0; x < GAME_WIDTH; x++)
		{
			if (y == 0 || (y == GAME_HEIGHT - 1)
				|| x == 0 || (x == GAME_WIDTH - 1)
				//|| (y == 6 && x < 20)
				//|| (y == 3 && x > 10)
				)
			{
				printw(WALL_BRICK);
				//printw("%d,%d%s ", i, j, WALL_BRICK);
			}
			else if (tank_player_1.tank.x == x && tank_player_1.tank.y == y ){
				printw("%c", TANK_HEAD);
			} else {
				int drawn = 0;
				for (int i = 0; i < 5; i++){
					if (bullets[i].active && bullets[i].x == x && bullets[i].y == y){
						printw(BULLET);
						drawn = 1;
						break;
					}
				} 
				if (enemy.x == x && enemy.y == y){
					printw(TARGET);
					drawn = 1;
				} 
				if (!drawn){
					printw(" ");
				}
			}
		}
		printw("\n");
	}
	refresh();
}

void process_input()
{
    int ch = getch();
    if( ch == KEY_UP ) {
		tank_player_1.direction = UP;
		tank_player_1.tank.y--;
	} 
    if ( ch == KEY_DOWN ) {
		tank_player_1.direction = DOWN;
		tank_player_1.tank.y++;
	}   
    if ( ch == KEY_LEFT ) {
		tank_player_1.direction = LEFT;
		tank_player_1.tank.x--;
	}
    if ( ch == KEY_RIGHT ) {
		tank_player_1.direction = RIGHT;
		tank_player_1.tank.x++;
	}
	if ( ch == ' '){
			fire_bullet();
	}
}

int main()
{
	initscr();
	noecho();
	curs_set(FALSE);
	timeout(100);
	keypad(stdscr, TRUE);
	srand(time(0));

	init_game();

	while (!game_over){
		process_input();
		update_bullets();
		draw_game();
		usleep(100000);
	}

	clear();
	endwin();
	cout << "Game Over!";
	getch();

	return 0;
}