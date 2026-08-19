// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int ledlevel = 1, wqreq=0;
static int irq;
long count = 0;

static int advance = 50;
struct timer_list timer_x;

struct gpio_desc *gpio_led, *gpio_sw;

void timer_timeout(struct timer_list *tm)
{
	wqreq=0;
	if(gpiod_get_value(gpio_sw)==1) { 
		ledlevel = 1 - ledlevel;
		gpiod_set_value(gpio_led,ledlevel);
	}
}


static irqreturn_t irq_sw(int irq, void *dev_id) {
	if(!wqreq) {
		wqreq++;
		timer_x.expires = jiffies + msecs_to_jiffies(advance);
		add_timer(&timer_x);
	}
	count++;
	return IRQ_HANDLED;
}


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

    timer_setup(&timer_x, timer_timeout, 0);

	irq = gpiod_to_irq(gpio_sw);
	if(request_irq(irq,
		irq_sw,
		IRQF_SHARED|IRQF_TRIGGER_FALLING,
		"GPIO SW INT",
		THIS_MODULE->name)<0) printk("request_irq failed");
	pr_info("Hello hGPIO_wq(IRQ=%d) SW1=%d\n",irq, gpiod_get_value(gpio_sw)); 
	return 0;
}


static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye hGPIO_wq interrupt count=%d\n",count);
    del_timer(&timer_x);
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
