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
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

static int advance = 100;
struct gpio_desc *gpio_led;

struct timer_list timer_x;
int led = 0;
void timer_timeout(struct timer_list *tm)
{
	led = 1 - led;
	gpiod_set_value(gpio_led,led);
	tm->expires = jiffies + msecs_to_jiffies(advance);
	add_timer(tm);
}

static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

	pr_info("Hello timer\n");
    gpio_led = devm_gpiod_get_index(dev, "ex_led", 0, GPIOD_OUT_LOW);
    if (!gpio_led) {
        pr_err("Failed to get LED gpio descriptor\n");
        return -EINVAL;
    }
    gpiod_set_value(gpio_led, led);
    timer_setup(&timer_x, timer_timeout, 0);
    timer_x.expires = jiffies + msecs_to_jiffies(advance);
    add_timer(&timer_x);
    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    del_timer(&timer_x);
    pr_info("Goodbye timer\n");
    return ;
}

static const struct of_device_id gpiod_dt_ids[] = {
	{ .compatible = "shmz,exboard",},
	{ /* sentinel */},
};

static struct platform_driver my_probe = {
	.probe = my_init,
	.remove = my_remove,
	.driver = {
		.name = "exboard-shmz" ,
		.of_match_table = gpiod_dt_ids,
		.owner = THIS_MODULE,
	},
};



MODULE_DEVICE_TABLE(of, gpiod_dt_ids);
module_platform_driver(my_probe);

