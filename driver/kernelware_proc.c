#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include "kernelware.h"

#define PROC_NAME "kernelware"

static int my_proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "open_count:  %d\n", drv_state.open_count);
    seq_printf(m, "read_count:  %d\n", drv_state.read_count);
    seq_printf(m, "write_count: %d\n", drv_state.write_count);
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, my_proc_show, NULL);
}

static const struct proc_ops my_proc_ops = {
    .proc_open    = my_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

int my_proc_init(void) {
    if (!proc_create(PROC_NAME, 0444, NULL, &my_proc_ops)) {
        printk(KERN_ERR "KernelWare: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
    printk(KERN_INFO "KernelWare: created /proc/%s\n", PROC_NAME);
    return 0;
}

void my_proc_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "KernelWare: removed /proc/%s\n", PROC_NAME);
}