#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/gpio/consumer.h>
#include "head.h"

#include <linux/of_irq.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

#include <linux/io.h>
#include <linux/mod_devicetable.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>

struct i2c_client *gclient;

#define CNAME "mycdev"
struct cdev *cdev;
int major = 0;
int minor = 0;
int number =0;
const int count = 1;
struct class *cls;
struct device *dev;
char kbuf[128] = {0};
struct gpio_desc *gd[3];
struct device_node *pnode;
struct device_node *cnode1,*cnode2;
char *name[] = {"led1","led2","led3"};
struct gpio_desc *gp[3];
char *name2[] = {"hmb","fun","motor"};



wait_queue_head_t wq;
int condition = 0;

int irqno;
int frequ=0;
struct resource * res;
struct spi_device *gspi;


irqreturn_t key_led_irq_handle(int irq, void * dev)
{
	int i;
	//1.修改灯的状态 ,修改number
	number = !number;
	for(i=0;i<3;i++)
		{
		gpiod_set_value(gd[i],number);
		}
	printk("enter  %d que \n",frequ++);


	return IRQ_HANDLED;
}

int i2c_read_serial_version(unsigned short reg)
{
	int ret;
	char r_buf[] = {(reg>>8)&0xff,(reg&0xff)};
	char val;
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr = gclient->addr,
			.flags =0,
			.len = 2,
			.buf = r_buf,
		},
		[1] = {
			.addr = gclient->addr,
			.flags =1,
			.len = 1,
			.buf = &val,
		},
	};

	ret = i2c_transfer(gclient->adapter,r_msg,ARRAY_SIZE(r_msg));
	if(ret != ARRAY_SIZE(r_msg)){
		printk("i2c transfer error\n");
		return -EAGAIN;
	}

	return val;
}
int i2c_read_tmp_hum(unsigned char reg)
{
	int ret;
	char r_buf[] = {reg};
	unsigned short val;
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr = gclient->addr,
			.flags =0,
			.len = 1,
			.buf = r_buf,
		},
		[1] = {
			.addr = gclient->addr,
			.flags =1,
			.len = 2,
			.buf = (__u8*)&val,
		},
	};

	ret = i2c_transfer(gclient->adapter,r_msg,ARRAY_SIZE(r_msg));
	if(ret != ARRAY_SIZE(r_msg)){
		printk("i2c transfer error\n");
		return -EAGAIN;
	}

	return val>>8 | val<<8;
}


int mycdev_open(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	
	return 0;
}

ssize_t mycdev_read(struct file *file,
	char __user * ubuf, size_t size, loff_t *offs)
{
	int ret;
	if(file->f_flags &O_NONBLOCK){
		//非阻塞
		return -EINVAL;
	}else{
		if(ret){
			printk("receive signal.....\n");
			return ret;
		}
	}

	
	return size;

}

ssize_t mycdev_write(struct file *file,
	const char __user *ubuf, size_t size, loff_t *offs)
{

	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return size;
}


int mycdev_close(struct inode *inode, struct file *file)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
long mycdev_ioctl(struct file *file,
	unsigned int cmd, unsigned long args)
{
	int ret,data;

	switch(cmd){
		case LED1_ON:
			gpiod_set_value(gd[0],1);
			gpiod_set_value(gd[1],1);
			gpiod_set_value(gd[2],1);
			break;
		case LED1_OFF:
			gpiod_set_value(gd[0],0);
			gpiod_set_value(gd[1],0);
			gpiod_set_value(gd[2],0);
			break;
		case HMB_ON:
			gpiod_set_value(gp[0],1);
			break;
		case FUN_ON:
			gpiod_set_value(gp[1],1);
			break;
		case PRE_MOTOR_ON:
			gpiod_set_value(gp[2],1);
			break;
		case HMB_OFF:
			gpiod_set_value(gp[0],0);
			break;
		case FUN_OFF:
			gpiod_set_value(gp[1],0);
			break;
		case PRE_MOTOR_OFF:
			gpiod_set_value(gp[2],0);
			break;
		case GET_TMP:
			data = i2c_read_tmp_hum(0xe3);
			if(data < 0){
				printk("i2c read tmp error\n");
				return -EINVAL;
			}
			data = data & 0xffff;
			ret = copy_to_user((void *)args,(void *)&data,sizeof(int));
			if(ret){
				printk("copy data to user error\n");
				return -EIO;
			}
			break;
		case GET_HUMM:
			data = i2c_read_tmp_hum(0xe5);
			if(data < 0){
				printk("i2c read tmp error\n");
				return -EINVAL;
			}
			data = data & 0xffff;
			ret = copy_to_user((void *)args,(void *)&data,sizeof(int));
			if(ret){
				printk("copy data to user error\n");
				return -EIO;
			}
			break;
		case SEG_WHICH:
			spi_write(gspi,&which[args],1);
			break;
		case SEG_DAT:
			spi_write(gspi,&code[args],1);
			break;
		case SEG_DAT1:
			spi_write(gspi,&code1[args],1);
			break;
		default: printk("ioctl error\n");break;
						
	}

	return 0;
}
int pdrv_probe(struct platform_device *pdev)
{
	int ret;

	//gd = gpiod_get_from_of_node(pdev->dev.of_node,"led1",0,GPIOD_OUT_LOW,NULL);
	//if(IS_ERR(gd)){
	//	printk("get gpio desc error\n");
	//	return PTR_ERR(gd);
	//}
	//printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	res = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if(res == NULL){
		printk("platform get resource error\n");
		return -EINVAL;
	}
	irqno = platform_get_irq(pdev,0);
	if(irqno <0){
		printk("platform get irq error\n");
		return irqno;
	}

	/*irqno = irq_of_parse_and_map(pdev->dev.of_node,0);
	if(irqno == 0){
		printk("get irq no erro\n");
		return -EAGAIN;
	}*/

	ret = request_irq(irqno,key_led_irq_handle,IRQF_TRIGGER_FALLING,CNAME,NULL);
	if(ret){
		printk("register irq error\n");
		return ret;
	}

	printk("addr = %#x,irqno = %d\n",res->start,irqno);
	printk("11111111111111\n");
	return 0;
}
int pdrv_remove(struct platform_device *pdev)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}


struct of_device_id ofmatch[] = {
	{.compatible = "hqyj,myplatform",},
	{}  //使用设备树匹配的时候一定要写一个{},表示结束
};

struct platform_driver pdrv = {
	.probe = pdrv_probe,
	.remove = pdrv_remove,
	.driver = {
		.name = "duangduangduang",
		.of_match_table = ofmatch,
	}
};


int si7006_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int ret;
	gclient = client;
	printk("si7006:%s:%s:%d\n",__FILE__,__func__,__LINE__);

	ret = i2c_read_serial_version(0xfcc9);
	printk("serial(0x06) = %#x\n",ret);

	ret = i2c_read_serial_version(0x84b8);
	printk("Version = %#x\n",ret);
	return 0;
}
int si7006_remove(struct i2c_client *client)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);

	return 0;
}
const struct of_device_id ofmatch2[] = {
	{.compatible = "hqyj,si7006",},
	{}
};
MODULE_DEVICE_TABLE(of,ofmatch2);

struct i2c_driver si7006 = {
	.probe = si7006_probe,
	.remove = si7006_remove,
	.driver = {
		.name = "test i2c",
		.of_match_table = ofmatch2,
	}
};



int	m74hc595_probe(struct spi_device *spi)
{
	u8 buf[2] = {0xf,0x0};
	printk("m74hc595:%s:%d\n",__func__,__LINE__);
	gspi = spi;
	spi_write(gspi,buf,ARRAY_SIZE(buf));

	return 0;
}

int m74hc595_remove(struct spi_device *spi)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
const struct of_device_id ofmatch3[] = {
	{.compatible = "m74hc595",},
	{}
};
MODULE_DEVICE_TABLE(of,ofmatch3);

struct spi_driver m74hc595 = {
	.probe = m74hc595_probe,
	.remove = m74hc595_remove,
	.driver = {
		.name = "test_spi",
		.of_match_table = ofmatch3,
	}
};


const struct file_operations fops = {
	.open = mycdev_open,
	.read = mycdev_read,
	.write = mycdev_write,
	.release = mycdev_close,
	.unlocked_ioctl = mycdev_ioctl,
};


static int __init mycdev_init(void)
{
	int ret,i;
	dev_t devno;
	//1.为cdev对象分配内存
	cdev = cdev_alloc();
	if(cdev == NULL){
		printk("cdev memory alloc error\n");
		ret = -ENOMEM;
		goto ERR1;
	}
	//2.cdev的初始化
	cdev_init(cdev,&fops);
	
	//3.设备号的申请
	if(major > 0){
		//静态
		ret = register_chrdev_region(MKDEV(major,minor),count,CNAME);
		if(ret){
			printk("static:request device number error\n");
			goto ERR2;
		}
	}else if (major == 0){
		//动态
		ret = alloc_chrdev_region(&devno,minor,count,CNAME);
		if(ret){
			printk("dynamic:request device number error\n");
			goto ERR2;
		}
		major = MAJOR(devno);
		minor = MINOR(devno);
	}
	
	//4.注册
	ret = cdev_add(cdev,MKDEV(major,minor),count);
	if(ret){
		printk("cdev register error\n");
		goto ERR3;
	}


	//5.自动创建设备节点
	cls = class_create(THIS_MODULE,CNAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret =  PTR_ERR(cls);
		goto ERR4;
	}

	for(i=0;i<count;i++){
		dev = device_create(cls,NULL,MKDEV(major,minor+i),NULL,"mycdev%d",i);
		if(IS_ERR(dev)){
			printk("device create error\n");
			ret = PTR_ERR(dev);
			goto ERR5;
		}
	}


	//2.解析设备树，申请要使用的gpio
		//获取到父节点
		pnode = of_find_node_by_path("/leds");
		if(pnode == NULL){
			printk("get parent node error\n");
			return -EINVAL;
		}
		//获取到子节点
		cnode2 = of_get_child_by_name(pnode,"extend-leds");
		if(cnode2 == NULL){
			printk("get child node 2 error\n");
			return -EINVAL;
		}
	
		for(i=0;i<3;i++){
			//从设备节点上直接解析得到gpio_desc
			gd[i] = gpiod_get_from_of_node(cnode2,name[i],0,
				GPIOD_OUT_LOW,NULL);
			if(IS_ERR(gd[i])){
				printk("gpio request error\n");
				return PTR_ERR(gd[i]);
			}
		}


		cnode1= of_find_node_by_path("/peri");
		if(cnode1== NULL){
			printk("get parent node error\n");
			return -EINVAL;
		}
		for(i=0;i<3;i++){
			//从设备节点上直接解析得到gpio_desc
			gp[i] = gpiod_get_from_of_node(cnode1,name2[i],0,
				GPIOD_OUT_LOW,NULL);
			if(IS_ERR(gp[i])){
				printk("gpio request error\n");
				return PTR_ERR(gd[i]);
			}
		}
		i2c_add_driver(&si7006);
		spi_register_driver(&m74hc595);
	return platform_driver_register(&pdrv);

 //!!!!!!!!!!!!!!!!!!!!!!!不能忘记!!!!!!!!!!!!!!!!!!!!!!!
	
ERR5:
	for(--i;i>=0;i--){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);
ERR4:
	cdev_del(cdev);
ERR3:
	unregister_chrdev_region(MKDEV(major,minor),count);
ERR2:
	kfree(cdev);
ERR1:
	return ret;
}

static void __exit mycdev_exit(void)
{
	int i;
	for(i=0;i<count;i++){
		device_destroy(cls,MKDEV(major,minor+i));
	}
	class_destroy(cls);
	i2c_del_driver(&si7006);
	spi_unregister_driver(&m74hc595);
	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(major,minor),count);
	for(i=0;i<3;i++){
		gpiod_set_value(gd[i],0);
		gpiod_put(gd[i]);
		gpiod_set_value(gp[i],0);
		gpiod_put(gp[i]);
	}
	//销毁字符设备驱动
	free_irq(irqno,NULL);
	platform_driver_unregister(&pdrv);
	kfree(cdev);
}
module_init(mycdev_init);
module_exit(mycdev_exit);
MODULE_LICENSE("GPL");

