// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int init_module(void)		{ printk("Hello, world\n"); return 0; }
void cleanup_module(void)	{ printk("Goodbye world\n"); }

