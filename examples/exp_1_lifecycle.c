#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "my_kmod_info"

static int my_show(struct seq_file *m, void *v)
{
    seq_printf(m, "module: my_kmod\n");
    seq_printf(m, "status: loaded\n");
    return 0;
}

static int my_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_show, NULL);
}

static const struct proc_ops my_proc_ops = {
    .proc_open    = my_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init my_module_init(void)
{
    struct proc_dir_entry *entry;

    entry = proc_create(PROC_NAME, 0444, NULL, &my_proc_ops);
    if (!entry) {
        pr_err("failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("my_kmod loaded\n");
    return 0;
}

static void __exit my_module_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("my_kmod unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple procfs kernel module example");
