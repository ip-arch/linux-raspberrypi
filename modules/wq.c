// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

#define LED	4
#define SW	2

struct work_struct work_exec;
int led = 0;
void work_body(struct work_struct *arg)
{
        gpio_set_value(LED,1 - gpio_get_value(SW));
	schedule_work(&work_exec);
}

int init_module(void)	{
	printk("Hello WorkQueue\n");
        pinctrl_request_gpio(LED);
        pinctrl_gpio_direction_output(LED);
        gpio_set_value(LED,led);
        pinctrl_request_gpio(SW);
        pinctrl_gpio_direction_input(SW);

	INIT_WORK(&work_exec,work_body);
	/*
	work_exec.func = work_body;
	*/
	schedule_work(&work_exec);
	return 0;
}
void cleanup_module(void)	{
        pinctrl_free_gpio(LED);
        pinctrl_free_gpio(SW);
	cancel_work_sync(&work_exec);
	printk("Goodbye WorkQueue\n");
}

