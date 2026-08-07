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
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int ledlevel = 1;
static int irq;
long count = 0;
static int flag=0;
struct gpio_desc *gpio_led, *gpio_sw;

struct hrtimer hrt;
ktime_t kt;

enum hrtimer_restart timer_timeout(struct hrtimer *timer)
{
	flag=0;
	if(gpiod_get_value(gpio_sw) == 0)  {
		ledlevel = 1 - ledlevel;
		gpiod_set_value(gpio_led,ledlevel);
	}
	return HRTIMER_NORESTART;
}

static irqreturn_t irq_sw(int irq, void *dev_id, struct pt_regs *regs) {
	if(!flag) {
		flag=1;
	        kt=ktime_set(0,20000000UL);
		hrtimer_start(&hrt,kt,HRTIMER_MODE_REL);
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
	hrtimer_init(&hrt, CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	hrt.function = timer_timeout;

    gpio_sw = devm_gpiod_get_index(dev, "ex_sw", 0, GPIOD_IN);
    if (!gpio_sw) {
        pr_err("Failed to get SW gpio descriptor\n");
        return -EINVAL;
    }


	irq = gpiod_to_irq(gpio_sw);
	if(request_irq(irq,
		(irq_handler_t) irq_sw,
		IRQF_SHARED|IRQF_TRIGGER_FALLING,
		"GPIO SW INT",
		THIS_MODULE->name)<0) printk("request_irq failed");
	pr_info("Hello GPIO(IRQ=%d) SW1=%d\n",irq, gpiod_get_value(gpio_sw)); 
	return 0;
}


static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye hGPIO interrupt count=%d\n",count);
    free_irq(irq, THIS_MODULE->name);
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
