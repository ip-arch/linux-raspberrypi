// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/init.h>
static int dec=0;
static char* name="world";

module_param(dec, int, 0);
module_param(name, charp, 0);

int __init init_hello(void)		{ printk("Hello, %s(%d)\n", name, dec); return 0; }
void __exit cleanup_hello(void)		{ printk("Goodbye %s\n", name); }

module_init(init_hello);
module_exit(cleanup_hello);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

