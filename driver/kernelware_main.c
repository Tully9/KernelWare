#include "kernelware.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>


static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;

static DECLARE_WAIT_QUEUE_HEAD(my_wq);  // wait queue
static int data_ready = 0;              // condition flag
static char kernel_buf[256];
static int buf_len = 0;

// Define the shared state here
struct my_driver_state drv_state = {0};


static ssize_t read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    if (*off > 0) return 0;

    if (wait_event_interruptible(my_wq, data_ready != 0))
        return -ERESTARTSYS;  // woken by signal not data

    int bytes = min(len, (size_t)buf_len);
    if (copy_to_user(buf, kernel_buf, bytes))
        return -EFAULT;

    data_ready = 0;

    *off += bytes; // advance the offset so the next call returns 0 (EOF) 
    return bytes;
}


static ssize_t write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    int bytes = min(len, sizeof(kernel_buf) - 1);

    if (copy_from_user(kernel_buf, buf, bytes))
        return -EFAULT;

    buf_len = bytes;
    kernel_buf[bytes] = '\0';

    data_ready = 1;
    wake_up_interruptible(&my_wq);

    return bytes;
}


static struct file_operations fops = {
    .owner = THIS_MODULE,
    //.open = ,
    .read = read,
    .write = write,
};

//sets permissions
static char *kernelware_devnode(const struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = 0666;
    return NULL;
}

static int __init my_module_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, "kernelware") < 0) {
        pr_err("KernelWare: failed to alloc dev region\n");
        return -1;
    }

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        pr_err("KernelWare: failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    my_class = class_create("kernelware");
    if (IS_ERR(my_class)) {
        pr_err("KernelWare: failed to create class\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }
    my_class->devnode = kernelware_devnode; // fixes permissions 
    
    device_create(my_class, NULL, dev_num, NULL, "kernelware");

    if (my_proc_init() != 0) {
        device_destroy(my_class, dev_num);
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    pr_info("KernelWare: loaded\n");
    return 0;
}

static void __exit my_module_exit(void) {
    my_proc_exit();
    printk(KERN_INFO "KernelWare: unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");