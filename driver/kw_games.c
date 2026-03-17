#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/err.h>
#include "../shared/kw_ioctl.h"
#include "kw_games.h"

extern wait_queue_head_t my_wq;
extern int data_ready;
extern char kernel_buf[256];
extern int buf_len;

#define PIPE_BUF_MAX ((int)(sizeof(kernel_buf) - 1))
#define FILL_BYTE 0xBB
#define FILL_INTERVAL 500

static struct task_struct *pipe_thread;
static int active_game_id;

static int pipe_writer_fn(void *data) {
    while (!kthread_should_stop()) {
        msleep(FILL_INTERVAL);

        if (kthread_should_stop())
            break;

        if (buf_len >= PIPE_BUF_MAX) {
            kernel_buf[0] = KW_EVENT_TIMEOUT;
            buf_len = 1;
        }
	
	else {
            kernel_buf[buf_len++] = FILL_BYTE;
        }

        data_ready = 1;
        wake_up_interruptible(&my_wq);
    }

    return 0;
}

int kw_game_start(int game_id) {
    active_game_id = game_id;

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
        kthread_stop(pipe_thread);
        pipe_thread = NULL;
    }
}
