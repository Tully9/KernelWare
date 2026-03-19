#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include "../../shared/kw_ioctl.h"
#include "games.h"

int game_hackhost_run(int fd)
{
    char correct[64] = {0};
    ioctl(fd, KW_IOCTL_GET_HOSTNAME, correct);

    pthread_mutex_lock(&game_mutex);
    snprintf(game_shared.message, 300, "RESTORE THE HOST!");
    snprintf(game_shared.subtext,  128, "The hostname is: PWND-%s-is_a_LOSER", correct);
    game_shared.typed[0] = '\0';
    game_shared.typed_len = 0;
    pthread_mutex_unlock(&game_mutex);

    int won = 0;
    unsigned char event;
    while (1) {
        ssize_t n = read(fd, &event, 1);
        if (n <= 0) break;

        if (event == KW_EVENT_TIMEOUT) {
            GAME_SET_MSG("TIME'S UP!", "The hostname stays hacked...");
            sleep(2);
            break;
        }

        if (event == KW_EVENT_CORRECT) {
            ioctl(fd, KW_IOCTL_SET_HOSTNAME, correct);
            GAME_SET_MSG("HOSTNAME RESTORED!", "Check /proc/kernelware/stats");
            sleep(2);
            won = 1;
            break;
        }
    }

    ioctl(fd, KW_IOCTL_STOP);
    sleep(2);
    return won;
}

void game_hackhost_draw(void)
{
    pthread_mutex_lock(&game_mutex);
    char msg[128], sub[128];
    strncpy(msg, game_shared.message, 128);
    strncpy(sub, game_shared.subtext,  128);
    pthread_mutex_unlock(&game_mutex);

    mvprintw(7,  10, "=== HACK THE HOST ===");
    mvprintw(9,  10, "%-40s", msg);
    mvprintw(10, 10, "%-40s", sub);
    mvprintw(12, 10, "Type hostname and press Enter");
    mvprintw(14, 10, "Your input: %s_", game_shared.typed);
}