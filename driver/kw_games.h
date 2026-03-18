#ifndef KW_GAMES_H
#define KW_GAMES_H

int kw_game_start(int game_id);
void kw_game_stop(void);

void kw_game_handle_input(unsigned char event);


// Game input functions
void pipe_drain(void);

#endif
