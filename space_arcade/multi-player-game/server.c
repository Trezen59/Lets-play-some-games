#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_PLAYERS 4
#define MAX_ASTEROIDS 5
#define MAX_BULLETS 5
#define GAME_WIDTH 30
#define GAME_HEIGHT 20

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
int client_sockets[MAX_PLAYERS] = {-1};
int num_clients = 0;
int frame_counter;
pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;

void update_bullets()
{
    if ( frame_counter % 2 == 0){
        pthread_mutex_lock(&game_mutex);
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            for (int j = 0; j < MAX_BULLETS; j++)
            {
                if (game_state.players[i].bullets[j].active)
                {
                    /* bullets move only in one direction - UP */
                    game_state.players[i].bullets[j].y--;

                    /* check if bullet is out of frame */
                    if (game_state.players[i].bullets[j].y < 1)
                    {
                        game_state.players[i].bullets[j].active = 0;
                        break;
                    }
                }
            }
        }
        pthread_mutex_unlock(&game_mutex);
    }
}

void update_asteroids()
{
    if (frame_counter % 3 == 0){
        pthread_mutex_lock(&game_mutex);

        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (game_state.asteroids[i].active) {
                game_state.asteroids[i].y++;
                if (game_state.asteroids[i].y >= GAME_HEIGHT - 1) {
                    game_state.asteroids[i].active = 0;
                }
            } else if (rand() % 20 == 0) {
                game_state.asteroids[i].x = rand() % (GAME_WIDTH - 2) + 1;
                game_state.asteroids[i].y = 1;
                game_state.asteroids[i].active = 1;
            }
        }
        pthread_mutex_unlock(&game_mutex);
    }
}

void check_collisions()
{
    pthread_mutex_lock(&game_mutex);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        for (int j = 0; j < MAX_BULLETS; j++) {
            if (game_state.players[i].bullets[j].active) {
                for (int k = 0; k < MAX_ASTEROIDS; k++) {
                    if (game_state.asteroids[k].active &&
                        game_state.players[i].bullets[j].x == game_state.asteroids[k].x &&
                        game_state.players[i].bullets[j].y == game_state.asteroids[k].y)
                    {
                        game_state.players[i].bullets[j].active = 0;
                        game_state.asteroids[k].active = 0;
                        game_state.players[i].score++;
                    }
                }
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
        update_bullets();
        update_asteroids();
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
        int ret = recv(client_socket, &command, sizeof(command), 0);
        if (ret < 0){
            printf("Failed to recv message, %d\n", ret);
            break;
        } else if (ret == 0){
            printf("Client %d disconnected.\n", player_id + 1);
            client_sockets[player_id] = -1;
			num_clients--;
            break;
        }

        pthread_mutex_lock(&game_mutex);

        if (command == 'L' && game_state.players[player_id].x > 1) {
            game_state.players[player_id].x--;
        } else if (command == 'R' && game_state.players[player_id].x < GAME_WIDTH - 2) {
            game_state.players[player_id].x++;
        } else if (command == 'F') {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!game_state.players[player_id].bullets[i].active) {
                    game_state.players[player_id].bullets[i].x = game_state.players[player_id].x;
                    game_state.players[player_id].bullets[i].y = GAME_HEIGHT - 2;
                    game_state.players[player_id].bullets[i].active = 1;
                    break;
                }
            }
        }
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
            send(cli_sock, "Server full. Try later.\n", 25, 0);
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
            game_state.players[num_clients].x = (GAME_WIDTH / 2) + num_clients + 1;
            game_state.players[num_clients].y = (GAME_HEIGHT - 2);
            game_state.players[num_clients].alive = 1;
            game_state.players[num_clients].id = num_clients + 1;
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
