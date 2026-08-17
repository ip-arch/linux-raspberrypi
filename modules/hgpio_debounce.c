// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

#define DEBOUNCE_DUR	50

static int ledlevel = 0;
static int irq;
static long count = 0;
static struct gpio_desc *gpio_led, *gpio_sw;

static int prev_sw = 0;

static void toggle_led(void)
{
	ledlevel = 1 - ledlevel;
	gpiod_set_value(gpio_led,ledlevel);
	count++;
	pr_info("set LED to =%d\n", ledlevel);
}

static irqreturn_t irq_sw_thread(int irq, void *dev_id)
{
	int cur_sw;
	cur_sw = gpiod_get_value_cansleep(gpio_sw);
	pr_info("sw=%d, prev=%d@%lu\n", cur_sw, prev_sw,jiffies);
	if( cur_sw == 1 && !prev_sw) {
		prev_sw = 1;
		toggle_led();
	}
	do {
		/*
		 * If the switch is held down for a long time, this IRQ thread
		 * continues running.
		 *
		 * A practical driver could separate the debounce/release handling
		 * using a workqueue or timer, but that requires additional state
		 * management to handle subsequent IRQs correctly.
		 */
		msleep(DEBOUNCE_DUR);
		cur_sw = gpiod_get_value_cansleep(gpio_sw);
		pr_info("sw=%d, prev=%d@%lu\n", cur_sw, prev_sw,jiffies);
		if( cur_sw == 1 && !prev_sw) {
			prev_sw = 1;
			toggle_led();
		}
	} while(cur_sw);
	prev_sw = cur_sw;
	return IRQ_HANDLED;
}


static int my_init(struct platform_device *pdev)
{
    int ret;
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

/* 初期化時 */
	irq = gpiod_to_irq(gpio_sw);
	if (irq < 0) {
		pr_err("Failed to get IRQ: %d\n", irq);
		return irq;
	}
	if(ret=request_threaded_irq(irq,
		NULL,
		irq_sw_thread,
		IRQF_ONESHOT|IRQF_TRIGGER_LOW,
		"GPIO SW INT",
		pdev));
       if(ret)	{
	       pr_err("request_irq failed %d\n", ret);
	       return ret;
       }
	pr_info("Hello GPIO(IRQ=%d) SW1=%d\n",irq, gpiod_get_value(gpio_sw)); 
	return 0;
}


static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye hGPIO toggle count=%ld\n",count);
    free_irq(irq, pdev);
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
