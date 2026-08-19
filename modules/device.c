// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Naohiko Shimizu <nshimizu@ip-arch.jp>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naohiko Shimizu");

static int led = 0;
struct gpio_desc *gpio_led;

static ssize_t led_read(struct file *f, char __user *buf, size_t
  len, loff_t *off)
{
printk("buf = %px",buf);
  copy_to_user(buf, (void*)&led, (len>4)?4:len);
  return (len>4)?4:len;
}
static ssize_t led_write(struct file *f, const char __user *buf,
  size_t len, loff_t *off)
{
printk("buf = %px",buf);
  copy_from_user((void*)&led, buf, (len>4)?4:len);
  gpiod_set_value(gpio_led,led&1);
  return (len>4)?4:len;
}

static struct file_operations LED_fops =
{
  .owner = THIS_MODULE,
  .read = led_read,
  .write = led_write
};
static int maj;
static int my_init(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;

    gpio_led = devm_gpiod_get_index(dev, "ex_led", 0, GPIOD_OUT_LOW);
    if (!gpio_led) {
        pr_err("Failed to get LED gpio descriptor\n");
        return -EINVAL;
    }
    gpiod_set_value(gpio_led, led);
    maj = register_chrdev(0, "device", &LED_fops);
    if(maj < 0) {printk("Could not get major\n"); return -1;}
    pr_info("Hello Device\nmajor = %d\n", maj);
    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    pr_info("Goodbye Device\n");
    if(maj) unregister_chrdev(maj, "device");
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

