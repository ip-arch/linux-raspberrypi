// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/sysctl.h>

#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

int ledlevel = 1;
static int irq;
int count = 0;
struct gpio_desc *gpio_led, *gpio_sw;

static int do_ledlevel(struct ctl_table *ctl, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos) {
	int ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if(write) {
		gpiod_set_value(gpio_led,ledlevel);
	}
	return ret;
}

static struct ctl_table proctest_table[] = {
   {
	   .procname="ledlevel", 
	   .data=&ledlevel, 
	   .maxlen=sizeof(int), 
	   .mode=S_IRUGO|S_IWUSR, 
	   .proc_handler=&do_ledlevel,},
   {
	   .procname="count", 
	   .data=&count, 
	   .maxlen=sizeof(int), 
	   .mode=S_IRUGO|S_IWUSR, 
	   .proc_handler=&proc_dointvec,},
   {}
};

static struct ctl_table sys_table[] = {
   {
	   .procname="hgpio_sysctl",
	   .maxlen=0, 
	   .mode=S_IXUGO|S_IRUGO, 
	   .child=proctest_table,},
   {}
};

struct ctl_table_header *s;

static irqreturn_t irq_sw(int irq, void *dev_id, struct pt_regs *regs) {
	ledlevel = 1 - ledlevel;
	gpiod_set_value(gpio_led,ledlevel);
	count++;
	return IRQ_HANDLED;
}

static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

    s=register_sysctl_table(sys_table);
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
    unregister_sysctl_table(s);
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
