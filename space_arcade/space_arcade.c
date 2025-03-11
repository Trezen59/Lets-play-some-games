#include "space_arcade.h"

Shooter shooter_player_1;
Position asteroid[5];
int game_over = 0;
int frame_counter;

void init_player()
{
	shooter_player_1.shooter_head = TANK_HEAD;
	shooter_player_1.direction = RIGHT;

	/* start the player from the middle of x axis */
	shooter_player_1.shooter.x = GAME_WIDTH / 2 - 2;
	shooter_player_1.shooter.y = GAME_HEIGHT - 2;

	shooter_player_1.key_left = LEFT_KEY;
	shooter_player_1.key_right = RIGHT_KEY;
	shooter_player_1.key_fire = FIRE_KEY;

	shooter_player_1.fire = (char)BULLET;
	shooter_player_1.alive = true;

	/* init the bullets to be inactive by default */
	for (int i = 0; i < 5; i++){
		shooter_player_1.bullets[i].active = 0;
		asteroid[i].active = 0;
	}

	shooter_player_1.score = 0;
}

void check_collision()
{
	for (int i = 0; i < 5; i++)
	{
		if (shooter_player_1.bullets[i].active)
		{
			for (int j = 0; j < 5; j++) {
				/* check bullet and enemy collision */
				if (asteroid[j].active
						&& shooter_player_1.bullets[i].x == asteroid[j].x
						&& shooter_player_1.bullets[i].y == asteroid[j].y
					)
				{
					shooter_player_1.bullets[i].active = 0;
					asteroid[j].active = 0;
					shooter_player_1.score += 1;
				}
			}
		}
	}
}

void update_bullets()
{
	if ( frame_counter % 2 == 0)
	{
		for (int i = 0; i < 5; i++)
		{
			if (shooter_player_1.bullets[i].active)
			{
				/* bullets move only in one direction - UP */
				shooter_player_1.bullets[i].y--;

				/* check if bullet is out of frame */
				if (shooter_player_1.bullets[i].y < 1)
				{
					shooter_player_1.bullets[i].active = 0;
					break;
				}
			}
		}
	}
}

void update_asteroids()
{
	if ( frame_counter % 3 == 0)
	{
		for (int i = 0; i < 5; i++)
		{
			if (asteroid[i].active){
				asteroid[i].y++;
				if (asteroid[i].y >= GAME_HEIGHT - 1){
					game_over = 1;
				}
			}
			else{
				if (rand() % 20 == 0){
					asteroid[i].x = rand() % (GAME_WIDTH - 2) + 1;
					asteroid[i].y = 2;
					asteroid[i].active = 1;
				}
			}
		}
	}
}

void fire_bullet()
{
	for (int i = 0; i < 5; i++){
		if ( !shooter_player_1.bullets[i].active ){
			shooter_player_1.bullets[i].x = shooter_player_1.shooter.x;
			shooter_player_1.bullets[i].y = shooter_player_1.shooter.y;
			shooter_player_1.bullets[i].active = 1;
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
			/* print shooter */
			else if (shooter_player_1.shooter.x == x
					&& shooter_player_1.shooter.y == y ){
				printw("%c", shooter_player_1.shooter_head);
			}
			/* print bullets */
			else {
				int drawn = 0;
				for (int i = 0; i < 5; i++){
					if (shooter_player_1.bullets[i].active
							&& shooter_player_1.bullets[i].x == x
							&& shooter_player_1.bullets[i].y == y){
						printw("%c", shooter_player_1.fire);
						drawn = 1;
						break;
					}
					if (asteroid[i].active
							&& asteroid[i].x == x
							&& asteroid[i].y == y){
						printw("%c", TARGET);
						drawn = 1;
						break;
					}
				}
				/* print blank space */
				if (!drawn) printw(" ");
			}
		}
		printw("\n");
	}
	printw("Score: %d", shooter_player_1.score);
	refresh();
}

void process_input()
{
    int ch = getch();

	/* move player only in left and right direction*/
    if ( ch == shooter_player_1.key_left ) {
		shooter_player_1.direction = LEFT;
		if (shooter_player_1.shooter.x > 1){
			shooter_player_1.shooter.x--;
		}
		else{
			shooter_player_1.shooter.x = 1;
			flushinp();
		}
	}
    if ( ch == shooter_player_1.key_right ) {
		shooter_player_1.direction = RIGHT;
		if (shooter_player_1.shooter.x < GAME_WIDTH - 2 ){
			shooter_player_1.shooter.x++;
		}
		else{
			shooter_player_1.shooter.x = GAME_WIDTH - 2;
			flushinp();
		}
	}
	if ( ch == QUIT_KEY){
		game_over = 1;
	}
	if ( ch == shooter_player_1.key_fire){
			fire_bullet();
	}
}

void init_game_settings()
{
	srand(time(NULL));
	initscr();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	timeout(INPUT_WAIT_TIMEOUT_MS);
}

void exit_game()
{
	clear();
	endwin();
	printf("Game Over!\n");
	printf("Score:\nPlayer: %d\n",shooter_player_1.score);
}

int main()
{
	init_game_settings();

	init_player();

	/* Game loop */
	while (!game_over){
		process_input();
		update_bullets();
		update_asteroids();
		check_collision();
		draw_game();
		usleep(GAME_UPDATE_SPEED_US);
		frame_counter++;
	}

	exit_game();
	return 0;
}
