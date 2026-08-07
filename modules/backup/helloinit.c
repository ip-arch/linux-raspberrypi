/*
 *   This sample source codes are design for a seminar.
 *
 *   Copyright (c) 2005-2015 Naohiko Shimizu, All Rights Reserved.
 *   Author: Naohiko Shimizu : nshimizu@ip-arch.jp
 *
 *   Everyone is permitted to copy and distribute verbatim copies of
 *   this source code under GPL Version 2.
 */
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

