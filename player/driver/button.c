#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/io.h>
//#define DEBUG
#ifdef DEBUG
#define DPRINTK printk
#else
#define DPRINTK( x... )
#endif
#define rGPGCON 0x56000060
#define rGPFCON 0x56000050
static int major;
static struct class *button_class;
static struct class_device *button_class_dev;
static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;
static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;
static struct timer_list button_timer;
static atomic_t canopen = ATOMIC_INIT(1);//定义原子变量并初始化为1
static struct pin_desc *irq_pd;
static struct timer_list button_timer;
static u32 start_jiffies=0;
static u32 end_jiffies=0;
static u8 button_count=0;
struct pin_desc{
	unsigned int key;
	unsigned int key_val;
	unsigned int irq_num;
	char *dev_name;
};
struct key_time{
	unsigned char key;
	u32 time;
};
static struct key_time keyval_time[1];
static struct pin_desc pins_desc[8]={
	{S3C2410_GPF0,0x01,IRQ_EINT0,"K1"},
	{S3C2410_GPF1,0x02,IRQ_EINT1,"K2"},
	{S3C2410_GPF2,0x03,IRQ_EINT2,"K3"},
	{S3C2410_GPF3,0x04,IRQ_EINT3,"K4"},
	{S3C2410_GPF4,0x05,IRQ_EINT4,"K5"},
	{S3C2410_GPF5,0x06,IRQ_EINT5,"K6"},
	{S3C2410_GPF6,0x07,IRQ_EINT6,"K7"},
	{S3C2410_GPG0,0x08,IRQ_EINT8,"K8"},
};
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
// 中断事件标志, 中断服务程序将它置1，button_drv_read将它清0
static volatile unsigned char ev_press = 0;
static irqreturn_t button_handle(int irq,void *dev_id){
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned char pinval;
	pinval = s3c2410_gpio_getpin(pindesc->key);
	irq_pd = (struct pin_desc*)dev_id;
	if (pinval){//up
		mod_timer(&button_timer,jiffies);//修改定时器超时时间
	}else{ //down
		if(button_count == 0)            //防止抖动
			start_jiffies = jiffies;
		button_count++;
	}
	return IRQ_HANDLED;
}
static int button_open(struct inode *inode, struct file *file){
	int i,err;
	if(!atomic_dec_and_test(&canopen)){		
		atomic_inc(&canopen);//原子变量增加1		return -EBUSY;	
	}
	for(i=0;i<8;i++){  //chip->set_type会把引脚设置成中断引脚
		err = request_irq(pins_desc[i].irq_num, button_handle, IRQT_BOTHEDGE, pins_desc[i].dev_name,&pins_desc[i]);
        if (err)            
			break;
	}
	if(err){
		i--;
		for (; i >= 0; i--)
			 free_irq(pins_desc[i].irq_num, (void *)&pins_desc[i]);
        return -EBUSY;
	}
	DPRINTK("button_drv_open\n");
	return 0;
}

static int button_read(struct file *file, char __user *buf,size_t len,loff_t *ppos){
	int ret;
	ev_press=0;
	wait_event_interruptible(button_waitq,ev_press);//如果没有按键动作发生，就休眠(让出CPU资源)		
	ret=copy_to_user(buf, &keyval_time, sizeof(keyval_time));
	keyval_time[0].key=0;
	keyval_time[0].time=0;
	DPRINTK("button_drv_read\n");
	return 1;
}
static int button_close(struct inode * inode, struct file * file){
	int i;
	atomic_inc(&canopen);
	for(i=0;i<8;i++){//释放中断
		free_irq(pins_desc[i].irq_num, (void *)&pins_desc[i]);
	}
	return 0;
}
static struct file_operations button_fops={
	.owner   =   THIS_MODULE,   
	.open    =   button_open,
	.read    =   button_read,
	.release =   button_close,
};
static void button_timer_function(unsigned long data){
	//u32 diff;
	struct pin_desc *pindesc = (struct pin_desc *)irq_pd;
	if(!pindesc)
		return ;
	end_jiffies = jiffies;
	keyval_time[0].key = pindesc->key_val;//读取键值
	keyval_time[0].time = end_jiffies - start_jiffies;
	end_jiffies = 0;
	start_jiffies = 0; 
	button_count=0;
	ev_press = 1;//表示中断发生了
	wake_up_interruptible(&button_waitq);//唤醒休眠的进程
}
static int button_init(void)
{
	init_timer(&button_timer);
	button_timer.function = button_timer_function;
	add_timer(&button_timer);//把定时器告诉内核
	major = register_chrdev(0, "button_drv", &button_fops);
    if (major < 0) {
      printk(" can't register major number\n");
      return major ;
    }
	button_class = class_create(THIS_MODULE, "player_button");
	button_class_dev = class_device_create(button_class, NULL, MKDEV(major, 0), NULL, "btn");//   /dev/btn
	gpfcon = (volatile unsigned long *)ioremap(rGPFCON, 16);
	gpfdat = gpfcon+1;
	gpgcon = (volatile unsigned long *)ioremap(rGPGCON, 16);
	gpgdat = gpgcon+1;
	DPRINTK("initialized\n");
	return 0;
}
static void button_exit(void){
	del_timer(&button_timer);
	unregister_chrdev(major, "button_drv");
	class_device_unregister(button_class_dev);
	class_destroy(button_class);
	iounmap(gpgcon);
	iounmap(gpfcon);
	DPRINTK("button_drv_exit\n");
}
module_init(button_init);
module_exit(button_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TaoJiaJun");
