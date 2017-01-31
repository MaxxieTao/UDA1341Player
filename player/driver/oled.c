#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/irq.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/hrtimer.h>
#define rGPECON 0x56000040
#define rGPGCON 0x56000060
#define rGPHCON 0x56000070
#define rGPJCON 0x560000D0
#define BIG_CHAR      0
#define TINY_CHAR 	  1
#define GRAPHIC   	  2
#define CLEAN_N       3
#define CLEAN_ALL     4
#define SET_MOSI(x)  	((x==0) ? (*gpjdat &= ~(1<<0)) : (*gpjdat |= (1<<0)))
#define SET_SCLK(x)  	((x==0) ? (*gpjdat &= ~(1<<1)) : (*gpjdat |= (1<<1)))
#define SET_LEDCS(x)	((x==0) ? (*gpjdat &= ~(1<<2)) : (*gpjdat |= (1<<2)))
#define SET_DC(x)		((x==0) ? (*gpjdat &= ~(1<<3)) : (*gpjdat |= (1<<3)))
#define SET_ROMCS(x)	((x==0) ? (*gpjdat &= ~(1<<4)) : (*gpjdat |= (1<<4)))
//#define SET_FSO(x)		((x==0) ? (*gpjdat &= ~(1<<5)) : (*gpjdat |= (1<<5)))
#define GET_FSO			(*gpjdat & 0x20)
typedef struct oled_type_data{
	u8 page; //第几页
	u8 col;  //第几列
	u8 type; //类型
	u8 text[32];
}oled_val_t;
static volatile unsigned long *gpjcon = NULL;
static volatile unsigned long *gpjdat = NULL;
static atomic_t canopen = ATOMIC_INIT(1);//定义原子变量并初始化为1
static struct class *leds_class;
static struct class_device  *leds_class_dev;
static u32  fontaddr=0;
static int major=0;
static oled_val_t oled_val[1];

static void send_cmd_to_led(int data)   //写指令到LED
{
	u8 i;
	SET_DC(0);
	SET_LEDCS(0);
	for(i=0;i<8;i++){
		SET_SCLK(0);       //低电平开启传输
		udelay(1);
		if(data & 0x80)   //取最高位
			SET_MOSI(1);
		else 
			SET_MOSI(0);
		udelay(1);
		SET_SCLK(1);        //高电平时完成传输
		udelay(1);
		data<<=1;
	}
	SET_LEDCS(1);
	SET_DC(1);
}

static void send_data_to_led(int data)  //写数据到LED
{
	char i;
	SET_DC(1);
	SET_LEDCS(0);
	for(i=0;i<8;i++){
		SET_SCLK(0);      //低电平开启传输
		udelay(1);
		if(data & 0x80) 
			SET_MOSI(1);
		else 
			SET_MOSI(0);
		udelay(1);
		SET_SCLK(1);      //高电平时完成传输
		udelay(1);
		data<<=1;
	}
	SET_LEDCS(1);
	SET_DC(1);	
}
static void init_led(void)
{
	SET_LEDCS(0);
	SET_ROMCS(1);
//	lcd_reset=0;        /*低电平复位*/
	udelay(20);    
	//lcd_reset=1;		    /*复位完毕*/
	udelay(20);       
	send_cmd_to_led(0xAE);   //设置显示关闭
	send_cmd_to_led(0x20);	//Set Memory Addressing Mode	
	send_cmd_to_led(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	send_cmd_to_led(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	send_cmd_to_led(0xc8);	//Set COM Output Scan Direction
	send_cmd_to_led(0x00);//---set low column address
	send_cmd_to_led(0x10);//---set high column address
	send_cmd_to_led(0x40);//--set start line address
	send_cmd_to_led(0x81);//--set contrast control register
	send_cmd_to_led(0x7f);
	send_cmd_to_led(0xa1);//--set segment re-map 0 to 127
	send_cmd_to_led(0xa6);//--set normal display
	send_cmd_to_led(0xa8);//--set multiplex ratio(1 to 64)
	send_cmd_to_led(0x3F);//
	send_cmd_to_led(0xa4);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	send_cmd_to_led(0xd3);//-set display offset
	send_cmd_to_led(0x00);//-not offset
	send_cmd_to_led(0xd5);//--set display clock divide ratio/oscillator frequency
	send_cmd_to_led(0xf0);//--set divide ratio
	send_cmd_to_led(0xd9);//--set pre-charge period
	send_cmd_to_led(0x22); //
	send_cmd_to_led(0xda);//--set com pins hardware configuration
	send_cmd_to_led(0x12);
	send_cmd_to_led(0xdb);//--set vcomh
	send_cmd_to_led(0x20);//0x20,0.77xVcc
	send_cmd_to_led(0x8d);//--set DC-DC enable
	send_cmd_to_led(0x14);//
	send_cmd_to_led(0xaf);//--turn on oled panel 
	SET_LEDCS(1);
}
static void led_address(u8 page,u8 column)
{

	send_cmd_to_led(0xb0 + column);  
	send_cmd_to_led(((page & 0xf0) >> 4) | 0x10);	
	send_cmd_to_led((page & 0x0f) | 0x00);	
}
static void clear_n_screen(u8 line)//清某一行
{
	u8 j;
	SET_LEDCS(0);
	SET_ROMCS(1);
	send_cmd_to_led(0xb0+line);//0xb3+line
	send_cmd_to_led(0x00);
	send_cmd_to_led(0x10);
	for(j=0;j<128;j++)
	{
	  	send_data_to_led(0x00);
	}
 	SET_LEDCS(1);
}
static void clear_all_screen(void)//清屏
{
	u8 i,j;
	SET_LEDCS(0);
	SET_ROMCS(1);
	for(i=0;i<8;i++){
		send_cmd_to_led(0xb0+i);
		send_cmd_to_led(0x00);
		send_cmd_to_led(0x10);
		for(j=0;j<128;j++)
		{
		  	send_data_to_led(0x00);
		}
	}
 	SET_LEDCS(1);
}
/*static void display_128x64(u8 *dp)
{
	u8 i,j;
	SET_LEDCS(0);
	for(j=0;j<8;j++){
		led_address(0,j);
		for (i=0;i<128;i++)
		{	
			send_data_to_led(*dp);					//写数据到LCD,每写完一个8位的数据后列地址自动加1
			dp++;
		}
	}
	SET_LEDCS(1);
}*/
static void send_cmd_to_rom(u8 data){
	u8 i;
	for(i=0;i<8;i++){
		if(data & 0x80)
			SET_MOSI(1);
		else
			SET_MOSI(0);
		udelay(1);
		data <<= 1;
		SET_SCLK(0);
		udelay(1);
		SET_SCLK(1);
		udelay(1);
	}
}
static u8 get_data_from_rom(void){
	u8 i;
	u8 ret=0;
	SET_SCLK(1);
	udelay(1);
	for(i=0;i<8;i++){
		SET_SCLK(0);
		udelay(1);
		ret=ret<<1;
		if(GET_FSO)
			ret=ret+1;
		else
			ret=ret+0;
		udelay(1);
		SET_SCLK(1);
		udelay(1);
	}
	return ret;
}

static void get_n_bytes_data_from_rom(u8 addr_high,u8 addr_mid,u8 addr_low,u8 *fontbuf,u8 len )
{
	u8 i;
	SET_LEDCS(1);
	SET_ROMCS(0);
	SET_SCLK(0);
	send_cmd_to_rom(0x03);
	send_cmd_to_rom(addr_high);
	send_cmd_to_rom(addr_mid);
	send_cmd_to_rom(addr_low);
	for(i = 0; i < len; i++ )
	     *(fontbuf+i) =get_data_from_rom();
	SET_ROMCS(1);
}
/*显示16x16点阵图像、汉字、生僻字或16x16点阵的其他图标*/
static void display_graphic_16x16(u32 page,u32 column,const u8 *dp)
{
	u8 i,j;
 	SET_LEDCS(0);
	SET_ROMCS(1); 	
	for(j=2;j>0;j--)
	{
		led_address(column,page);
		for (i=0;i<16;i++)
		{	
			send_data_to_led(*dp);					/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			dp++;
		}
		page++;
	}
	SET_LEDCS(1);
}


/*显示8x16点阵图像、ASCII, 或8x16点阵的自造字符、其他图标*/
static void display_graphic_8x16(u8 page,u8 column,u8 *dp)
{
	u8 i,j;
	SET_LEDCS(0);	
	for(j=2;j>0;j--)
	{
		led_address(column,page);
		for (i=0;i<8;i++)
		{	
			send_data_to_led(*dp);					/*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
			dp++;
		}
		page++;
	}
	SET_LEDCS(1);
}
static void display_GB2312_string(u8 y,u8 x,u8 *text)
{
	u8 i= 0;
	u8 addr_high,addr_mid,addr_low ;
	u8 fontbuf[32];			
	while((text[i]>0x00))
	{
		if(((text[i]>=0xb0) &&(text[i]<=0xf7))&&(text[i+1]>=0xa1))
		{						
			/*国标简体（GB2312）汉字在晶联讯字库IC中的地址由以下公式来计算：*/
			/*Address = ((MSB - 0xB0) * 94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;BaseAdd=0*/
			/*由于担心8位单片机有乘法溢出问题，所以分三部取地址*/
			fontaddr = (text[i]- 0xb0)*94; 
			fontaddr += (text[i+1]-0xa1)+846;
			fontaddr = (ulong)(fontaddr*32);
			
			addr_high = (fontaddr&0xff0000)>>16;  /*地址的高8位,共24位*/
			addr_mid = (fontaddr&0xff00)>>8;      /*地址的中8位,共24位*/
			addr_low = fontaddr&0xff;	     /*地址的低8位,共24位*/
			get_n_bytes_data_from_rom(addr_high,addr_mid,addr_low,fontbuf,32 );/*取32个字节的数据，存到"fontbuf[32]"*/
			display_graphic_16x16(y,x,fontbuf);/*显示汉字到LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
			i+=2;
			x+=16;
		}
		else if(((text[i]>=0xa1) &&(text[i]<=0xa3))&&(text[i+1]>=0xa1))
		{						
			/*国标简体（GB2312）15x16点的字符在晶联讯字库IC中的地址由以下公式来计算：*/
			/*Address = ((MSB - 0xa1) * 94 + (LSB - 0xA1))*32+ BaseAdd;BaseAdd=0*/
			/*由于担心8位单片机有乘法溢出问题，所以分三部取地址*/
			fontaddr = (text[i]- 0xa1)*94; 
			fontaddr += (text[i+1]-0xa1);
			fontaddr = (ulong)(fontaddr*32);
			
			addr_high = (fontaddr&0xff0000)>>16;  /*地址的高8位,共24位*/
			addr_mid = (fontaddr&0xff00)>>8;      /*地址的中8位,共24位*/
			addr_low = fontaddr&0xff;	     /*地址的低8位,共24位*/
			get_n_bytes_data_from_rom(addr_high,addr_mid,addr_low,fontbuf,32 );/*取32个字节的数据，存到"fontbuf[32]"*/
			display_graphic_16x16(y,x,fontbuf);/*显示汉字到LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
			i+=2;
			x+=16;
		}
		else if((text[i]>=0x20) &&(text[i]<=0x7e))	
		{						
			unsigned char fontbuf[16];			
			fontaddr = (text[i]- 0x20);
			fontaddr = (unsigned long)(fontaddr*16);
			fontaddr = (unsigned long)(fontaddr+0x3cf80);			
			addr_high = (fontaddr&0xff0000)>>16;
			addr_mid = (fontaddr&0xff00)>>8;
			addr_low = fontaddr&0xff;

			get_n_bytes_data_from_rom(addr_high,addr_mid,addr_low,fontbuf,16 );/*取16个字节的数据，存到"fontbuf[32]"*/
			
			display_graphic_8x16(y,x,fontbuf);/*显示8x16的ASCII字到LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
			i+=1;
			x+=8;
		}
		else
			i++;	
	}
	
}

/*显示5*7点阵图像、ASCII, 或5x7点阵的自造字符、其他图标*/
static void display_graphic_5x7(u32 page,u8 column,u8  *dp)
{
	u32 col_cnt;
	u8  page_address;
	u8  column_address_l,column_address_h;
	page_address = 0xb0+page-1;
	
	SET_LEDCS(0);	
	
	column_address_l =(column&0x0f)-1;
	column_address_h =((column>>4)&0x0f)+0x10;
	
	send_cmd_to_led(page_address); 		/*Set Page Address*/
	send_cmd_to_led(column_address_h);	/*Set MSB of column Address*/
	send_cmd_to_led(column_address_l);	/*Set LSB of column Address*/
	
	for (col_cnt=0;col_cnt<6;col_cnt++)
	{	
		send_data_to_led(*dp);
		dp++;
	}
	SET_LEDCS(1);
}
static void display_string_5x7(u8 y,u8 x,u8 *text)
{
	u8 i = 0;
	u8 addr_high,addr_mid,addr_low;
	while((text[i]>0x00))
	{
		if((text[i]>=0x20) &&(text[i]<=0x7e))	
		{						
			u8 fontbuf[8];			
			fontaddr = (text[i]- 0x20);
			fontaddr = (unsigned long)(fontaddr*8);
			fontaddr = (unsigned long)(fontaddr+0x3bfc0);			
			addr_high = (fontaddr&0xff0000)>>16;
			addr_mid = (fontaddr&0xff00)>>8;
			addr_low = fontaddr&0xff;

			get_n_bytes_data_from_rom(addr_high,addr_mid,addr_low,fontbuf,8);//取8个字节的数据，存到"fontbuf[32]"
			
			display_graphic_5x7(y,x,fontbuf);//显示5x7的ASCII字到LCD上，y为页地址，x为列地址，fontbuf[]为数据
			i+=1;
			x+=6;
		}
		else
		i++;	
	}
}
static int oled_open(struct inode *inode, struct file *file){
	/* 开发板引脚    OLED引脚   功能状态  
	 * GPJCON0        -MOSI      OUT 1
     * GPJCON1        -CLK       OUT 1
     * GPJCON2        -CS1       OUT 1
	 * GPJCON3        -DC        OUT 1
	 * GPJCON4        -CS2       OUT 1
     * GPJCON5        -FSO       IN  0
	 */
	 if(!atomic_dec_and_test(&canopen)){
		atomic_inc(&canopen);//原子变量增加1
		return -EBUSY;
	}
	*gpjcon &= ~((3<<(0*2)) | (3<<(1*2)) | (3<<(2*2)) | (3<<(3*2)) | (3<<(4*2)) | (3<<(5*2)));
	*gpjcon |=  ((1<<(0*2)) | (1<<(1*2)) | (1<<(2*2)) | (1<<(3*2)) | (1<<(4*2)) | (0<<(5*2)));

	init_led();

	return 0;
}
static int oled_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg){
	int ret;
	u32 line=0;
	switch(cmd){
		case CLEAN_N:
			ret = get_user(line, (u32 *)arg);
			clear_n_screen(line & 0xFF);
		break;
		case CLEAN_ALL:
			clear_all_screen();
		break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int oled_write(struct file *file,const char __user *buf,size_t len,loff_t *ppos){
	memset(oled_val[0].text, 0, 4);
	if(copy_from_user(oled_val, buf, len)==0); //从用户空间获取打包来的oled数据
	if(oled_val[0].type == BIG_CHAR){
		display_GB2312_string(oled_val[0].page, oled_val[0].col, oled_val[0].text);
	}else if(oled_val[0].type == TINY_CHAR){
		display_string_5x7(oled_val[0].page, oled_val[0].col, oled_val[0].text);
	}else if(oled_val[0].type == GRAPHIC){
		display_graphic_16x16(oled_val[0].page, oled_val[0].col, oled_val[0].text);	
	}
	return 0;
}
static int oled_close(struct inode * inode, struct file * file){
	atomic_inc(&canopen);
	clear_all_screen();
	return 0;
}
static struct file_operations oled_fops={
    .owner   =   THIS_MODULE,   
	.open    =   oled_open,
	.release =   oled_close,
	.write   =   oled_write,
	.ioctl   =	 oled_ioctl,
};

int oled_init(void)
{
	major = register_chrdev(0, "oled_drv", &oled_fops); //把这个fops结构告诉内核
	leds_class = class_create(THIS_MODULE, "oled");
	leds_class_dev = class_device_create(leds_class,NULL, MKDEV(major, 0), NULL, "oled"); // /dev/xxx   创建一个设备

	gpjcon = (volatile unsigned long *)ioremap(rGPJCON, 16);
	gpjdat = gpjcon+1;
	
	//printk("initialized\n");
	return 0;
}
void oleds_exit(void){
	iounmap(gpjcon);
	unregister_chrdev(major, "oled_drv");
	class_device_unregister(leds_class_dev);
	class_destroy(leds_class);
}
module_init(oled_init);
module_exit(oleds_exit);
MODULE_LICENSE("GPL");

