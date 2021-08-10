#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct led_device {
	dev_t dev;
	int major, minor;
	struct cdev cdev;
	struct class *class;
	struct device *device;
	struct device_node *node;
};

static struct led_device led_dev;

static ssize_t led_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
	printk("led_write\n");
	return 0;
}

static int led_open(struct inode *inode, struct file *file)
{
	printk("led_open\n");

	return 0;
}

static struct file_operations led_ops = {
	.owner 	= THIS_MODULE,
	.open	= led_open,
	.write	= led_write,
};

static int led_probe(struct platform_device *dev)
{
	int ret = 0;
	struct resource *led_resource[1];

	printk("led device and driver have been matched\n");

	led_resource[0] = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if(!led_resource[0]){
		printk("platform_get_resource failed\n");
		return -ENXIO;
	}

	printk("resource start is %d\n", led_resource[0]->start);
	printk("resource end is %d\n", led_resource[0]->end);

	ret = alloc_chrdev_region(&led_dev.dev, 0, 1, "led_dev");
	if(ret < 0){
		printk("alloc_chrdev_region\n");
		goto err_alloc_chrdev;
	}

	led_dev.major = MAJOR(led_dev.dev);
	led_dev.minor = MINOR(led_dev.dev);

	cdev_init(&led_dev.cdev, &led_ops);
	ret = cdev_add(&led_dev.cdev, led_dev.dev, 1);
	if(ret < 0){
		printk("cdev_add failed\n");
		goto err_cdev_add;
	}

	led_dev.class = class_create(THIS_MODULE, "led_dev");
	if(IS_ERR(led_dev.class)){
		printk("class_create failed\n");
		goto err_class_create;
	}

	led_dev.device = device_create(led_dev.class, NULL, led_dev.dev, NULL,"led_dev");
	if(IS_ERR(led_dev.device)){
		printk("device_create failed\n");
		goto err_device_create;
	}

	return 0;

err_device_create:
	class_destroy(led_dev.class);

err_class_create:
	cdev_del(&led_dev.cdev);

err_cdev_add:
	unregister_chrdev_region(led_dev.dev, 1);

err_alloc_chrdev:
	return ret;
}

static int led_remove(struct platform_device *dev)
{
	device_destroy(led_dev.class, led_dev.dev);
	class_destroy(led_dev.class);
	cdev_del(&led_dev.cdev);
	unregister_chrdev_region(led_dev.dev, 1);

	return 0;
}

static struct platform_driver led_driver = {
	.probe 	= led_probe,
	.remove	= led_remove,
	.driver	= {
		.name = "imx6d-led"
	}
};

static int __init led_driver_init(void)
{
	return platform_driver_register(&led_driver);
}

static void __exit led_driver_exit(void)
{
	platform_driver_unregister(&led_driver);
}


module_init(led_driver_init);
module_exit(led_driver_exit);

MODULE_AUTHOR("david smith");
MODULE_DESCRIPTION("a symplify program to learn device tree");
MODULE_LICENSE("GPL");


