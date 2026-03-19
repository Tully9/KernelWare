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
    // fetch the original hostname from kernel
    char correct[64] = {0};
    ioctl(fd, KW_IOCTL_GET_HOSTNAME, correct);

    pthread_mutex_lock(&game_mutex);
    snprintf(game_shared.message, 128, "RESTORE THE HOST!");
    snprintf(game_shared.subtext,  128, "Type the original hostname to fix it");
    pthread_mutex_unlock(&game_mutex);

    char input[64] = {0};
    int len = 0;

    clear();
    mvprintw(1,  10, "=== KernelWare ===");
    mvprintw(7,  10, "=== HACK THE HOST ===");
    mvprintw(9,  10, "The hostname has been changed to: PWND-%s", correct);
    mvprintw(11, 10, "Type the ORIGINAL hostname to restore it!");
    refresh();

    curs_set(1);
    timeout(100);

    while (len < (int)sizeof(input) - 1) {
        struct kw_state state = {0};
        ioctl(fd, KW_IOCTL_GET_STATE, &state);
        if (state.score >= 100) {
            timeout(-1);
            curs_set(0);
            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 128, "TIME'S UP!");
            snprintf(game_shared.subtext,  128, "The hostname stays hacked...");
            pthread_mutex_unlock(&game_mutex);
            sleep(2);
            return 0;
        }

        pthread_mutex_lock(&game_mutex);
        int cur_score = game_shared.score;
        int cur_lives = game_shared.lives;
        pthread_mutex_unlock(&game_mutex);
        mvprintw(3, 10, "Score: %-5d  Lives: %d", cur_score, cur_lives);

        int tw_rem = 100 - state.score;
        if (tw_rem < 0) tw_rem = 0;
        const int TW = 30;
        int tf = (tw_rem * TW) / 100;
        if (tw_rem < 20) attron(A_BOLD);
        mvprintw(5, 10, "TIME [");
        for (int i = 0; i < TW; i++)
            mvaddch(5, 16 + i, i < tf ? '#' : '-');
        mvprintw(5, 16 + TW, "] %3d%%", tw_rem);
        if (tw_rem < 20) attroff(A_BOLD);

        mvprintw(14, 10, "> %s ", input);
        refresh();

        int c = getch();
        if (c == ERR) continue;
        if (c == '\n' || c == 13) break;
        if ((c == 127 || c == KEY_BACKSPACE) && len > 0)
            input[--len] = '\0';
        else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')
            input[len++] = (char)c;
    }

    timeout(-1);
    curs_set(0);

    if (len == 0) return 0;

    if (strncmp(input, correct, sizeof(correct)) == 0) {
        // correct — restore the hostname in kernel state
        ioctl(fd, KW_IOCTL_SET_HOSTNAME, correct);
        pthread_mutex_lock(&game_mutex);
        snprintf(game_shared.message, 128, "HOSTNAME RESTORED!");
        snprintf(game_shared.subtext,  128, "Check /proc/kernelware/stats");
        pthread_mutex_unlock(&game_mutex);
        sleep(2);
        return 1;   // win
    }

    pthread_mutex_lock(&game_mutex);
    snprintf(game_shared.message, 128, "WRONG! Got: %s", input);
    snprintf(game_shared.subtext,  128, "Correct was: %s", correct);
    pthread_mutex_unlock(&game_mutex);
    sleep(2);
    return 0;   // loss
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
    mvprintw(12, 10, "Type a hostname and press Enter");
}