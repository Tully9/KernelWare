// Kernel timer logic for the mini-games

#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include "kw_timer.h"
#include "kw_state.h"

static struct hrtimer game_timer;
static bool timer_active = false;

static enum hrtimer_restart kw_timer_callback(struct hrtimer *timer) {
    unsigned long flags;
    bool correct;

    spin_lock_irqsave(&game_state.lock, flags);
    correct = game_state.answer_correct;
    spin_unlock_irqrestore(&game_state.lock, flags);

    if (correct)
        kw_state_next_game();
    else
        kw_state_timeout();

    timer_active = false;

    return HRTIMER_NORESTART;
}

int kw_timer_init(void) {
    pr_info("Initialising game timer...\n");

    hrtimer_init(&game_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    game_timer.function = kw_timer_callback;

    timer_active = false;

    pr_info("Timer initialised successfully.\n");
    return 0;
}

int kw_timer_start(unsigned int timeout_ms) { // timeout_ms == time to solve mini-game
    ktime_t ktime_interval;

    if (timer_active) {
        pr_warn("Timer already running, cancelling old timer...\n");
        hrtimer_cancel(&game_timer);
    }

    pr_info("Timer started for %u milliseconds.\n", timeout_ms);

    ktime_interval = ktime_set(0, timeout_ms * 1000000ULL); // milliseconds to ktime_t

    hrtimer_start(&game_timer, ktime_interval, HRTIMER_MODE_REL); //starts timer

    timer_active = true;

    pr_info("Timer running...\n");
    return 0;
}

void kw_timer_stop(void) {
    if (timer_active) {
        pr_info("\nStopping timer...\n");

        hrtimer_cancel(&game_timer);

        timer_active = false;
        pr_info("Timer has been stopped.\n");
    }
    
    else {
        pr_info("Timer is currently stopped already.\n");
    }
}

void kw_timer_exit(void) {
    pr_info("Exiting timer...\n");

    if (timer_active) {
        hrtimer_cancel(&game_timer);
        timer_active = false;
    }

    pr_info("Timer exited successfully.\n");
}

bool kw_timer_is_active(void) {
    return timer_active;
}