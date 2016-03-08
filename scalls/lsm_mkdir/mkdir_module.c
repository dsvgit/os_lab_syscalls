#include <linux/init.h>
#include <linux/module.h>

#include <linux/moduleparam.h>
#include <linux/unistd.h>

#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/uidgid.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Dedkov Sergey" );

static int uid;
module_param(uid, int, 0644);

unsigned long **sys_call_table;
unsigned long original_cr0;

asmlinkage int (* ref_sys_mkdir) (struct inode *dir, struct dentry *dentry, int mode);
asmlinkage int new_sys_mkdir (struct inode *dir, struct dentry *dentry, int mode) {
	if (uid == get_current_user()->uid.val)
		return -1;
	printk ("mkdir hijacked! uid: %d \n", get_current_user()->uid.val);
	printk ("mkdir hijacked! passed uid: %d \n", uid);
	return ref_sys_mkdir(dir, dentry, mode);
}

asmlinkage int (* ref_sys_fstat) (unsigned int fd, struct kstat *stat);

static unsigned long **aquire_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;

	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;

		if (sct[__NR_close] == (unsigned long *) sys_close)
			return sct;

		offset += sizeof(void *);
	}

	return NULL;
}

static int __init md_init(void)
{
	if(!(sys_call_table = aquire_sys_call_table()))
		return -1;

	original_cr0 = read_cr0();

	write_cr0(original_cr0 & ~0x00010000);
	ref_sys_mkdir = (void *)sys_call_table[__NR_mkdir];
	sys_call_table[__NR_mkdir] = (unsigned long *)new_sys_mkdir;
	write_cr0(original_cr0);

	return 0;
}

static void __exit md_exit(void)
{
	if(!sys_call_table) {
		return;
	}

	write_cr0(original_cr0 & ~0x00010000);
	sys_call_table[__NR_mkdir] = (unsigned long *)ref_sys_mkdir;
	write_cr0(original_cr0);

	msleep(2000);
}

module_init(md_init);
module_exit(md_exit);
