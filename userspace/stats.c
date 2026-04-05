#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

stats_t global_stats = {0};

static const char *stats_file_path(void)
{
    static char path[256] = {0};
    if (!path[0]) {
        const char *home = getenv("HOME");
        if (!home) home = "/tmp";
        snprintf(path, sizeof(path), "%s/.kernelware_stats", home);
    }
    return path;
}

void stats_load(void)
{
    const char *path = stats_file_path();
    FILE *f = fopen(path, "rb");
    if (!f) {
        memset(&global_stats, 0, sizeof(global_stats));
        return;
    }
    
    fread(&global_stats, sizeof(global_stats), 1, f);
    fclose(f);
}

void stats_save(void)
{
    const char *path = stats_file_path();
    FILE *f = fopen(path, "wb");
    if (!f) return;
    
    fwrite(&global_stats, sizeof(global_stats), 1, f);
    fclose(f);
}

void stats_record_session(int final_score, int games_played, int games_won, const int *game_ids)
{
    global_stats.total_games_played += games_played;
    global_stats.total_games_won += games_won;
    global_stats.total_games_lost += (games_played - games_won);
    
    if (final_score > global_stats.high_score) {
        global_stats.high_score = final_score;
    }
    
    global_stats.last_session_time = time(NULL);
    
    // Track per-game stats - assume games_won is tracked separately via won flag
    // For now, we track games_played_per_type
    // To properly track wins, we'd need to pass a won_array or modify the interface
    if (game_ids) {
        for (int i = 0; i < games_played; i++) {
            if (game_ids[i] >= 0 && game_ids[i] < 7) {
                global_stats.games_played_per_type[game_ids[i]]++;
            }
        }
    }
    
    stats_save();
}

void stats_reset(void)
{
    memset(&global_stats, 0, sizeof(global_stats));
    stats_save();
}

int stats_get_win_rate(void)
{
    if (global_stats.total_games_played == 0) return 0;
    return (global_stats.total_games_won * 100) / global_stats.total_games_played;
}
