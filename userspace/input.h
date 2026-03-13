#ifndef INPUT_H
#define INPUT_H

extern volatile char last_key;

void input_init(int fd);
void *kw_input_thread(void *arg);

#endif