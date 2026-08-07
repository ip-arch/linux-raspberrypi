// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

struct gpio_descs *gpio_led, *gpio_sw, *gpio_disp, *gpio_seg;
static unsigned long disp=3, seg=0x5a; 

static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

    pr_info("My_init start\n");
    gpio_led = devm_gpiod_get_array(dev, "ex_led", GPIOD_OUT_LOW);
    if (!gpio_led) {
        pr_err("Failed to get LED gpio descriptor\n");
        return -EINVAL;
    }

    gpio_sw = devm_gpiod_get_array(dev, "ex_sw", GPIOD_IN);
    if (!gpio_sw) {
        pr_err("Failed to get SW gpio descriptor\n");
        return -EINVAL;
    }

    gpio_disp = devm_gpiod_get_array(dev, "ex_disp", GPIOD_OUT_LOW);
    if (!gpio_disp) {
        pr_err("Failed to get DISP gpio descriptor\n");
        return -EINVAL;
    }

    gpio_seg = devm_gpiod_get_array(dev, "ex_seg", GPIOD_OUT_LOW);
    if (!gpio_seg) {
        pr_err("Failed to get SEG gpio descriptor\n");
        return -EINVAL;
    }

    pr_info("Hello GPIO array.\nLED:%d, SW:%d, DISP:%d, SEG:%d\n", gpio_led->ndescs, gpio_sw->ndescs, gpio_disp->ndescs, gpio_seg->ndescs);

    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye GPIO array\n");
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
