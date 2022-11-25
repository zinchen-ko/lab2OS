#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/vmalloc.h>

#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/net.h>


#define WR_VALUE _IOW('a','a',struct message*)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Stab linux module for labs");
MODULE_VERSION("1.0");

void fill_structs(void);

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

struct task_struct_info {
	int prio;
	int static_prio;
	int normal_prio;
	unsigned int rt_priority;

};

struct socket_info {
	short type;
	unsigned long flags;
};

struct message {
	struct task_struct_info* tsi;
	struct socket_info* si;
	pid_t pid;
	int fd;
	int *err;
};


static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read = etx_read,
	.write = etx_write,
	.open = etx_open,
	.unlocked_ioctl = etx_ioctl,
	.release = etx_release,
};

struct message msg;
struct task_struct* task;
struct socket *sk;

static int etx_open(struct inode *inode, struct file *file) {
	pr_info("Device File Opened...!!!\n");
	return 0;
}


static int etx_release(struct inode *inode, struct file *file) {
	pr_info("Device File Closed...!!!\n");
	return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
	pr_info("Read Function\n");
	return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
	pr_info("Write function\n");
	return len;
}

static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	switch(cmd) {
		case WR_VALUE:
			if(copy_from_user(&msg ,(struct message*) arg, sizeof(msg))) {
                                pr_err("Data Write : Err!\n");
                        }
			pr_info("Pid = %d\n", msg.pid);
			fill_structs();
			break;
		default:
			pr_info("Default\n");
			break;
	}
	return 0;
}

static int __init etx_driver_init(void) {
	if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0) {
		pr_err("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	cdev_init(&etx_cdev,&fops);

	if((cdev_add(&etx_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_class;
	}

	if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}

	if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
		pr_err("Cannot create the Device 1\n");
		goto r_device;
	}
	pr_info("Device Driver Insert...Done!!!\n");
	return 0;

	r_device:
	class_destroy(dev_class);
	r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

void fill_structs() {
	task = get_pid_task(find_get_pid(msg.pid), PIDTYPE_PID);
	if (task != NULL) {
		msg.tsi->prio = task->prio;
		msg.tsi->static_prio = task->static_prio;
		msg.tsi->normal_prio = task->normal_prio;
		msg.tsi->rt_priority = task->rt_priority;
		pr_info("Prio = %d\n", task->prio);
		pr_info("Prio Static = %d\n", task->static_prio);
		pr_info("Prio Normal= %d\n", task->normal_prio);
		pr_info("Prio RT = %d\n", task->rt_priority);
    	}
	
	sk = sockfd_lookup(msg.fd, msg.err);
	//if (sk != NULL) {
	//	msg.si->type = sk->type;
	//	msg.si->flags = sk->flags;
	//	pr_info("Type = %d\n", sk->type);
	//	pr_info("Flags = %lu\n", sk->flags);
	//}
	msg.si->type = sk->type;
	msg.si->flags = sk->flags;
	pr_info("Type = %d\n", sk->type);
	pr_info("Flags = %lu\n", sk->flags);
}

static void __exit etx_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove...Done!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
