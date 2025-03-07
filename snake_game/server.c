#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define GAME_WIDTH 30
#define GAME_HEIGHT 20
#define MAX_PLAYERS 4
#define MAX_FOOD 5
#define MAXLEN 100

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
int client_sockets[MAX_PLAYERS] = {-1};
int num_clients = 0;
int frame_counter = 0;

pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;

void place_food()
{
	game_state.food->x = rand() % (GAME_WIDTH - 2) + 1;
	game_state.food->y = rand() % (GAME_HEIGHT - 2) + 1;
	game_state.food->active = 1;
}

void move_snake()
{
	pthread_mutex_lock(&game_mutex);

	/* shift body forward */
	for (int i = 0; i < num_clients; i++)
	{
		if (game_state.snakes[i].alive){
			for (int j = game_state.snakes[j].length - 1; j > 0; j--){
				game_state.snakes[i].body[j] = game_state.snakes[i].body[j - 1];
			}
		}
	}

	/* move head */
	for (int i = 0; i < num_clients; i++)
	{
		if (game_state.snakes[i].alive)
		{
			if (game_state.snakes[i].direction == UP ){
				game_state.snakes[i].body[0].y--;
			}
			else if (game_state.snakes[i].direction == DOWN ){
				game_state.snakes[i].body[0].y++;
			}
			else if (game_state.snakes[i].direction == LEFT ){
				game_state.snakes[i].body[0].x--;
			}
			else if (game_state.snakes[i].direction == RIGHT ){
				game_state.snakes[i].body[0].x++;
			}
		}
	}

	/* check if food is eaten */
	for (int i = 0; i < num_clients; i++)
	{
		if (game_state.snakes[i].alive && game_state.snakes[i].body[0].x == game_state.food->x
				&& game_state.snakes[i].body[0].y == game_state.food->y)
		{
			game_state.snakes[i].length++;
			game_state.snakes[i].score++;
			place_food();
		}
	}

	pthread_mutex_unlock(&game_mutex);
}

void check_collisions()
{
    pthread_mutex_lock(&game_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++)
	{
		/* Collision with wall has happened */
		if (game_state.snakes[i].alive
				&& game_state.snakes[i].body[0].x < 1
				|| game_state.snakes[i].body[0].x > GAME_WIDTH - 2
				|| game_state.snakes[i].body[0].y < 1
				|| game_state.snakes[i].body[0].y > GAME_HEIGHT - 2)
		{
			game_state.snakes[i].alive = 0;
		}

		/* Collision with itself */
		for (int j = 1; j < game_state.snakes[i].length; j++){
			if (game_state.snakes[i].body[0].x == game_state.snakes[i].body[j].x
					&& game_state.snakes[i].body[0].y == game_state.snakes[i].body[j].y)
			{
				game_state.snakes[i].alive = 0;
			}
		}
	}

    pthread_mutex_unlock(&game_mutex);
}

void send_game_state_to_all()
{
    pthread_mutex_lock(&game_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        send(client_sockets[i], &game_state, sizeof(GameState), 0);
    }

    pthread_mutex_unlock(&game_mutex);
}

void *update_game(void *arg)
{
    while (1) {
		move_snake();
        check_collisions();
        send_game_state_to_all();
        frame_counter++;
        usleep(100000);
    }
    return NULL;
}

void *handle_client(void *socket)
{
    int client_socket = *(int*)socket;
    int player_id = num_clients - 1;

    char command;
    while (1)
    {
		/* receive the direction snake as keypress */
        ssize_t bytes = recv(client_socket, &command, sizeof(command), 0);
        if (bytes < 0){
            printf("Failed to recv message, %ld\n", bytes);
            break;
        } else if (bytes == 0){
            printf("Client %d disconnected.\n", player_id + 1);
            client_sockets[player_id] = -1;
			num_clients--;
            break;
        }

        pthread_mutex_lock(&game_mutex);

		/* Update snake direction */
        if (command == 'L' && game_state.snakes[player_id].direction != RIGHT) {
            game_state.snakes[player_id].direction = LEFT;
        } else if (command == 'R' && game_state.snakes[player_id].direction != LEFT) {
            game_state.snakes[player_id].direction = RIGHT;
        } else if (command == 'U' && game_state.snakes[player_id].direction != DOWN) {
            game_state.snakes[player_id].direction = UP;
        } else if (command == 'D' && game_state.snakes[player_id].direction != UP) {
            game_state.snakes[player_id].direction = DOWN;
        }
		//printf("%c\n", command);

        pthread_mutex_unlock(&game_mutex);
    }

    close(client_socket);
    return NULL;
}

void close_socket(int socket)
{
    if (socket >= 0){
        close(socket);
    }
}

int main()
{
    int ret = -1;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
        printf("Failed to create the socket, %d\n", server_socket);
        return server_socket;
    } else {
        printf("Server Socket created: %d\n", server_socket);
    }

    struct sockaddr_in server_addr = {AF_INET, htons(8080), INADDR_ANY};

    ret = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret < 0){
        printf("Failed to Bind socket, %d\n", ret);
        goto END;
    } else {
        printf("Server socket binded.\n");
    }

    ret = listen(server_socket, MAX_PLAYERS);
    if (ret < 0){
        printf("Failed to listen client connection.\n");
        goto END;
    } else {
        printf("Listening for client connections...\n");
    }

    pthread_t game_thread;

    ret = pthread_create(&game_thread, NULL, update_game, NULL);
    if (ret < 0){
        printf("Failed to create game thread, %d\n", ret);
        goto END;
    } else {
        printf("Game thread created.\n");
    }

    while (1)
    {
        int cli_sock = accept(server_socket, NULL, NULL);
        if (cli_sock < 0){
            printf("Failed to accept the client. %d\n", cli_sock);
            continue;
        } else {
            printf("Client %d connection accepted, FD: %d\n", num_clients + 1, cli_sock);
        }

        if (num_clients >= MAX_PLAYERS)
        {
            printf("MAX PLAYERS\n");
            close(cli_sock);
            continue;
        }

        if (num_clients < MAX_PLAYERS)
		{
            client_sockets[num_clients] = cli_sock;

            pthread_t thread;
            ret = pthread_create(&thread, NULL, handle_client, &client_sockets[num_clients]);
            if (ret < 0){
                printf("Failed to create thread to handle client. %d\n", ret);
                continue;
            } else {
                printf("Thread created to handle the client.\n");
            }

            pthread_mutex_lock(&game_mutex);

            game_state.snakes[num_clients].alive = 1;
            game_state.snakes[num_clients].length = 3;
            game_state.snakes[num_clients].direction = LEFT;
            game_state.snakes[num_clients].score = 0;
            game_state.snakes[num_clients].id = num_clients + 1;

			place_food();
			
			for (int i = 0; i < game_state.snakes[num_clients].length; i++){
				game_state.snakes[num_clients].body[i].x = GAME_WIDTH / 2 - 1;
				game_state.snakes[num_clients].body[i].y = GAME_HEIGHT / 2;
			}

            num_clients++;

            pthread_mutex_unlock(&game_mutex);
        }
    }

    ret = pthread_join(game_thread, NULL);
    if (ret < 0){
        printf("Failed to join game thread.\n");
        goto END;
    } else {
        printf("Joined game thread.\n");
    }

END:
    close_socket(server_socket);
    return 0;
}
