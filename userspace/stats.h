#ifndef STATS_H
#define STATS_H

#include <time.h>

typedef struct {
    int total_games_played;
    int total_games_won;
    int total_games_lost;
    int high_score;
    int games_won_per_type[7];  // one per game type
    int games_played_per_type[7];
    time_t last_session_time;
} stats_t;

extern stats_t global_stats;

// Load stats from file (~/.kernelware_stats)
void stats_load(void);

// Save stats to file
void stats_save(void);

// Update stats after a game session (final score, games list)
void stats_record_session(int final_score, int games_played, int games_won, const int *game_ids);

// Reset all stats
void stats_reset(void);

// Get win rate (0-100)
int stats_get_win_rate(void);

#endif
