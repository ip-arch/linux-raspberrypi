// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int init_module(void)		{
	printk("TASK_SIZE=%p, PAGE_OFFSET=%p, high_memory=%p"
		" VMALLOC_START=%p, VMALLOC_END=%p\n",
	TASK_SIZE, PAGE_OFFSET, high_memory, VMALLOC_START, VMALLOC_END
	); return 0; }
void cleanup_module(void)	{  }

