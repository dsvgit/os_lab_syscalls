#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Dedkov dsv.mail@yandex.ru");
MODULE_DESCRIPTION("driver skeleton");

static int __init md_init(void) {
	printk("+++ driver skeleton: md_init()\n");
	return 0;
}

static void __exit md_exit(void) {
	printk("+++ driver skeleton: md_exit()\n");
}

module_init(md_init);
module_exit(md_exit);