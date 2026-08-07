// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

static int ledlevel = 1;
struct gpio_desc *gpio_led, *gpio_sw;

static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

    gpio_led = devm_gpiod_get_index(dev, "ex_led", 0, GPIOD_OUT_LOW);
    if (!gpio_led) {
        pr_err("Failed to get LED gpio descriptor\n");
        return -EINVAL;
    }
    gpio_sw = devm_gpiod_get_index(dev, "ex_sw", 0, GPIOD_IN);
    if (!gpio_sw) {
        pr_err("Failed to get SW gpio descriptor\n");
        return -EINVAL;
    }
    gpiod_set_value(gpio_led, ledlevel);
    pr_info("Hello GPIO SW=%d\n", gpiod_get_value(gpio_sw));
    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye GPIO\n");
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

