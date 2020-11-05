#include <linux/init.h>
#include <linux/module.h>


static int hello_init(void)		//模块加载函数（一般需要）
{
    printk(KERN_ALERT"hello, world\n");
    
    return 0;
}


static void hello_exit(void)	//模块卸载函数（一般需要）	
{
    printk(KERN_ALERT"Goodbye, cruel world\n");
}

module_init(hello_init);		//加载模块
module_exit(hello_exit);		//卸载模块

MODULE_AUTHOR("David");			//模块作者
MODULE_LICENSE("GPL"); 			//模块许可证声明（必须）
MODULE_DESCRIPTION("A simple Hello World Module");	//模块描述
MODULE_ALIAS("a simplest module");					//模块别名
