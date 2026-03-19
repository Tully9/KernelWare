#ifndef KW_GAMES_H
#define KW_GAMES_H

#define LB_THREAD_COUNT 3

int kw_game_start(int game_id);
void kw_game_stop(void);
void kw_set_timer_ms(unsigned int ms);
extern unsigned int timer_duration_ms;

void kw_game_handle_input(unsigned char event);

int kw_games_pick(int prev);

// Game input functions
void pipe_drain(void);

void rotbrain_start(void);
int rotbrain_check_answer(const char *input);
int kw_games_pick(int prev);
unsigned char lb_kill_thread(const char *input);
int hackhost_change(const char *new_name, int len);
void kw_hackhost_win(void);
int hackhost_check_answer(const char *input);

// Proc accessors
int  kw_game_get_fill_percent(void);
bool kw_lb_get_alive(int idx);
int  kw_lb_get_pid(int idx);
bool kw_memleak_is_allocated(void);
int  kw_typefaster_get_count(void);

#define TYPEFASTER_TARGET_EXPORTED 20


#endif
