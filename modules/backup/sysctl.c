/*
 *   This sample source codes are design for a seminar.
 *
 *   Copyright (c) 2005-2012 Naohiko Shimizu, All Rights Reserved.
 *   Author: Naohiko Shimizu : nshimizu@ip-arch.jp
 *
 *   Everyone is permitted to copy and distribute verbatim copies of
 *   this source code under GPL Version 2.
 */
#include <linux/module.h>
#include <linux/sysctl.h>

#define CTL_SYSCTLTEST 6502
#define CTL_COUNT1 1
#define CTL_COUNT2 2

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");


static int count1=100,count2=200;

static struct ctl_table proctest_table[] = {
   {"count1", &count1, sizeof(int), 0644, NULL, &proc_dointvec, NULL, NULL},
   {"count2", &count2, sizeof(int), 0644, NULL, &proc_dointvec, NULL, NULL},
   {0}
};

static struct ctl_table sys_table[] = {
   {"sysctl_test", NULL, 0, 0555, proctest_table, NULL, NULL},
   {0}
};

struct ctl_table_header *s;

int init_module(void)		{
	printk("Hello sysctl\n");
	s=register_sysctl_table(sys_table);
	return 0;
}
void cleanup_module(void)	{
	printk("Goodbye sysctl\n");
	unregister_sysctl_table(s);
}

