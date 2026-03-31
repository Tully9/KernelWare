#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include "../../shared/kw_ioctl.h"
#include "games.h"

#define WIN_SCORE 3
#define INITIAL_LIVES 3

// Code snippet with a memory leak
typedef struct {
    const char *snippet;
    const char *description;
    int leak_line;  // 1-indexed
} leak_puzzle_t;

static leak_puzzle_t puzzles[] = {
    {
        .snippet = "1: int *ptr = malloc(100);\n"
                   "2: ptr[0] = 42;\n"
                   "3: free(ptr);\n"
                   "4: return ptr[0];",
        .description = "Which line causes the leak?",
        .leak_line = 4  // Use-after-free counts as leak
    },
    {
        .snippet = "1: char *buf = malloc(256);\n"
                   "2: strcpy(buf, userdata);\n"
                   "3: if (buf[0] == 'X')\n"
                   "4:   return -1;\n"
                   "5: free(buf);",
        .description = "Which line is the problem?",
        .leak_line = 4  // Early return without free
    },
    {
        .snippet = "1: struct node *head = NULL;\n"
                   "2: for (int i = 0; i < 10; i++) {\n"
                   "3:   head = malloc(sizeof(*head));\n"
                   "4:   head->next = NULL;\n"
                   "5:   if (i == 5) break;\n"
                   "6: }\n"
                   "7: free(head);",
        .description = "Where's the leak (loop)?",
        .leak_line = 5  // Break loses previous allocations
    },
    {
        .snippet = "1: void *data = malloc(1024);\n"
                   "2: if (!validate(data))\n"
                   "3:   goto error;\n"
                   "4: process(data);\n"
                   "5: free(data);\n"
                   "6: error:\n"
                   "7: return -1;",
        .description = "Where's the leak?",
        .leak_line = 3  // goto error skips free
    },
    {
        .snippet = "1: char *tmp = malloc(50);\n"
                   "2: tmp = realloc(tmp, 100);\n"
                   "3: if (!tmp) {\n"
                   "4:   free(tmp);\n"
                   "5:   return NULL;\n"
                   "6: }",
        .description = "Which line has the leak?",
        .leak_line = 4  // Can't free NULL after failed realloc
    }
};
static int num_puzzles = sizeof(puzzles) / sizeof(puzzles[0]);

int game_memleak_detective_run(int fd)
{
    int score = 0, lives = INITIAL_LIVES;
    int puzzle_idx = rand() % num_puzzles;

    leak_puzzle_t *current = &puzzles[puzzle_idx];

    pthread_mutex_lock(&game_mutex);
    snprintf(game_shared.message, 300, "%s", current->description);
    snprintf(game_shared.subtext,  128, "Type the line number and press Enter");
    snprintf(game_shared.prompt,   256, "%s", current->snippet);
    game_shared.typed[0] = '\0';
    game_shared.typed_len = 0;
    game_shared.fill_percent = score;
    game_shared.solved = lives;
    pthread_mutex_unlock(&game_mutex);

    while (1) {
        unsigned char event;
        ssize_t n = read(fd, &event, 1);
        if (n <= 0)
            continue;

        if (event == KW_EVENT_TIMEOUT) {
            lives--;
            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 300, "TIME'S UP! The leak was on line %d.", current->leak_line);
            snprintf(game_shared.subtext,  128, "Lives: %d", lives);
            game_shared.solved = lives;
            pthread_mutex_unlock(&game_mutex);

            if (lives <= 0) {
                GAME_SET_MSG("GAME OVER!", "You missed too many leaks.");
                sleep(1);
                ioctl(fd, KW_IOCTL_STOP);
                return 0;
            }

            // Load next puzzle
            sleep(1);
            puzzle_idx = rand() % num_puzzles;
            current = &puzzles[puzzle_idx];

            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 300, "%s", current->description);
            snprintf(game_shared.subtext,  128, "Score: %d/%d   Lives: %d", score, WIN_SCORE, lives);
            snprintf(game_shared.prompt,   256, "%s", current->snippet);
            game_shared.typed[0] = '\0';
            game_shared.typed_len = 0;
            pthread_mutex_unlock(&game_mutex);
        }

        if (event == KW_EVENT_CORRECT) {
            score++;
            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 300, "CORRECT! Line %d had the leak.", current->leak_line);
            snprintf(game_shared.subtext,  128, "Score: %d/%d", score, WIN_SCORE);
            game_shared.fill_percent = score;
            pthread_mutex_unlock(&game_mutex);

            if (score >= WIN_SCORE) {
                GAME_SET_MSG("DETECTIVE MASTERY!", "All memory leaks identified!");
                sleep(1);
                ioctl(fd, KW_IOCTL_STOP);
                return 1;
            }

            // Load next puzzle
            sleep(1);
            puzzle_idx = rand() % num_puzzles;
            current = &puzzles[puzzle_idx];

            pthread_mutex_lock(&game_mutex);
            snprintf(game_shared.message, 300, "%s", current->description);
            snprintf(game_shared.subtext,  128, "Score: %d/%d   Lives: %d", score, WIN_SCORE, lives);
            snprintf(game_shared.prompt,   256, "%s", current->snippet);
            game_shared.typed[0] = '\0';
            game_shared.typed_len = 0;
            pthread_mutex_unlock(&game_mutex);
        }
    }

    return 0;
}

void game_memleak_detective_draw(void)
{
    pthread_mutex_lock(&game_mutex);
    char msg[300], sub[128], prompt[256], typed[64];
    strncpy(msg,    game_shared.message, 300);
    strncpy(sub,    game_shared.subtext,  128);
    strncpy(prompt, game_shared.prompt,   256);
    strncpy(typed,  game_shared.typed,    64);
    pthread_mutex_unlock(&game_mutex);

    mvprintw(2,  10, "=== MEMORY LEAK DETECTIVE ===");
    mvprintw(4,  10, "%-50s", msg);
    
    // Display multi-line code snippet
    char *line_start = (char *)prompt;
    int row = 6;
    while (line_start && *line_start && row < 15) {
        char *line_end = strchr(line_start, '\n');
        int len = line_end ? (int)(line_end - line_start) : (int)strlen(line_start);
        
        char line_buf[300];
        strncpy(line_buf, line_start, len);
        line_buf[len] = '\0';
        
        mvprintw(row, 10, "  %s", line_buf);
        row++;
        
        if (!line_end) break;
        line_start = line_end + 1;
    }

    mvprintw(16, 10, "%-50s", sub);
    mvprintw(18, 10, "Enter line number: %s_", typed);
}
