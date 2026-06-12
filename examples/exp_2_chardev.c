#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#define DEVICE_NAME "mychardev"
#define CLASS_NAME  "mychar"
#define BUF_SIZE    128

static dev_t devno;
static struct cdev my_cdev;
static struct class *my_class;

static char data_buf[BUF_SIZE];
static size_t data_len;
static DEFINE_MUTEX(data_lock);
static DECLARE_WAIT_QUEUE_HEAD(read_wq);
static bool data_ready;

static ssize_t my_read(struct file *filp, char __user *buf,
                       size_t count, loff_t *ppos)
{
    ssize_t ret;

    if (wait_event_interruptible(read_wq, data_ready))
        return -ERESTARTSYS;

    mutex_lock(&data_lock);

    if (count > data_len)
        count = data_len;

    if (copy_to_user(buf, data_buf, count)) {
        ret = -EFAULT;
        goto out;
    }

    data_ready = false;
    ret = count;

out:
    mutex_unlock(&data_lock);
    return ret;
}

static ssize_t my_write(struct file *filp, const char __user *buf,
                        size_t count, loff_t *ppos)
{
    ssize_t ret;

    if (count > BUF_SIZE)
        count = BUF_SIZE;

    mutex_lock(&data_lock);

    if (copy_from_user(data_buf, buf, count)) {
        ret = -EFAULT;
        goto out;
    }

    data_len = count;
    data_ready = true;
    wake_up_interruptible(&read_wq);
    ret = count;

out:
    mutex_unlock(&data_lock);
    return ret;
}

static int my_open(struct inode *inode, struct file *filp)
{
    pr_info("device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
    pr_info("device closed\n");
    return 0;
}

static const struct file_operations my_fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
    .write   = my_write,
};

static int __init my_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, devno, 1);
    if (ret)
        goto err_unregister;

    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        ret = PTR_ERR(my_class);
        goto err_cdev;
    }

    device_create(my_class, NULL, devno, NULL, DEVICE_NAME);

    pr_info("mychardev loaded: major=%d minor=%d\n",
            MAJOR(devno), MINOR(devno));
    return 0;

err_cdev:
    cdev_del(&my_cdev);
err_unregister:
    unregister_chrdev_region(devno, 1);
    return ret;
}

static void __exit my_exit(void)
{
    device_destroy(my_class, devno);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(devno, 1);
    pr_info("mychardev unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
