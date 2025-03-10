#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

char *VERSION = "0.0.1";

#define WALL_BRICK	"#"
#define FOOD		"*"
#define SNAKE_BODY	"o"
#define SNAKE_HEAD	"O"

#define HEAD		0
#define MAXLEN		100
#define WIDTH		30
#define HEIGHT		20

/* Direction Enum */
typedef enum {
	UP = 0,
	DOWN,
	LEFT,
	RIGHT
}DIRECTION;

/* Position co-ordinates */
typedef struct {
	int x, y;
} Position;

/* Define Snake struct */
typedef struct {
	/* Max length */
	Position body[100];
	int length;
	int direction;
} Snake;

/* global declarations */
Position food;

int start_x, start_y;

/* Monitor game over */
int game_over = 0;

/* Initialize the game */
int init_game()
{
	int	ret = -1;

	/* Starts ncurses mode */
	if (initscr() == NULL){
		return -EXIT_FAILURE;
	}

	/* Prevents keypresses from displaying on-screen */
	ret = noecho();
	if (ret < 0){
		printf("noecho() Failed with %d\n", ret);
		return ret;
	}

	/* Hides the cursor */
	ret = curs_set(FALSE);
	if (ret < 0){
		printf("curs_set() Failed with %d\n", ret);
		return ret;
	}

	/* Sets input wait time
	   Adjust speed of snake
	*/
	timeout(100);

	/* sequence of psuedo random integers */
	srand(time(0));

	return 0;
}

void init_snake(Snake *snake, int length, int dir)
{
	/* Initialize snake */
	snake->length = length;
	snake->direction = dir;

	int center_x = start_x + WIDTH / 2;
	int center_y = start_y + HEIGHT / 2;

	for (int i = 0; i < snake->length; i++){
		snake->body[i].x = center_x - 1;
		snake->body[i].y = center_y;
	}
}

void place_food(Snake *snake)
{
	bool valid_food_position = false;

	while (!valid_food_position) {
		food.x = start_x + rand() % (WIDTH - 2) + 1;
		food.y = start_y + rand() % (HEIGHT - 2) + 1;

		valid_food_position = true;
		for (int i = 0; i < snake->length; i++) {
			if (snake->body[i].x == food.x && snake->body[i].y == food.y) {
				valid_food_position = false;
				break;
			}
		}
	}
}

void draw_walls()
{
	for (int i = 0; i < WIDTH; i++) {
		mvprintw(start_y, start_x + i, WALL_BRICK);
		mvprintw(start_y + HEIGHT - 1, start_x + i, WALL_BRICK);
	}
	for (int i = 0; i < HEIGHT; i++) {
		mvprintw(start_y + i, start_x, WALL_BRICK);
		mvprintw(start_y + i, start_x + WIDTH - 1, WALL_BRICK);
	}
}

void draw_snake(Snake *snake)
{
	clear();
	draw_walls();

	mvprintw(food.y, food.x, FOOD);

	for (int i = 0; i < snake->length; i++) {
		mvprintw(snake->body[i].y, snake->body[i].x,
				(i == HEAD) ? SNAKE_HEAD : SNAKE_BODY);
	}
	refresh();
}

int check_collision(Snake *snake)
{
	/* Collision check (wall) */
	if (snake->body[HEAD].x <= start_x || snake->body[HEAD].x >= start_x + WIDTH - 1 ||
			snake->body[HEAD].y <= start_y || snake->body[HEAD].y >= start_y + HEIGHT - 1) {
		game_over = 1;
	}

	/* Collision check (self) */
	for (int i = 1; i < snake->length; i++) {
		if (snake->body[HEAD].x == snake->body[i].x &&
				snake->body[HEAD].y == snake->body[i].y) {
			game_over = 1;
		}
	}
	return 0;
}

void move_snake(Snake *snake)
{
	/* Shift body forward */
	/* Update each segment from tail to head */
	for (int i = snake->length - 1; i > 0; i--) {
		snake->body[i] = snake->body[i - 1];
	}

	/* Move head */
	if      (snake->direction == UP)	snake->body[0].y--;
	else if (snake->direction == DOWN)	snake->body[0].y++;
	else if (snake->direction == LEFT)	snake->body[0].x--;
	else if (snake->direction == RIGHT) snake->body[0].x++;

	/* Check if food is eaten */
	if (snake->body[0].x == food.x && snake->body[0].y == food.y) {
		snake->length++;
		/* Make more food */
		place_food(snake);
	}
}

int process_input(Snake *snake)
{
	int ch = getch();

	/* do not move in opposite direction */
	if      (ch == 'w' && snake->direction != DOWN)		snake->direction = UP;
	else if (ch == 's' && snake->direction != UP)		snake->direction = DOWN;
	else if (ch == 'a' && snake->direction != RIGHT)	snake->direction = LEFT;
	else if (ch == 'd' && snake->direction != LEFT)		snake->direction = RIGHT;
	return 0;
}

int main()
{
	int ret = -1;
	int term_width = -1, term_height = -1;
	int snake_length = 3;

	ret = init_game();
	if (ret < 0){
		printf("Failed to Init snake game, %d\n", ret);
		return -EXIT_FAILURE;
	}

	getmaxyx(stdscr, term_height, term_width);
	start_x = (term_width - WIDTH) / 2;
	start_y = (term_height - HEIGHT) / 2;

	Snake snake;

	init_snake(&snake, snake_length, RIGHT);
	place_food(&snake);

	/* Game loop */
	while (!game_over) {
		process_input(&snake);
		move_snake(&snake);
		check_collision(&snake);
		draw_snake(&snake);
		/* Sleep to control game update speed */
		usleep(100000);
	}

	clear();
	printw("Game Over!\n");
	printw("Press any key to exit...");
	refresh();

	/* update timeout to wait for input */
	timeout(1000);
	getch();

	endwin();
	return 0;
}
