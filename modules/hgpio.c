// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int ledlevel = 1;
static int irq;
long count = 0;
struct gpio_desc *gpio_led, *gpio_sw;

static irqreturn_t irq_sw(int irq, void *dev_id) {
	ledlevel = 1 - ledlevel;
	gpiod_set_value(gpio_led,ledlevel);
	count++;
	return IRQ_HANDLED;
}

static int my_init(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    gpio_led = devm_gpiod_get_index(dev, "ex_led", 0, GPIOD_OUT_LOW);
    if (IS_ERR(gpio_led)) {
        pr_err("Failed to get LED gpio descriptor\n");
        return PTR_ERR(gpio_led);
    }

    gpio_sw = devm_gpiod_get_index(dev, "ex_sw", 0, GPIOD_IN);
    if (IS_ERR(gpio_sw)) {
        pr_err("Failed to get SW gpio descriptor\n");
        return PTR_ERR(gpio_sw);
    }


	irq = gpiod_to_irq(gpio_sw);
	if(request_irq(irq,
		irq_sw,
		IRQF_TRIGGER_FALLING,
		"GPIO SW INT",
		NULL)) printk("request_irq failed");
	pr_info("Hello GPIO(IRQ=%d) SW1=%d\n",irq, gpiod_get_value(gpio_sw)); 
	return 0;
}


static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye hGPIO interrupt count=%ld\n",count);
    free_irq(irq, NULL);
    return;
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
