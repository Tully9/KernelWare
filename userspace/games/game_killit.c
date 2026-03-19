#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include "../../shared/kw_ioctl.h"
#include "games.h"

void game_killit_run(int fd)
{
    ioctl(fd, KW_IOCTL_START, 3);

    pthread_mutex_lock(&game_mutex);
    game_shared.score = 0;
    snprintf(game_shared.message, 128, "Type the PID to kill it!");
    snprintf(game_shared.subtext,  128, "");
    pthread_mutex_unlock(&game_mutex);

    struct kw_state state;
    char input[32];
    unsigned char result;

    while (1) {
        memset(&state, 0, sizeof(state));
        ioctl(fd, KW_IOCTL_GET_STATE, &state);

        pthread_mutex_lock(&game_mutex);
        snprintf(game_shared.message, 128, "Kill PID: %s", state.prompt);
        pthread_mutex_unlock(&game_mutex);

        memset(input, 0, sizeof(input));
        echo();
        mvgetnstr(15, 10, input, sizeof(input) - 1);
        noecho();

        write(fd, input, strlen(input));
        read(fd, &result, 1);

        pthread_mutex_lock(&game_mutex);
        game_shared.score = state.score + (result == 0x01 ? 1 : 0);
        snprintf(game_shared.subtext, 128, result == 0x01 ? "Correct!" : "Wrong!");
        pthread_mutex_unlock(&game_mutex);
    }

    ioctl(fd, KW_IOCTL_STOP);
}

void game_killit_draw(void)
{
    pthread_mutex_lock(&game_mutex);
    int  score = game_shared.score;
    char msg[128], sub[128];
    strncpy(msg, game_shared.message, 128);
    strncpy(sub, game_shared.subtext,  128);
    pthread_mutex_unlock(&game_mutex);

    mvprintw(4,  10, "=== KILL IT ===");
    mvprintw(6,  10, "Score: %d", score);
    mvprintw(8,  10, "%-40s", msg);
    mvprintw(10, 10, "%-40s", sub);
    mvprintw(13, 10, "Type the PID shown above and press Enter");
}
