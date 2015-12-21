#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <asm/paravirt.h>

unsigned long **sys_call_table;
unsigned long original_cr0;

asmlinkage int (* ref_sys_mkdir) (struct inode *dir, struct dentry *dentry, int mode);
asmlinkage int new_sys_mkdir (struct inode *dir, struct dentry *dentry, int mode) {
    printk ("mkdir hijacked!\n");
    return ref_sys_mkdir(dir, dentry, mode);
}

asmlinkage int (* ref_sys_fstat) (unsigned int fd, struct kstat *stat);

// find sys call table
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

// change sys call
static int __init interceptor_start(void) 
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

// revert changes
static void __exit interceptor_end(void) 
{
    if(!sys_call_table) {
        return;
    }
    
    write_cr0(original_cr0 & ~0x00010000);
    sys_call_table[__NR_mkdir] = (unsigned long *)ref_sys_mkdir;
    write_cr0(original_cr0);
    
    msleep(2000);
}

module_init(interceptor_start);
module_exit(interceptor_end);

MODULE_LICENSE("GPL");