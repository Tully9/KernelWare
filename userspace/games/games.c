#include "games.h"

game_shared_t game_shared = {0};
pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;


void game_pipedream_run(int fd);
void game_pipedream_draw(void);

void game_killit_run(int fd);
void game_killit_draw(void);

game_def_t games[] = {
    {
        .name = "PIPE DREAM - Drain the FIFO before it fills",
        .run  = game_pipedream_run,
        .draw = game_pipedream_draw,
    },
    {
        .name = "KILL IT - Type the PID to kill the process",
        .run  = game_killit_run,
        .draw = game_killit_draw,
    },
};
int num_games = sizeof(games) / sizeof(games[0]);
