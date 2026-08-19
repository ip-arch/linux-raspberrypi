// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int init_module(void)		{
	pr_info("TASK_SIZE=%lx, PAGE_OFFSET=%lx, high_memory=%lx"
		" VMALLOC_START=%lx, VMALLOC_END=%lx\n",
	TASK_SIZE, PAGE_OFFSET, (unsigned long)high_memory, VMALLOC_START, VMALLOC_END
	); return 0; }
void cleanup_module(void)	{  }

