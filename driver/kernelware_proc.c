#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include "kernelware.h"
#include "kw_driver.h"
#include "kw_games.h"

#define PROC_DIR_NAME  "kernelware"
#define PROC_STATS_NAME     "stats"
#define PROC_LEADERBOARD_NAME "leaderboard"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_stats;
static struct proc_dir_entry *proc_leaderboard;

static const char *game_name(int id)
{
    switch (id) {
    case 1: return "Pipe Dream";
    case 2: return "Rot Brain";
    case 3: return "Kill It";
    case 4: return "Memory Leak";
    case 5: return "Type Faster";
    case 6: return "Load Balancer";
    case 7: return "Hack the Host";
    default: return "None";
    }
}

static int stats_show(struct seq_file *m, void *v)
{
    int game_id = current_state.game_id;
    int i;

    seq_printf(m, "=== Session ===\n");
    seq_printf(m, "score:       %d\n", current_state.session_score);
    seq_printf(m, "lives:       %d\n", current_state.session_lives);
    seq_printf(m, "game:        %s (%d)\n", game_name(game_id), game_id);

    seq_printf(m, "\n=== Driver ===\n");
    seq_printf(m, "open_count:  %d\n", drv_state.open_count);
    seq_printf(m, "read_count:  %d\n", drv_state.read_count);
    seq_printf(m, "write_count: %d\n", drv_state.write_count);

    if (game_id < 1)
        return 0;

    seq_printf(m, "\n=== %s ===\n", game_name(game_id));
    switch (game_id) {
    case 1:  // Pipe Dream
        seq_printf(m, "pipe_fill:   %d%%\n", kw_game_get_fill_percent());
        break;

    case 2:  // Rot Brain 
        if (current_state.prompt[0])
            seq_printf(m, "scrambled:   %s\n", current_state.prompt);
        break;

    case 3:  // Kill It 
        if (current_state.prompt[0])
            seq_printf(m, "target_pid:  %s\n", current_state.prompt);
        break;

    case 4:  // Memory Leak
        seq_printf(m, "allocated:   %s\n", kw_memleak_is_allocated() ? "yes" : "no");
        seq_printf(m, "ml_lives:    %d\n", current_state.lives);
        break;

    case 5:  // Type Faster
        seq_printf(m, "keypresses:  %d / %d\n",
                   kw_typefaster_get_count(), TYPEFASTER_TARGET_EXPORTED);
        break;

    case 6:  // Load Balancer
        for (i = 0; i < LB_THREAD_COUNT; i++) {
            if (kw_lb_get_alive(i))
                seq_printf(m, "kw_lb_%d:     running (pid %d)\n",
                           i + 1, kw_lb_get_pid(i));
            else
                seq_printf(m, "kw_lb_%d:     dead\n", i + 1);
        }
        break;

    case 7:  // Hack the Host
        seq_printf(m, "scrambled:   %s\n", init_uts_ns.name.nodename);
        if (current_state.prompt[0])
            seq_printf(m, "original:    %s\n", current_state.prompt);
        break;

    default:
        if (current_state.prompt[0])
            seq_printf(m, "prompt:      %s\n", current_state.prompt);
        break;
    }
    return 0;
}

static int stats_open(struct inode *inode, struct file *file)
{
    return single_open(file, stats_show, NULL);
}

static const struct proc_ops stats_ops = {
    .proc_open    = stats_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int leaderboard_show(struct seq_file *m, void *v)
{
    seq_printf(m, "=== KernelWare Leaderboard ===\n");
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        if (leaderboard[i] > 0)
            seq_printf(m, "%d. %d\n", i + 1, leaderboard[i]);
        else
            seq_printf(m, "%d. ---\n", i + 1);
    }
    return 0;
}

static int leaderboard_open(struct inode *inode, struct file *file)
{
    return single_open(file, leaderboard_show, NULL);
}

static const struct proc_ops leaderboard_ops = {
    .proc_open    = leaderboard_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

int my_proc_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (!proc_dir) {
        pr_err("KernelWare: failed to create /proc/%s\n", PROC_DIR_NAME);
        return -ENOMEM;
    }

    proc_stats = proc_create(PROC_STATS_NAME, 0444, proc_dir, &stats_ops);
    if (!proc_stats) {
        pr_err("KernelWare: failed to create /proc/%s/%s\n", PROC_DIR_NAME, PROC_STATS_NAME);
        remove_proc_entry(PROC_DIR_NAME, NULL);
        return -ENOMEM;
    }

    proc_leaderboard = proc_create(PROC_LEADERBOARD_NAME, 0444, proc_dir, &leaderboard_ops);
    if (!proc_leaderboard) {
        pr_err("KernelWare: failed to create /proc/%s/%s\n", PROC_DIR_NAME, PROC_LEADERBOARD_NAME);
        remove_proc_entry(PROC_STATS_NAME, proc_dir);
        remove_proc_entry(PROC_DIR_NAME, NULL);
        return -ENOMEM;
    }

    pr_info("KernelWare: created /proc/%s/{%s,%s}\n",
            PROC_DIR_NAME, PROC_STATS_NAME, PROC_LEADERBOARD_NAME);
    return 0;
}

void my_proc_exit(void)
{
    remove_proc_entry(PROC_LEADERBOARD_NAME, proc_dir);
    remove_proc_entry(PROC_STATS_NAME, proc_dir);
    remove_proc_entry(PROC_DIR_NAME, NULL);
    pr_info("KernelWare: removed /proc/%s\n", PROC_DIR_NAME);
}
