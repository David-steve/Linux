#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>			//cdev
#include <mach/devices.h>		//中断号
#include <linux/interrupt.h>	//request_irq
#include <linux/ioctl.h>		//ioctl
#include <linux/sched.h>		//wait_queue

static struct cdev 		key_cdev;		// 字符设备
static struct device 	*key_device;
static struct class		*key_class;
static dev_t 			key_dev_num = 0;

static long key_ioctl(struct file *file, unsigned int cmd, unsigned long addr);
static int key_open(struct inode *inode, struct file *file);
int key_close(struct inode *inode, struct file *file);

/****************ioctl cmd define***********************************/
#define IOCTL_KEY_ALL	_IOR('K', 0, unsigned long)
//#define IOCTL_KEY_ALL				_IOW('L', 0, unsigned long)

static int irq_flag=0;		//等待条件
static wait_queue_head_t key_queue;	//等待队列
static int key_val=0;		//按下的按键

static struct file_operations key_ops = {
	.owner 			= THIS_MODULE,
	.open 			= key_open,
	.release 		= key_close,
	//.write			= key_write,
	//.read 			= key_read,
	.unlocked_ioctl = key_ioctl,
};

irqreturn_t key_handler(int irq_n, void *dat)
{
	if(irq_n == IRQ_GPIO_A_START+28){
		//printk("key_2 pressed\n");
		key_val |= 1<<2;
	}
	else if(irq_n == IRQ_GPIO_B_START+9)
		key_val |= 1<<6;

	irq_flag = 1;
	/* 有按键按下， 唤醒等待队列 */
	wake_up(&key_queue);

	//中断处理结束
	return IRQ_HANDLED;
}

static long key_ioctl(struct file *file, unsigned int cmd, unsigned long addr)
{
	int ret = 0;
	switch(cmd)
	{
		case IOCTL_KEY_ALL:{
			wait_event_interruptible(key_queue, irq_flag);
			irq_flag = 0;
			ret = copy_to_user((void __user*)addr, &key_val, sizeof key_val);
			if(ret)
				printk("copy_to_user failed\n");

			key_val = 0;
		}break;

		default:
			break;
	}

	return 0;
		
}


static int key_open(struct inode *inode, struct file *file)
{
	printk("key_drv open\n");

	return 0;
}


int key_close(struct inode *inode, struct file *file)
{
	printk("key_drv close\n");
	
	return 0;
}

//入口函数
static int __init key2_init(void)
{
	int ret = 0;
	int major;
	int minor;  
    printk(KERN_ALERT"hello, world\n");

#if 0
    //静态注册设备号
    key_dev_num = MKDEV(239, 0);
    //若设备号有效，则可以在/proc/device找到“mykey”
    ret = register_chrdev_region(key_dev_num, 1, "mykey");
 	if(ret < 0){
 		printk("register_chrdev_region failed\n");
 		goto err_register_chrdev_region;
 	}
#else
    /*	动态注册设备号
		@key_dev_num：传出参数，系统分配的设备号
		@baseminor	：次设备的起始值
		@count		：申请次设备的数量
		@name		：若设备号有效，则可以在/proc/device找到“mykey”
	*/
    ret = alloc_chrdev_region(&key_dev_num, 0, 1, "mykey");
	if(ret < 0){
 		printk("register_chrdev_region failed\n");
 		goto err_alloc_chrdev_region;
 	}

 	major = MAJOR(key_dev_num);
	minor = MINOR(key_dev_num);

 	printk("major is %d\n", major);
	printk("minor is %d\n", minor);

#endif
 	//初始化字符设备
 	cdev_init(&key_cdev, &key_ops);

 	//将cdev加入到内核
 	ret = cdev_add(&key_cdev, key_dev_num, 1);
 	if(ret < 0){
 		printk("cdev_add failed\n");
 		goto err_cdev_add;
 	}

	key_class = class_create(THIS_MODULE, "mykey");
	if(IS_ERR(key_class)){
		ret = PTR_ERR(key_class);
		goto err_class_create;
	}

	/* 应在cdev_add之后再进行创建设备，否则将会引起内核报错
	 * 且只能用于字符设备创建
	 * 调用成功将会在/dev路径下创建mykey设备节点 */
	key_device = device_create(key_class, NULL, key_dev_num, NULL, "mykey");
	if(IS_ERR(key_device)){
		ret = PTR_ERR(key_device);
		goto err_device_create;
	}

	/* request for key interrupt handle */
	/* request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
	    const char *name, void *dev) */
	ret = request_irq(IRQ_GPIO_A_START+28, key_handler, IRQF_TRIGGER_FALLING, "key_k2_irq", NULL);
	if(ret < 0){
		printk("irq_request failed\n");
		goto err_request_irq_k2;
	}

	ret = request_irq(IRQ_GPIO_B_START+9, key_handler, IRQF_TRIGGER_FALLING, "key_k6_irq", NULL);
	if(ret < 0){
		printk("irq_request failed\n");
		goto err_request_irq_k6;
	}

	/* 初始化等待队列*/
	init_waitqueue_head(&key_queue);

	return 0;

err_request_irq_k6:
	free_irq(IRQ_GPIO_A_START+28, NULL);

err_request_irq_k2:
	//设备文件的销毁
	device_destroy(key_class, key_dev_num);	

err_device_create:	
	class_destroy(key_class);

err_class_create:
	//删除字符设备
	cdev_del(&key_cdev);

err_cdev_add:
	//注销设备号
	unregister_chrdev_region(key_dev_num, 1);	

err_alloc_chrdev_region:
	return ret;
}

//出口函数
static void __exit key2_exit(void)
{
	printk("GoodBye Teacher.Wen\n");
	free_irq(IRQ_GPIO_A_START+28, NULL);
	//销毁设备
	device_destroy(key_class, key_dev_num);	
	class_destroy(key_class);
	//删除字符设备
	cdev_del(&key_cdev);
	//注销设备号
	unregister_chrdev_region(key_dev_num, 1);
}

//驱动程序的入口：安装驱动的时候，自动调用module_init，module_init继续调用led_init函数
module_init(key2_init);
//驱动程序的出口：卸载驱动的时候，自动调用module_exit，module_exit继续调用led_exit函数
module_exit(key2_exit);


MODULE_AUTHOR("David-smith");//作者描述
MODULE_DESCRIPTION("key Device Driver");//模块描述
MODULE_LICENSE("GPL");			//遵循GPL
