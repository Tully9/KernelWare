#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include "../../shared/kw_ioctl.h"
#include "games.h"

extern volatile int input_active;
extern volatile int currentScreen;

int game_killit_run(int fd)
{
    //ioctl(fd, KW_IOCTL_START, 3);

    struct kw_state state;
    memset(&state, 0, sizeof(state));
    ioctl(fd, KW_IOCTL_GET_STATE, &state);

    pthread_mutex_lock(&game_mutex);
    snprintf(game_shared.message, 300, "KILL PID: %s", state.prompt);
    snprintf(game_shared.subtext,  128, "Type the PID and press Enter");
    game_shared.typed[0] = '\0';
    game_shared.typed_len = 0;
    pthread_mutex_unlock(&game_mutex);

    int won = 0;
    unsigned char event;
    while (1) {
        ssize_t n = read(fd, &event, 1);
        if (n <= 0) break;

        if (event == KW_EVENT_CORRECT) {
            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 300, "CORRECT! PID eliminated.");
            game_shared.subtext[0] = '\0';
            pthread_mutex_unlock(&game_mutex);
            won = 1;
            break;
        } else if (event == KW_EVENT_TIMEOUT) {
            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 300, "TIME'S UP!");
            game_shared.subtext[0] = '\0';
            pthread_mutex_unlock(&game_mutex);
            break;
        }
    }

    ioctl(fd, KW_IOCTL_STOP);
    sleep(2);
    currentScreen = 0;
    return won;
}

void game_killit_draw(void)
{
    pthread_mutex_lock(&game_mutex);
    char msg[300], sub[128], typed[64];
    strncpy(msg,   game_shared.message, 300);
    strncpy(sub,   game_shared.subtext,  128);
    strncpy(typed, game_shared.typed,    64);
    pthread_mutex_unlock(&game_mutex);

    mvprintw(7,  10, "=== KILL IT ===");
    mvprintw(9,  10, "%-40s", msg);
    mvprintw(10, 10, "%-40s", sub);
    mvprintw(12, 10, "Type the PID and press Enter");
    mvprintw(12, 10, "Your input: %s_", typed);
    mvprintw(14, 10, "[0-9] type | [Enter] submit | [Backspace] delete");
}
