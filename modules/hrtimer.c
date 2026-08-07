// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

static int ledlevel = 1;
struct gpio_desc *gpio_led;

struct hrtimer hrt;
ktime_t kt;
int count = 2000;
enum hrtimer_restart timer_timeout(struct hrtimer *timer)
{
	ledlevel = 1 - ledlevel;
        gpiod_set_value(gpio_led,ledlevel);
	if(count) {
		count--;
		hrtimer_forward_now(timer,kt);
		return HRTIMER_RESTART;
		}
	return HRTIMER_NORESTART;
}
static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

	pr_info("Hello hrtimer\n");
    gpio_led = devm_gpiod_get_index(dev, "ex_led", 0, GPIOD_OUT_LOW);
    if (!gpio_led) {
        pr_err("Failed to get LED gpio descriptor\n");
        return -EINVAL;
    }
    gpiod_set_value(gpio_led, ledlevel);
	kt=ktime_set(0,100000000UL);
	hrtimer_init(&hrt, CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	hrt.function = timer_timeout;
	hrtimer_start(&hrt,kt,HRTIMER_MODE_REL);
    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    hrtimer_cancel(&hrt);
    pr_info("Goodbye hrtimer\n");
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
