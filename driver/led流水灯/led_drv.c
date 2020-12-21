#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>		//cdev
#include <linux/fs.h>		//file_oprations
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/uaccess.h>

static int 		led_open (struct inode *inode, struct file *file);
static int 		led_close (struct inode *inode, struct file *file);
static ssize_t 	led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);
static ssize_t 	led_read(struct file *file, char __user *buf, size_t len, loff_t *offset);

static struct 			cdev led_cdev;
static struct device 	*led_device;
static struct class		*led_class;
static dev_t 			led_dev_num = 0;
static struct resource *led_res;

static struct file_operations led_ops = {
	.owner 		= THIS_MODULE,
	.open 		= led_open,
	.release 	= led_close,
	.write		= led_write,
	.read 		= led_read,
};

void __iomem *gpioe_out 	= NULL;
#define		  GPIOE_OUTENB_VA 	 (gpioe_out + 0x4)
#define		  GPIOE_ALFNO0_VA 	 (gpioe_out + 0x20)

void __iomem *gpioc_out = NULL;
#define		  GPIOC_OUTENB_VA 	 (gpioc_out + 0x4)
#define		  GPIOC_ALFNO0_VA 	 (gpioc_out + 0x20)
#define		  GPIOC_ALFNO1_VA 	 (gpioc_out + 0x24)

void __iomem *gpioc_outenb = NULL;
void __iomem *gpioc_alfn0 = NULL;
void __iomem *gpioc_alfn1 = NULL;

#define GPIO_BASE			0xC0010000

#define GPIOE_OUT 			(GPIO_BASE + 0xE000)
#define GPIOE_OUTENB 	    (GPIOE_OUT + 4)
#define GPIOE_ALFNO0 	    (GPIOE_OUT + 20)

#define GPIOC_OUT 			(GPIO_BASE + 0xC000)
#define GPIOC_OUTENB 	    *(volatile unsigned int*)0xC001C004
#define GPIOC_ALFNO1 	    *(volatile unsigned int*)0xC001C024

static int led_open (struct inode *inode, struct file *file)
{
	//配置GPIO为
	*(volatile unsigned int*)GPIOE_ALFNO0_VA &= ~(0x3<<26);
	*(volatile unsigned int*)GPIOE_OUTENB_VA |= 0x1<<13;

	*(volatile unsigned int*)GPIOC_ALFNO0_VA &= ~(0x3<<14 |0x3<<16);
	*(volatile unsigned int*)GPIOC_ALFNO0_VA |= (0x1<<14 |0x1<<16);

	//配置GPIOB17为GPIO模式
	*(volatile unsigned int*)GPIOC_ALFNO1_VA &= ~(0x3<<2);
	*(volatile unsigned int*)GPIOC_ALFNO1_VA |= (0x1<<2);

	*(volatile unsigned int*)GPIOC_OUTENB_VA |= (0x1<<7);
	*(volatile unsigned int*)GPIOC_OUTENB_VA |= (0x1<<8);
	*(volatile unsigned int*)GPIOC_OUTENB_VA |= (0x1<<17);

	*(volatile unsigned int*)gpioe_out |= (0x1<<13);
	*(volatile unsigned int*)gpioc_out |= (0x1<<7);
	*(volatile unsigned int*)gpioc_out |= (0x1<<8);
	*(volatile unsigned int*)gpioc_out |= (0x1<<17);
	printk("<4>""led open\n");

	return 0;
}


static int led_close (struct inode *inode, struct file *file)
{
	*(volatile unsigned int*)gpioe_out |= (0x1<<13);
	*(volatile unsigned int*)gpioc_out |= (0x1<<7);
	*(volatile unsigned int*)gpioc_out |= (0x1<<8);
	*(volatile unsigned int*)gpioc_out |= (0x1<<17);

	printk("<4>""led close\n");
	
	return 0;
}

/*function:写入函数

*/
ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	char kbuf[64] = {0};
	int ret = 0;

	if(NULL == buf)
		return EINVAL;

	if(len > 64)
		return EINVAL;

	ret = copy_from_user(kbuf, buf, len);
	len = len - ret;

	if(kbuf[1]){		//点亮led灯
		switch(kbuf[0])
		{
		case 7:
			*(volatile unsigned int*)gpioe_out &= ~(0x1<<13);
			break;
			
		case 8:
			*(volatile unsigned int*)gpioc_out &= ~(0x1<<17);
			break;
		
		case 9:
			*(volatile unsigned int*)gpioc_out &= ~(0x1<<8);
			break;
		
		case 10:
			*(volatile unsigned int*)gpioc_out &= ~(0x1<<7);
			break;
		default:
			printk("no such option\n");
		}
	} else{
		switch(kbuf[0])
		{
		case 7:
			*(volatile unsigned int*)gpioe_out |= (0x1<<13);
			break;
		case 8:
			*(volatile unsigned int*)gpioc_out |= (0x1<<17);
			break;
		
		case 9:
			*(volatile unsigned int*)gpioc_out |= (0x1<<8);
			break;
		
		case 10:
			*(volatile unsigned int*)gpioc_out |= (0x1<<7);
			break;
		default:
			printk("no such option: %d\n", kbuf[0]);
		}
	}
	
	printk("led:%d\n", kbuf[0]);
	//printk(KERN_ALERT"kbuf: %s", kbuf);

	return len;
}


ssize_t led_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	char kbuf[] = "12345";
	int ret = 0;

	if(NULL == buf)
		return EINVAL;

	if(len > 5)
		return EINVAL;

	ret = copy_to_user(buf, kbuf, len);


	return len - ret;
}


static int __init led_init(void)		//模块加载函数（一般需要）
{
	int ret = 0;
	int major;
	int minor;  
    printk(KERN_ALERT"hello, world\n");

#if 0
    //静态注册设备号
    led_dev_num = MKDEV(239, 0);
    //若设备号有效，则可以在/proc/device找到“myled”
    ret = register_chrdev_region(led_dev_num, 1, "myled");
 	if(ret < 0){
 		printk("register_chrdev_region failed\n");
 		goto err_register_chrdev_region;
 	}
#endif
    /*动态注册设备号
		@led_dev_num：传出参数，系统分配的设备号
		@baseminor	：次设备的开始值
		@count		：申请次设备的数量
		@name		：若设备号有效，则可以在/proc/device找到“myled”
	*/
    ret = alloc_chrdev_region(&led_dev_num, 0, 1, "myled");
	if(ret < 0){
 		printk("register_chrdev_region failed\n");
 		goto err_alloc_chrdev_region;
 	}

 	major = MAJOR(led_dev_num);
	minor = MINOR(led_dev_num);

 	printk("major is %d\n", major);
	printk("minor is %d\n", minor);

 	//初始化字符设备
 	cdev_init(&led_cdev, &led_ops);

 	//将cdev加入到内核
 	ret = cdev_add(&led_cdev, led_dev_num, 1);
 	if(ret < 0){
 		printk("cdev_add failed\n");
 		goto err_cdev_add;
 	}

 	//申请物理内存
 	led_res = request_mem_region(GPIOE_OUT, 0x24, "gpioe");
	if(NULL == led_res){
		ret = -ENOMEM;
		printk("<4>""request_mem_region fail\n");
		goto err_request_mem_region;
	}

	//将物理内存映射到虚拟内存
	gpioe_out = ioremap(GPIOE_OUT, 0x24);
	if(NULL == gpioe_out){
		ret = -ENOMEM;
		printk("<4>""ioremap fail\n");
		goto err_ioremap;
	}

	led_res = request_mem_region(GPIOC_OUT, 0x28, "gpioc");
	if(NULL == led_res){
		ret = -ENOMEM;
		printk("<4>""request_mem_region fail\n");
		goto err_gpioc_request_mem_region;
	}

	//将物理内存映射到虚拟内存
	gpioc_out = ioremap(GPIOC_OUT, 0x28);
	if(NULL == gpioc_out){
		ret = ENOMEM;
		printk("<4>""ioremap fail\n");
		goto err_gpioc_ioremap;
	}

	//创建一个类，成功会在/sys/class/路径下创建“myled”文件夹
	led_class = class_create(THIS_MODULE, "myled");
	if(IS_ERR(led_class)){
		ret = PTR_ERR(led_class);
		goto err_class_create;
	}

	//应在cdev_add之后再进行创建设备，否则将会引起内核报错
	//且只能用于字符设备创建
	//调用成功将会在/dev路径下创建myled设备节点
	led_device = device_create(led_class, NULL, led_dev_num, NULL, "myled");
	if(IS_ERR(led_device)){
		ret = PTR_ERR(led_device);
		goto err_device_create;
	}

	//GPIOE_OUTENB_VA = gpioe_out+4;
	//GPIOE_OUTENB_VA  = gpioe_out+20;

	// led_res = request_mem_region(0xC001C000, 0x28, "led");
	// if(NULL == led_res){
	// 	ret = ENOMEM;
	// 	goto err_request_mem_region;
	// }

	// gpioc_out = ioremap(0xC001c000, 0x24);
	// if(NULL == gpioc_out){
	// 	ret = ENOMEM;
	// 	goto err_ioremap;
	// }

	// gpioc_outenb = gpioe_out+4;
	// gpioc_alfn1  = gpioe_out+24;

    return 0;

err_device_create:
	//类的销毁
	class_destroy(led_class);

err_class_create:
	iounmap(gpioc_out);

err_gpioc_ioremap:
	release_mem_region(GPIOC_OUT, 0x24);

err_gpioc_request_mem_region:
	//取消虚拟内存映射
	iounmap(gpioe_out);

err_ioremap:
	//释放IO内存
	release_mem_region(GPIOE_OUT, 0x24);

err_request_mem_region:
	//删除字符设备
	cdev_del(&led_cdev);

err_cdev_add:
	//注销设备号
	unregister_chrdev_region(led_dev_num, 1);

err_alloc_chrdev_region:
//err_register_chrdev_region:
	return ret;
}


static void __exit led_exit(void)	//模块卸载函数（一般需要）	
{
	//设备文件的销毁
	device_destroy(led_class, led_dev_num);
	//销毁类
	class_destroy(led_class);
	release_mem_region(GPIOC_OUT, 0x28);
	iounmap(gpioc_out);
	release_mem_region(GPIOE_OUT, 0x24);
	iounmap(gpioe_out);
	//删除字符设备
	cdev_del(&led_cdev);
	//注销设备号
	unregister_chrdev_region(led_dev_num, 1);

    printk(KERN_ALERT"Goodbye, cruel world\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
