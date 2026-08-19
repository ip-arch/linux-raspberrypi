// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

void* mmap_area=0;
char* mmap_vm;

static ssize_t my_read(struct file *file, char __user *buf,
size_t byte, loff_t *loff)
{
    int res;
    if(byte != 4) return -EFAULT;
    res = copy_to_user((int *)buf, mmap_area,sizeof(int));
    return 4;
}
static ssize_t my_write(struct file *file, const char *buf,
size_t byte, loff_t *loff)
{
    int res;
    if(byte != 4) return -EFAULT;
    res = copy_from_user(mmap_area, (int *)buf, sizeof(int));
    return 4;
}


void vma_open(struct vm_area_struct *vma)
{
	printk(KERN_NOTICE "mmap open. virt : %lx, phys : %lx\n ", vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

void vma_close(struct vm_area_struct *vma)
{
	printk(KERN_NOTICE "mmap close.\n");
}

static struct vm_operations_struct remap_vm_ops = {
	.open = vma_open,
	.close = vma_close,
};

static int remap_mm(struct file *filp, struct vm_area_struct *vma) {
printk("calling remap_mm(%lx:%lx)\n",vma->vm_start, vma->vm_end);
printk("pgoff=(%x)\n", vma->vm_pgoff);
	vma->vm_flags |= VM_IO;
	vma->vm_pgoff=virt_to_phys(mmap_area)>>PAGE_SHIFT;

	if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
		vma->vm_end - vma->vm_start,
		vma->vm_page_prot))
	return -EAGAIN;

	vma->vm_ops = &remap_vm_ops;
	vma_open(vma);
	return 0;
}

static struct file_operations mmap_fops = {
 mmap: remap_mm,
 read: my_read,
 write: my_write,
 open: NULL,
 release: NULL,
};

static dev_t dev;
static struct class *sClass;
static struct device *sDevice;
static struct cdev c_dev;

int init_module(void)		{
	mmap_area=(void*)__get_free_page(GFP_KERNEL);
	*(int*)(mmap_area)=0x12345678;
	printk("Hello mmap(%px)\n",(void*)mmap_area);
	alloc_chrdev_region(&dev,0,1,"mmap_test");
	sClass=class_create(THIS_MODULE,"Mmap");
	sDevice=device_create(sClass,NULL,dev,NULL,"Mmap");
	cdev_init(&c_dev, &mmap_fops);
	cdev_add(&c_dev, dev, 1);
	return 0;
}
void cleanup_module(void)	{
	printk("Goodbye mmap\n");
	cdev_del(&c_dev);
	device_destroy(sClass,dev);
	class_destroy(sClass);
	unregister_chrdev_region(dev,1);
	free_page((unsigned int)mmap_area);
}

