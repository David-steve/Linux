#include <linux/module.h>
#include <linux/init.h>
#include <mach/devices.h>
#include <linux/interrupt.h>    //irq
#include <linux/input.h>	    // input_dev
#include <linux/gpio.h>		    //gpio_get_value

static struct input_dev *key_dev;

static irqreturn_t key_handler(int irq_n, void *dat)
{
	if(irq_n == IRQ_GPIO_A_START+28){
        /* report event:KEY_ENTER */
		input_report_key(key_dev, KEY_ENTER, !gpio_get_value(PAD_GPIO_A+28));
        /* report ends*/
		input_sync(key_dev);
	}

	return IRQ_HANDLED;
}

//入口函数
static int __init key2_init(void)
{
	int ret=0;
	printk("Hello Teacher.Wen\n");

	/*request for key interrupt handle*/
	/*request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev)*/
	ret = request_irq(IRQ_GPIO_A_START+28, key_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "key2_irq", NULL);
	if(ret < 0){
		printk("irq_request failed\n");
		return -EBUSY;
	}

	/* allocate memory for key_dev */
	key_dev = input_allocate_device();
	if(!key_dev){
		printk("No enough memmory\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}

	key_dev->name = "input_mykeys";

	key_dev->id.bustype = 0x0000;
	key_dev->id.vendor	= 0x1688;
	key_dev->id.product	= 0x1000;
	key_dev->id.version	= 0x1001;

	/* input event is keyboard */
	set_bit(EV_KEY, key_dev->evbit);

	/* the keycodes which device supports */
	set_bit(KEY_ENTER, key_dev->keybit);

	/* register input device */
	ret = input_register_device(key_dev);
	if(ret){
		printk("input_register_device error\n");
		goto err_input_register_device;
	}

	return 0;

err_input_register_device:
	input_free_device(key_dev);
err_input_allocate_device:
	free_irq(IRQ_GPIO_A_START+28, NULL);

	return ret;	
}

//出口函数
static void __exit key2_exit(void)
{
	printk("GoodBye Teacher.Wen\n");
	input_unregister_device(key_dev);
	input_free_device(key_dev);
	free_irq(IRQ_GPIO_A_START+28, NULL);
}

//驱动程序的入口：安装驱动的时候，自动调用module_init，module_init继续调用led_init函数
module_init(key2_init);
//驱动程序的出口：卸载驱动的时候，自动调用module_exit，module_exit继续调用led_exit函数
module_exit(key2_exit);


MODULE_AUTHOR("David-smith");//作者描述
MODULE_DESCRIPTION("key Device Driver");//模块描述
MODULE_LICENSE("GPL");			//遵循GPL
