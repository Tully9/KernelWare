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

static ssize_t my_proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    char buf[64];
    int val;

    //cap to avoid overflow
    if (count > sizeof(buf)) {
        count = sizeof(buf) - 1;
    }

    //can't dereference ubuf (userspace pointer) directly - copy instead
    if (copy_from_user(buf, ubuf, count)) {
        return -EFAULT; //returns number of bytes failed to copy - 0 = success
    }

    buf[count] = '\0';

    //test
    //strip newline from echo
    if (count > 0 && buf[count - 1] == '\n') {
        buf[count - 1] = '\0';
    }

    if (strcmp (buf, "reset") == 0) {
        drv_state.open_count = 0;
        drv_state.read_count = 0;
        drv_state.write_count = 0;
        printk(KERN_INFO "KernelWare: test reset\n");
    } else if (strcmp (buf, "wah") == 0) {
        printk(KERN_INFO "WarioWare mode activated\n");
    } else {
        printk(KERN_INFO "KernelWare: unknown command\n");
        return -EFAULT;
    }

    return count;
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