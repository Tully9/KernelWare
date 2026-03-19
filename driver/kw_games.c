#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/random.h>
#include <linux/string.h>
#include "../shared/kw_ioctl.h"
#include "kw_games.h"
#include "kw_driver.h"
#include <linux/completion.h>

#define PIPE_BUF_MAX ((int)(sizeof(kernel_buf) - 1))
#define FILL_BYTE 0xBB
#define FILL_INTERVAL 500

static DECLARE_COMPLETION(pipe_done);
static struct task_struct *pipe_thread;
static int active_game_id;

static int fill_count = 0; // pipe dream

static int pipe_writer_fn(void *data) {
    while (!kthread_should_stop()) {
        msleep(FILL_INTERVAL);

        if (kthread_should_stop())
            break;

        if (fill_count >= PIPE_BUF_MAX) {
            kernel_buf[0] = KW_EVENT_TIMEOUT;
            buf_len = 1;
            fill_count = 0;
        } else {
            fill_count++;
            kernel_buf[0] = FILL_BYTE;
            buf_len = 1;
            current_state.score = (fill_count * 100) / PIPE_BUF_MAX;
        }

        data_ready = 1;
        wake_up_interruptible(&my_wq);
    }
    complete(&pipe_done);
    return 0;
}

int kw_game_start(int game_id) {
    active_game_id = game_id;

    if (game_id == 3) {
        u32 pid = get_random_u32() % 65534 + 1;
        snprintf(current_state.prompt, sizeof(current_state.prompt), "%u", pid);
        current_state.score = 0;
        return 0;
    }

    fill_count = 0;
    reinit_completion(&pipe_done);
    pipe_thread = kthread_run(pipe_writer_fn, NULL, "kw_pipe_writer");

    if (IS_ERR(pipe_thread)) {
        int err = PTR_ERR(pipe_thread);
        pipe_thread = NULL;
        return err;
    }

    return 0;
}

void kw_game_stop(void) {
    if (pipe_thread) {
        if (!IS_ERR(pipe_thread))
            kthread_stop(pipe_thread);
        pipe_thread = NULL;
    }
    reinit_completion(&pipe_done);
}


// Pipe dream
void pipe_drain(void)
{
    if (fill_count > 0)
        fill_count -= 5;
    if (fill_count < 0)
        fill_count = 0;
    current_state.score = (fill_count * 100) / PIPE_BUF_MAX;

    kernel_buf[0] = FILL_BYTE;
    buf_len = 1;
    data_ready = 1;
    wake_up_interruptible(&my_wq);
}


void kw_game_handle_input(unsigned char event)
{
    switch (active_game_id) {
    case 1:  // pipe dream -> button press drains the pipe
        if (KW_EVENT_IS_PRESS(event))
            pipe_drain();
        break;

    case 2:  // memory leak -> 'A' allocates, 'F' frees
        if (event == 'A') {
            current_state.score++;
        } else if (event == 'F') {
            if (current_state.score > 0)
                current_state.score--;
            else if (current_state.lives > 0)
                current_state.lives--;
        }
        break;

    case 3:  // kill it -> compare typed PID to prompt
        if (strcmp(kernel_buf, current_state.prompt) == 0) {
            current_state.score++;
            u32 pid = get_random_u32() % 65534 + 1;
            snprintf(current_state.prompt, sizeof(current_state.prompt), "%u", pid);
            kernel_buf[0] = 0x01;
        } else {
            kernel_buf[0] = 0x00;
        }
        buf_len = 1;
        break;
    }
}


