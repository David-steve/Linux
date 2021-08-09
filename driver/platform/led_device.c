#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/platform_device.h>

static struct resource led_resource[] = {
	[0] = {
		.start 	= 0x01, 
		.end	= 0x100,
		.flags 	= IORESOURCE_MEM,
	}
};

static void led_release(struct device *dev)
{
	printk("led release\n");

	return;
}

static struct platform_device led_device = {
	.name 	= "imx6d-led",
	.id		= -1,
	.dev	= {
		.release = led_release,
	},
	.num_resources = ARRAY_SIZE(led_resource),
	.resource = led_resource,

};

static int __init led_init(void)
{
	return platform_device_register(&led_device);
}

static void __exit led_exit(void)
{
	platform_device_register(&led_device);

}

module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("david smith");
MODULE_DESCRIPTION("a symplify program to learn device tree");
MODULE_LICENSE("GPL");


