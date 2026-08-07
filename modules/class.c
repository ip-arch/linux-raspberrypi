// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
//#include <linux/device.h>
//#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

struct gpio_desc *gpio_led;
static int advance = 100;

struct timer_list timer_x;
int led = 0;

void timer_timeout(struct timer_list *tm)
{
	led = 1 - led;
	gpiod_set_value(gpio_led,led);
	tm->expires = jiffies + msecs_to_jiffies(advance);
	add_timer(tm);
}

static ssize_t callback(struct device *dev,struct device_attribute *attr, const char *buf, size_t count) {
	long value;
        pr_info("callback:%s\n",buf);
	if(kstrtol(buf,10,&value) < 0) return -EINVAL;
	advance = value;
	del_timer(&timer_x);
	add_timer(&timer_x);
	return count;
}


DEVICE_ATTR(period,0220,NULL,callback);

static struct class *sClass;
static struct device *sDevice;
static struct cdev c_dev;
static dev_t dev0;
static ssize_t led_read(struct file *f, char __user *buf, size_t
  len, loff_t *off)
{
printk("buf = %p",buf);
  copy_to_user(buf, (void*)&advance, (len>4)?4:len);
  del_timer(&timer_x);
  add_timer(&timer_x);
  return (len>4)?4:len;
}
static ssize_t led_write(struct file *f, const char __user *buf,
  size_t len, loff_t *off)
{
printk("buf = %p",buf);
  copy_from_user((void*)&advance, buf, (len>4)?4:len);
  del_timer(&timer_x);
  add_timer(&timer_x);
  return (len>4)?4:len;
}

static struct file_operations LED_fops =
{
  .owner = THIS_MODULE,
  .read = led_read,
  .write = led_write
};

static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

    gpio_led = devm_gpiod_get_index(dev, "ex_led", 0, GPIOD_OUT_LOW);
    if (!gpio_led) {
        pr_err("Failed to get LED gpio descriptor\n");
        return -EINVAL;
    }
    timer_setup(&timer_x, timer_timeout, 0);
    timer_x.expires = jiffies + msecs_to_jiffies(advance);
    add_timer(&timer_x);
    alloc_chrdev_region(&dev0, 0, 1, "LED");
    sClass = class_create(THIS_MODULE, "LED");
    sDevice = device_create(sClass,NULL,dev0,NULL,"LED");
    cdev_init(&c_dev, &LED_fops);
    cdev_add(&c_dev, dev0, 1);
    device_create_file(sDevice,&dev_attr_period);
    pr_info("Hello class\n");
    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    del_timer(&timer_x);
    cdev_del(&c_dev);
    device_remove_file(sDevice,&dev_attr_period);
    device_destroy(sClass,dev0);
    class_destroy(sClass);
    unregister_chrdev_region(dev0,1);
    pr_info("Goodbye class\n");
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

