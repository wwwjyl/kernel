/*
 * This file is for learning block reading. 
 * This file is very ugly, not thinking fllowing:
 * continue write
 * not have real data!
 * and so on
 * */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wwwjyl@sina.cn");


static int br_major = 241;

static bool is_readable = false;  /*0:no readable  1:readable*/
static struct mutex br_rw_mutex;

static wait_queue_head_t br_wait_queue;

static bool br_readable(void)
{
    bool r;
    
    mutex_lock(&br_rw_mutex);
    r = is_readable;
    mutex_unlock(&br_rw_mutex);

    return r;
}

static void br_set_reabable(bool v)
{
    mutex_lock(&br_rw_mutex);
    is_readable = v;
    mutex_unlock(&br_rw_mutex);
}

static bool br_get_readable(void)
{
    return is_readable;
}
static int brreal_open(struct inode* inode, struct file* filep)
{
    return 0;
}

static ssize_t brreal_read(struct file* filep, char *buf,
        size_t cnt, loff_t* ppos)
{
    unsigned long sz;
    bool v;
    printk("br read! start\n");

    if (filep->f_flags & O_NONBLOCK)
    {
        printk("nonblock fd!\n");
        if ( br_readable() )
        {
            goto lab_read;
        }
        else
            return -EAGAIN;
    }

    if (wait_event_interruptible(br_wait_queue, br_readable()) != 0)
    {
        return -ERESTARTSYS;
    }

lab_read:
    v = br_get_readable();
    printk("readable flag:%d\n", v);
    sz = min_t(unsigned long, sizeof(is_readable), cnt);
    if (copy_to_user(buf, &v, sz))
    {
        return -EFAULT;
    }

    br_set_reabable(false);

    printk("br read! end\n");
    return cnt;
}

static ssize_t brreal_write(struct file* filep, const char *buf,
        size_t size, loff_t* ppos)
{
    printk("br write! start\n");

    br_set_reabable(true);
    wake_up_interruptible(&br_wait_queue);

    printk("br write! end\n");
    return size;
}

static int brreal_release(struct inode* inode, struct file* filep)
{
    return 0;
}

static struct file_operations brreal_fops =
{
    .owner = THIS_MODULE,
    .open = brreal_open,
    .release = brreal_release,
    .read = brreal_read,
    .write = brreal_write,
};

static int br_open(struct inode* inode, struct file* filep)
{
    if (MINOR(inode->i_rdev) == 0)
    {
        filep->f_op = &brreal_fops;
    }
    else
        return -ENXIO;

    if (filep->f_op && filep->f_op->open)
        return filep->f_op->open(inode, filep);

    return 0;
}

static struct file_operations br_fops =
{
    .owner = THIS_MODULE,
    .open = br_open,
};

static struct class* br_class;
static struct device* br_dev;

static int __init br_init(void)
{
    int ret = 0;

    printk("br_init call!\n");

    if (register_chrdev(br_major, "br", &br_fops) != 0)
    {
        printk("register br error!\n");
        return -EIO;
    }

    init_waitqueue_head(&br_wait_queue);
    mutex_init(&br_rw_mutex);

    /*for see "br" under /dev*/
    br_class = class_create(THIS_MODULE, "br_class");
    if (NULL == br_class)
    {
        printk("class create error!\n");
        ret = -EIO;
        goto lab_unreg;
    }

    /*for see "br" under /dev*/
    br_dev = device_create(br_class, NULL, MKDEV(br_major, 0),
            NULL, "br");
    if (NULL == br_dev)
    {
        printk("device create error!\n");
        ret = -EIO;
        goto lab_cls;
    }


    return 0;

lab_cls:
    class_destroy(br_class);

lab_unreg:
    unregister_chrdev(br_major, "br");

    return ret;
}


static void __exit br_exit(void)
{
    printk("br_exit call!\n");

    device_destroy(br_class, MKDEV(br_major, 0)); 
    class_destroy(br_class);
    unregister_chrdev(br_major, "br");
}

module_init(br_init);
module_exit(br_exit);

