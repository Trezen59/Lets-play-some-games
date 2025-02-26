#include "main.h"
using namespace std;

#define GAME_HEIGHT 10
#define GAME_WIDTH 30

#define WALL_BRICK "#"
#define TARGET "O"
#define TANK_HEAD '^'
#define BULLET '|'

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
		Position bullets[5];
		int score;
};

Tank tank_player_1;
Position enemy;
int game_over = 0;

/* draw enemy in the top half of game only */
void draw_enemy() {
	enemy.x = rand() % (GAME_WIDTH - 2) + 1;
	enemy.y = rand() % (GAME_HEIGHT/2 - 2) + 1;
	enemy.active = true;
}

void init_player()
{
	tank_player_1.tank_head = TANK_HEAD;
	tank_player_1.direction = RIGHT;

	/* start the player from the middle of x axis */
	tank_player_1.tank.x = GAME_WIDTH / 2;
	tank_player_1.tank.y = GAME_HEIGHT - 2;

	tank_player_1.key_left = KEY_LEFT;
	tank_player_1.key_right = KEY_RIGHT;

	tank_player_1.fire = (char)BULLET;
	tank_player_1.alive = true;

	/* init the bullets to be inactive by default */
	for (int i = 0; i < 5; i++){
		tank_player_1.bullets[i].active = 0;
	}

	tank_player_1.score = 0;
}

void check_collision()
{
	for (int i = 0; i < 5; i++)
	{
		if (tank_player_1.bullets[i].active)
		{
			/* check bullet and enemy collision */
			if (tank_player_1.bullets[i].x == enemy.x
					&& tank_player_1.bullets[i].y == enemy.y){
				draw_enemy();
				tank_player_1.bullets[i].active = 0;
				tank_player_1.score += 1;
				break;
			}
		}
	}
}

void update_bullets(){
	for (int i = 0; i < 5; i++)
	{
		if (tank_player_1.bullets[i].active)
		{
			/* bullets move only in one direction - UP */
			tank_player_1.bullets[i].y--;

			/* check if bullet is out of frame */
			if (tank_player_1.bullets[i].y < 1)
			{
				tank_player_1.bullets[i].active = 0;
				break;
			}
		}
	}
}

void fire_bullet(){
	for (int i = 0; i < 5; i++){
		if ( !tank_player_1.bullets[i].active ){
			tank_player_1.bullets[i].x = tank_player_1.tank.x;
			tank_player_1.bullets[i].y = tank_player_1.tank.y;
			tank_player_1.bullets[i].active = 1;
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
			/* print boundry walls */
			if (   y == 0 || (y == GAME_HEIGHT - 1)
				|| x == 0 || (x == GAME_WIDTH - 1)
				){
				printw(WALL_BRICK);
			}
			/* print tank */
			else if (tank_player_1.tank.x == x
					&& tank_player_1.tank.y == y ){
				printw("%c", tank_player_1.tank_head);
			}
			/* print bullets */
			else {
				int drawn = 0;
				for (int i = 0; i < 5; i++){
					if (tank_player_1.bullets[i].active
							&& tank_player_1.bullets[i].x == x
							&& tank_player_1.bullets[i].y == y){
						printw("%c", tank_player_1.fire);
						drawn = 1;
						break;
					}
				}
				/* print enemy */
				if (enemy.x == x && enemy.y == y){
					printw(TARGET);
					drawn = 1;
				}
				/* print blank space */
				if (!drawn){
					printw(" ");
				}
			}
		}
		printw("\n");
	}
	mvprintw(GAME_HEIGHT, 0, "Score: %d", tank_player_1.score);
	refresh();
}

void process_input()
{
    int ch = getch();

	/* move player only in left and right direction*/
    if ( ch == tank_player_1.key_left ) {
		tank_player_1.direction = LEFT;
		if (tank_player_1.tank.x > 1){
			tank_player_1.tank.x--;
		}
		else{
			tank_player_1.tank.x = 1;
			flushinp();
		}
	}
    if ( ch == tank_player_1.key_right ) {
		tank_player_1.direction = RIGHT;
		if (tank_player_1.tank.x < GAME_WIDTH - 2 ){
			tank_player_1.tank.x++;
		}
		else{
			tank_player_1.tank.x = GAME_WIDTH - 2;
			flushinp();
		}
	}
	if ( ch == 'q'){
		game_over = 1;
	}
	if ( ch == ' '){
			fire_bullet();
	}
}

void init_game_settings()
{
	initscr();
	noecho();
	curs_set(FALSE);
	timeout(100);
	keypad(stdscr, TRUE);
	srand(time(0));
}

void exit_game()
{
	clear();
	endwin();
	cout << "Game Over!\n";
	cout << "Score: Player1: "<< tank_player_1.score << endl;
}

int main()
{
	init_game_settings();

	init_player();
	draw_enemy();

	/* Game loop */
	while (!game_over){
		process_input();
		update_bullets();
		check_collision();
		draw_game();
		usleep(100000);
	}

	exit_game();
	return 0;
}
