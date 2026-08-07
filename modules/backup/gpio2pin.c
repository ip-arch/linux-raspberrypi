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
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/pinctrl/consumer.h>

#define SW 2
#define LED 4

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

struct bcm2708_gpio {
        struct list_head list;
        void __iomem *base;
        struct gpio_chip gc;
};



struct gpio_desc *gd;
struct gpio_chip *gc;
struct device *dev;
struct pinctrl *pc;
struct platform_device *pdev;
struct bcm2708_gpio *p;

int init_module(void)		{
	pinctrl_request_gpio(LED);
	pinctrl_request_gpio(SW);
	pinctrl_gpio_direction_output(LED);
	gpio_set_value(LED,0);
	pinctrl_gpio_direction_input(SW);
	gd = gpio_to_desc(SW);
	gc = gpiod_to_chip(gd);
	dev = gc->dev;
	pdev = to_platform_device(dev);
	p=(struct bcm2708_gpio *)((pdev->dev).driver_data);
	pc = pinctrl_get(dev);
	printk("Hello pc=%p, pdev=%p,p=%p\n, pdev->name=%s", pc, pdev, p->base, pdev->name); 
	return 0;
}

void cleanup_module(void)	{
	pinctrl_free_gpio(LED);
	pinctrl_free_gpio(SW);
	printk("Goodbye GPIO\n");
}
