#include "games.h"

game_shared_t game_shared = {0};
pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;


void game_pipedream_run(int fd);
void game_pipedream_draw(void);


static game_def_t pipedream = {
    .name = "PIPE DREAM - Drain the FIFO before it fills",
    .run  = game_pipedream_run,
    .draw = game_pipedream_draw,
};


game_def_t games[] = {
    pipedream,
};


int num_games = sizeof(games) / sizeof(games[0]);
