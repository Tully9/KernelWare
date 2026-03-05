#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;


static struct file_operations fops = {
    .owner   = THIS_MODULE,
    //.open    = ,
    //.read    = ,
    //.write   = ,
};


static int __init my_module_init(void) {
    printk(KERN_INFO "KernelWare: loaded\n");
    return 0;
}

static void __exit my_module_exit(void) {
    printk(KERN_INFO "KernelWare: unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");