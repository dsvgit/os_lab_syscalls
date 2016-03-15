#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Dedkov");
MODULE_DESCRIPTION("Test Character Driver");

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static char   message[256] = {0};
static short  size_of_message;

static int		dev_open(struct inode *, struct file *);
static int		dev_release(struct inode *, struct file *);
static ssize_t	dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t	dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write
};

static int __init chr_init(void) /* Constructor */
{
	if (alloc_chrdev_region(&first, 0, 1, "chr_test_driver") < 0)
	{
		return -1;
	}
	
	if ((cl = class_create(THIS_MODULE, "char_test_drv")) == NULL)
	{
		unregister_chrdev_region(first, 1);
		return -1;
	}
	
	if (device_create(cl, NULL, first, NULL, "chrnull") == NULL)
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	
	cdev_init(&c_dev, &fops);
	
	if (cdev_add(&c_dev, first, 1) == -1)
	{
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}

	return 0;
}

static void __exit chr_exit(void) /* Destructor */
{
	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
}

static int dev_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Character driver: open()\n");
	return 0;
}

static int dev_release(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Character driver: close()\n");
	return 0;
}

static ssize_t dev_read(struct file *f, char __user *buffer, size_t len, loff_t *offset)
{
	int error_count = 0;
	int size = 0;
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	error_count = copy_to_user(buffer, message, size_of_message);

	if (error_count != 0) {            // if true then have success
		printk(KERN_INFO "Failed to send %d characters to the user\n", error_count);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
	
	printk(KERN_INFO "Sent %d characters to the user\n", size_of_message);
	size = size_of_message;
	size_of_message = 0;
	return size;  // clear the position to the start and return 0
}

static ssize_t dev_write(struct file *f, const char __user *buffer, size_t len, loff_t *offset)
{
	sprintf(message, "%s(%d letters) \n", buffer, (int)len);   // appending received string with its length
	size_of_message = strlen(message);                 // store the length of the stored message
	printk(KERN_INFO "Received %d characters from the user\n", (int)len);
	return len;
}

module_init(chr_init);
module_exit(chr_exit);