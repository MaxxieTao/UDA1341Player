#include <linux/module.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/sound.h>
#include <linux/soundcard.h>
#include <linux/jiffies.h>
#include <linux/pm.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/semaphore.h>
#include <asm/dma.h>
#include <asm/arch/dma.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/regs-iis.h>
#include <asm/arch/regs-clock.h>
#include <linux/dma-mapping.h>
#include <asm/dma-mapping.h>
#include <asm/arch/hardware.h>
#include <asm/arch/map.h>
#define IIS_BASS_ADDR 0x55000000 //IISCON�ĵ�ַ
#define GPB_BASS_ADDR 0x56000010 //GPBCON�ĵ�ַ
#define GPE_BASS_ADDR 0x56000040 //GPECON�ĵ�ַ
#define DMA0_BASE_ADDR 0x4B000000
#define DMA1_BASE_ADDR 0x4B000040
#define DMA2_BASE_ADDR 0x4B000080
#define DMA3_BASE_ADDR 0x4B0000C0
#define BUF_SIZE 17640//16*1024
#define NBFRAG    2
#define SET_L3MODE(x) ((x==0) ? (*gpbdat &= ~(1<<2)) : (*gpbdat |= (1<<2)))
#define SET_L3DAT(x)  ((x==0) ? (*gpbdat &= ~(1<<3)) : (*gpbdat |= (1<<3)))
#define SET_L3CLK(x)  ((x==0) ? (*gpbdat &= ~(1<<4)) : (*gpbdat |= (1<<4)))
#define VOLUME_MUTE      0
#define VOLUME_UNMUTE    1
#define VOLUME_UP_DOWN   2
//#define DEBUG
#ifdef DEBUG
#define DPRINTK printk
#else
#define DPRINTK( x... )
#endif
struct iis_regs{
	unsigned long iiscon; //IIS ����
	unsigned long iismod; //IIS ģʽ
	unsigned long iispsr; //IIS Ԥ�궨��
	unsigned long iisfcon;//IIS FIFO ����
	unsigned long iisfifo;//IIS FIFO ���
};
struct dma_regs{
	unsigned long disrc;
	unsigned long disrcc;
	unsigned long didst;
	unsigned long didstc;
	unsigned long dcon;
	unsigned long dstat;
	unsigned long dcsrc;
	unsigned long dcdst;
	unsigned long dmasktrig;
};
struct frag_buffer{
	u8 *buffer;//Դ�������ַ
	u32 buffer_phy;//Դ�������ַ
};
struct double_buffer{
	u8 read;
	u8 write;
	struct frag_buffer fbuffer[NBFRAG];
};
static volatile struct iis_regs *iis_reg;
static volatile struct dma_regs *dma1_reg;
static volatile struct dma_regs *dma2_reg;

volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;
volatile unsigned long *gpbup  = NULL;

volatile unsigned long *gpecon = NULL;
volatile unsigned long *gpedat = NULL;
volatile unsigned long *gpeup  = NULL;
struct timespec ts;
static int dsp_major;
static struct class *dsp_audio_cls;
static struct class_device *mix_cls_dev;
static int mix_major;
static struct class *mix_audio_cls;
static struct class_device *dsp_cls_dev;

static struct clk *iis_clock;
static u32 playing_count;
static u32 recording_count;
static u32 dma_count;
static struct double_buffer *output_stream;
static struct double_buffer *input_stream;
static atomic_t dspopen = ATOMIC_INIT(1);//����ԭ�ӱ�������ʼ��Ϊ1
static atomic_t mixopen = ATOMIC_INIT(1);//����ԭ�ӱ�������ʼ��Ϊ1
static DECLARE_WAIT_QUEUE_HEAD(dma_waitq);
static volatile unsigned char ev_dma = 0;// �ж��¼���־, �жϷ����������1
static DECLARE_MUTEX(ev_lock);//���廥����
static struct double_buffer *init_double_buffer(void){
	u8 i;
	struct double_buffer *dbuffer = kmalloc(sizeof(struct double_buffer),GFP_KERNEL);
	dbuffer->read=0;
	dbuffer->write=0;
	for(i=0;i<NBFRAG;i++){
		 dbuffer->fbuffer[i].buffer = dma_alloc_writecombine(NULL, BUF_SIZE, &(dbuffer->fbuffer[i].buffer_phy), GFP_KERNEL);
	}
	return dbuffer;
}
static void *free_double_buffer(struct double_buffer *dbuffer){
	u8 i;
	for(i=0;i<NBFRAG;i++){
		dma_free_writecombine(NULL, BUF_SIZE, dbuffer->fbuffer[i].buffer, (dbuffer->fbuffer[i].buffer_phy));
		dbuffer->fbuffer[i].buffer= NULL;
	}
	kfree(dbuffer);
	dbuffer = NULL;
	return NULL;
}
static void send_l3_address(u8 data){
	int i;
	unsigned long flags;
	local_irq_save(flags);
	SET_L3MODE(0);//���͵�ַģʽ
	SET_L3CLK(1);//�ߵ�ƽ
	udelay(1);
	for (i = 0; i < 8; i++) {
		if(data & 0x01){ //�����λΪ1
			SET_L3CLK(0);//��Ϊ�͵�ƽ
			SET_L3DAT(1);
			udelay(1);
			SET_L3CLK(1);//��Ϊ�ߵ�ƽ���ȴ���һ�η���
		}else{
			SET_L3CLK(0);//��Ϊ�͵�ƽ
			SET_L3DAT(0);
			udelay(1);
			SET_L3CLK(1);//��Ϊ�ߵ�ƽ���ȴ���һ�η���
		}
		data >>= 1;
	}
	SET_L3MODE(1);//��������ģʽ
	SET_L3CLK(1);//�ߵ�ƽ
	local_irq_restore(flags);
}
static void send_l3_data(u8 data){
	int i;
	unsigned long flags;
	local_irq_save(flags);
	udelay(1);
	for (i = 0; i < 8; i++) {
		if (data & 0x1) {//�����λΪ1
			SET_L3CLK(0);//��Ϊ�͵�ƽ
			SET_L3DAT(1);
			udelay(1);
			SET_L3CLK(1);//��Ϊ�ߵ�ƽ���ȴ���һ�η���
		}else {
			SET_L3CLK(0);//��Ϊ�͵�ƽ
			SET_L3DAT(0);
			udelay(1);
			SET_L3CLK(1);//��Ϊ�ߵ�ƽ���ȴ���һ�η���
		}
		data >>= 1;
	}	
	local_irq_restore(flags);
}

/*
 * GPB 2: L3MODE
 * GPB 3: L3DATA
 * GPB 4: L3CLOCK
 */
static void init_uda1341(void){
	unsigned long flags;
	//1.����gpio��
	local_irq_save(flags);
	SET_L3MODE(1);
	SET_L3CLK(1);
	//2.uda1341��λ  
	//2.1���͵�ַ   UDA1341�豸��ַ[7:2]000101  + ��������STATUS[1:0]10
	send_l3_address(0x16); //00010110 = 0x16
	//2.2��������  0101 1001
	/* STATUS   ״̬���ƼĴ���
	 * bit[7]   0   ѡ���0��Ĵ��� 
	 * bit[6]   1   ��λ  
   	 * bit[5:4] 01  ʱ��ѡ��384fs
	 * bit[3:1] 100 MSB ���������ʽ 
	 * bit[0]   1   ֱ���˲���
	 */
	send_l3_data(0x59); //0101 1001 = 0x59
	//2.3��������  1000 0011
	/* STATUS   
	 * bit[7]   1 ѡ���1��Ĵ��� 
	 * bit[1]   1 ����ADC 
	 * bit[0]   1 ����DAC
	*/
	send_l3_data(0x83); //1000 0011 = 0x83
	//3.uda1341����
	//3.1 ���͵�ַ   UDA1341�豸��ַ[7:2]000101  +  ��������DATA0[1:0]00
	send_l3_address(0x14); //00010100 = 0x14
	/*  DATA0
	 *  bit[7:6] 00       ѡ����������Ĵ���
	 *  bit[5:0] 001111   (0x0Ϊ�������0db   ֵԽ��  ����ԽС)
	 */
	send_l3_data(0x0F); //00001111 = 0x0F
	/*  DATA0
	 *  bit[7:6] 01    ѡ����Ƶ�������Ч���Ĵ���
	 *  bit[5:2] 1111  ������ǿ18dB
	 *  bit[1:0] 00    ����0dB
	 */
	send_l3_data(0x7C); //01111100 = 0x7C
	/*  DATA0
	 *  bit[7:6] 10 ��ֵ���λ��  ȥ����  ����
	 *  bit[5]   1  
	 *  bit[4:3] 00 Ĭ��ֵ  
	 *  bit[2]   0  ������
	 *  bit[1:0] 00 Ĭ��ֵ
 	 */	
 	send_l3_data(0xA0); //10100000 = 0xA0
	/*  DATA0
	 *  bit[7:6] 11 ��չ��ַ
	 *  bit[5:3] 000  ������ Ĭ��0
	 *  bit[3:0] 010 ��˷����ж� ������ģʽ  
 	 */	
 	send_l3_data(0xC2); //11000010 = 0xC2
	/*  DATA0
	 *  bit[7:5] 111 ��չ����
	 *  bit[4:2] 110 ��˷����ж� +27dB
	 *  bit[1:0] 10  ѡ��ͨ��2
 	 */	
 	send_l3_data(0xFA); //11111010 = 0xFA
 	/*  DATA0
	 *  bit[7:6] 11 ��չ��ַ
	 *  bit[5:3] 000  ������ Ĭ��0
	 *  bit[3:0] 101 ��������  
 	 */	
 	send_l3_data(0xC5); //11000101 = 0xC5
	/*  DATA0
	 *  bit[7:5] 111 ��չ����
	 *  bit[4:0] 00000  ������ֵ
 	 */	
 	send_l3_data(0xF0); //11110000 = 0xF0
 	local_irq_restore(flags);
}
static void init_l3_gpio(void){
	/*
	 * GPB 2: L3MODE,  OUTPUT 01
	 * GPB 3: L3DATA,  OUTPUT 01
	 * GPB 4: L3CLOCK, OUTPUT 01
	 * ����ʹ��
	 */
	unsigned long  flags;
	local_irq_save(flags);
	*gpbcon &= ~((0x3<<(2*2)) | (0x3<<(3*2)) | (0x3<<(4*2)));//����
	*gpbcon |=  ((0x1<<(2*2)) | (0x1<<(3*2)) | (0x1<<(4*2)));//����Ϊ�������
	*gpbup  |=  (0x1<<2) | (0x1<<3) | (0x1<<4);//����
	local_irq_restore(flags);//�ָ��ж�
}
static void init_iis_gpio(void){
	/*
	 * GPE 0: I2SLRCK 10
	 * GPE 1: I2SSCLK 10
	 * GPE 2: CDCLK   10
	 * GPE 3: I2SSDI  10
	 * GPE 4: I2SSDO  10
	 */
	unsigned long  flags;
	local_irq_save(flags);
	*gpecon &= ~((0x3<<(0*2)) | (0x3<<(1*2)) | (0x3<<(2*2)) | (0x3<<(3*2)) | (0x3<<(4*2)));//����
	*gpecon |=  ((0x2<<(0*2)) | (0x2<<(1*2)) | (0x2<<(2*2)) | (0x2<<(3*2)) | (0x2<<(4*2)));//����Ϊ��������
	*gpeup  &= ~((0x1<<0) | (0x1<<1) | (0x1<<2) | (0x1<<3) | (0x1<<4));
	local_irq_restore(flags);//�ָ��ж�
}
static void init_iis(void){
	unsigned long flags;
	local_irq_save(flags);
	iis_reg->iiscon  = 0x0;
	iis_reg->iispsr  = 0x0;
	iis_reg->iismod  = 0x0; 
	iis_reg->iisfcon = 0x0;
	iis_reg->iisfifo = 0x0;
	local_irq_restore(flags);//�ָ��ж�
	clk_disable(iis_clock);
}
/*��������Ϊ¼��*/
static void iis_rx(void){
	unsigned long flags;
	local_irq_save(flags);
	iis_reg->iiscon  = 0x0;
	iis_reg->iispsr  = 0x0;
	iis_reg->iismod  = 0x0; 
	iis_reg->iisfcon = 0x0;
	iis_reg->iisfifo = 0x0;
	clk_enable(iis_clock);
	local_irq_restore(flags);//�ָ��ж�
	local_irq_save(flags);
	/* IISCON
	 * bit[5] ���� DMA �������� 1-ʹ��
	 * bit[4] ���� DMA ��������	1-ʹ��
	 * bit[3] ����ͨ����������	1-����
	 * bit[2] ����ͨ����������	0-�ǿ���
	 * bit[1] IIS Ԥ��Ƶ��	    1-ʹ��
	 * bit[0] IIS �ӿ�	        0-�ر�
	 */
	iis_reg->iiscon  |= (1<<5) | (1<<4) | (1<<3) | (0<<2) | (1<<1) | (0<<0);
	/* IISMOD
	 * bit[9]   ��ʱ��ѡ��             0 -PLCK
	 * bit[8]   ��/�ӻ�ģʽѡ��        0 -����ģʽ��IISLRCK �� IISCLK Ϊ���ģʽ��
	 * bit[7:6] ����/����ģʽѡ��      01-����ģʽ (¼��)
	 * bit[5]   ��/��ͨ������Ч��ƽ    0 -����ģʽ��IISLRCK �� IISCLK Ϊ���ģʽ��
	 * bit[4]   ���нӿڸ�ʽ           1 -MSB���󣩶����ʽ
	 * bit[3]   ��������ÿͨ��         1 -16 λ
	 * bit[2]   ��ʱ��Ƶ��ѡ��         1 -384fs
	 * bit[1:0] ����λʱ��Ƶ��ѡ��     01-32fs
	 */
	iis_reg->iismod  |= (0<<9) | (0<<8) | (1<<6) | (0<<5) | (1<<4) | (1<<3) | (1<<2) |(1<<0);   
	/* IISPSR
	 * bit[9:5] Ԥ��Ƶ������ A  2
	 * bit[4:0] Ԥ��Ƶ������ B  2
	 */
	iis_reg->iispsr  |= (2<<5) | (2<<0);
	/* IISFCON
	 * bit[15] ���� FIFO ����ģʽѡ�� 1-DMA
	 * bit[14] ���� FIFO ����ģʽѡ�� 1-DMA
	 * bit[13] ���� FIFO (����)       1-��ֹ
	 * bit[12] ���� FIFO (¼��)       1-ʹ��
	 */	
	iis_reg->iisfcon |= (1<<15) | (1<<14) |(0<<13) | (1<<12);
	local_irq_restore(flags);//�ָ��ж�
}
static irqreturn_t dma_irq_rx(int irq,void *devid){
	input_stream->write = !input_stream->write;
	down(&ev_lock);
	ev_dma = 1;
	up(&ev_lock);
	wake_up_interruptible(&dma_waitq);//�������ߵĽ���
	dma1_reg->didst  = (u32)input_stream->fbuffer[input_stream->write].buffer_phy;
	dma1_reg->dmasktrig = (0<<2) | (1<<1) | (0<<0);//����DMA
	return IRQ_HANDLED;
}
static int start_dma_rx(void){
	unsigned long  flags;
	local_irq_save(flags);
	dma1_reg->didst  = (u32)input_stream->fbuffer[input_stream->write].buffer_phy;
	dma1_reg->dmasktrig = (0<<2) | (1<<1) | (0<<0);//����DMA
	local_irq_restore(flags);//�ָ��ж�
}
static int init_dma_rx(void){
	unsigned long  flags;
	local_irq_save(flags);
	dma1_reg->disrc  = (u32)0x55000010;
	/* DISRCC
     * bit[1] 1-Դ���������ߣ�APB����
     * bit[0] 1-�̶���ַ
	 */
	dma1_reg->disrcc = (1<<1) | (1<<0);
	//dma2_reg->didst  = 
	/* DIDSTC
	 * bit[2] 0���� TC ���� 0 ʱ�����ж�
     * bit[1] 0��Ŀ����ϵͳ���ߣ�AHB����
     * bit[0] 0������
	 */
	dma1_reg->didstc = (0<<2) | (0<<1) | (0<<0);
	/* DCON
	 * bit[31]     1-ѡ������ģʽ
	 * bit[30]     
	 * bit[29]     1-�����д�����ɲ����ж����󣨼� CURR_TC ��Ϊ 0��
	 * bit[26:24]  010-I2SSDI
     * bit[23]     0-Ŀ����ϵͳ���ߣ�AHB����
     * bit[22]     1-���Զ�����
     * bit[21:20]  01-����
     * bit[19:0]   
	 */
	dma1_reg->dcon   = (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(2<<24)|(1<<23)|(1<<22)|(1<<20)|((BUF_SIZE>>1)<<0);
	/* DMASKTRIG
     * bit[2] 1-ֹͣ
     * bit[1] 0-�ر�ͨ��
	 */
	dma1_reg->dmasktrig = (1<<2) | (0<<1) | (0<<0);//������DMA
	local_irq_restore(flags);//�ָ��ж�
}
/*��������Ϊ����*/
static void iis_tx(void){
	unsigned long flags;
	local_irq_save(flags);
	iis_reg->iiscon  = 0x0;
	iis_reg->iispsr  = 0x0;
	iis_reg->iismod  = 0x0; 
	iis_reg->iisfcon = 0x0;
	iis_reg->iisfifo = 0x0;
	clk_enable(iis_clock);
	local_irq_restore(flags);//�ָ��ж�
	local_irq_save(flags);
	/* IISCON
	 * bit[5] ���� DMA �������� 1-ʹ��
	 * bit[4] ���� DMA ��������	1-ʹ��
	 * bit[3] ����ͨ����������	0-�ǿ���
	 * bit[2] ����ͨ����������	1-����
	 * bit[1] IIS Ԥ��Ƶ��	    1-ʹ��
	 * bit[0] IIS �ӿ�	        0-�ر�
	 */
	iis_reg->iiscon  |= (1<<5) | (1<<4) | (0<<3) | (1<<2) | (1<<1) | (0<<0);
	/* IISMOD
	 * bit[9]   ��ʱ��ѡ��             0 -PLCK
	 * bit[8]   ��/�ӻ�ģʽѡ��        0 -����ģʽ��IISLRCK �� IISCLK Ϊ���ģʽ��
	 * bit[7:6] ����/����ģʽѡ��      10-����ģʽ
	 * bit[5]   ��/��ͨ������Ч��ƽ    0 -����ģʽ��IISLRCK �� IISCLK Ϊ���ģʽ��
	 * bit[4]   ���нӿڸ�ʽ           1 -MSB���󣩶����ʽ
	 * bit[3]   ��������ÿͨ��         1 -16 λ
	 * bit[2]   ��ʱ��Ƶ��ѡ��         1 -384fs
	 * bit[1:0] ����λʱ��Ƶ��ѡ��     01-32fs
	 */
	iis_reg->iismod  |= (0<<9) | (0<<8) | (2<<6) | (0<<5) | (1<<4) | (1<<3) | (1<<2) |(1<<0);   
	/* IISPSR
	 * bit[9:5] Ԥ��Ƶ������ A  2
	 * bit[4:0] Ԥ��Ƶ������ B  2
	 */
	iis_reg->iispsr  |= (2<<5) | (2<<0);
	/* IISFCON
	 * bit[15] ���� FIFO ����ģʽѡ�� 1-DMA
	 * bit[14] ���� FIFO ����ģʽѡ�� 1-DMA
	 * bit[13] ���� FIFO (����)       1-ʹ��  
	 * bit[12] ���� FIFO (¼��)       0-��ֹ
	 */	
	iis_reg->iisfcon |= (1<<15) | (1<<14) |(1<<13) | (0<<12);
	local_irq_restore(flags);//�ָ��ж�
}
static irqreturn_t dma_irq_tx(int irq,void *devid){
	dma_count++;
	output_stream->read = !output_stream->read;
	dma2_reg->disrc  = (u32)output_stream->fbuffer[output_stream->read].buffer_phy;
	if(playing_count>dma_count)
		dma2_reg->dmasktrig = (0<<2) | (1<<1) | (0<<0);//����DMA
	down(&ev_lock);
	ev_dma = 1;
	up(&ev_lock);
	wake_up_interruptible(&dma_waitq);//�������ߵĽ���
	return IRQ_HANDLED;
}
static void start_dma_tx(void){
	unsigned long  flags;
	local_irq_save(flags);
	dma2_reg->disrc  = (u32)output_stream->fbuffer[output_stream->read].buffer_phy;
	dma2_reg->dmasktrig = (0<<2) | (1<<1) | (0<<0);//����DMA
	local_irq_restore(flags);//�ָ��ж�
}
static void init_dma_tx(void){
	unsigned long  flags;
	local_irq_save(flags);
	//dma2_reg->disrc  = (u32)buffer_phy[0];
	dma2_reg->disrcc = (0<<1) | (0<<0);
	dma2_reg->didst  = (u32)0x55000010;
	dma2_reg->didstc = (0<<2) | (1<<1) | (1<<0);
	dma2_reg->dcon   = (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(0<<24)|(1<<23)|(1<<22)|(1<<20)|((BUF_SIZE>>1)<<0);
	dma2_reg->dmasktrig = (1<<2) | (0<<1) | (0<<0);//������DMA
	local_irq_restore(flags);//�ָ��ж�
}
static int player_open(struct inode *inode, struct file *file){
	/*�Լ�������������Ƿ�Ϊ0��Ϊ0�򷵻�true�����򷵻�false
	 *����Լ���dspopen���ڵ���0
	 */
	if(!atomic_dec_and_test(&dspopen)){
		atomic_inc(&dspopen);//ԭ�ӱ�������1
		return -EBUSY;
	}
	if((file->f_flags & O_ACCMODE) == O_WRONLY){//����
		iis_tx();
		init_dma_tx();
		playing_count=0;
		dma_count=0;
		//memset(output_stream->fbuffer[0].buffer, 0, BUF_SIZE);
		//memset(output_stream->fbuffer[1].buffer, 0, BUF_SIZE);
		output_stream->read=0;
		output_stream->write=0;
		iis_reg->iiscon |= (1<<0);
	}else if((file->f_flags & O_ACCMODE) == O_RDONLY){//¼��
		iis_rx();
		init_dma_rx();
		recording_count=0;
		//memset(input_stream->fbuffer[0].buffer, 0, BUF_SIZE);
		//memset(input_stream->fbuffer[1].buffer, 0, BUF_SIZE);
		input_stream->read=0;
		input_stream->write=0;
		iis_reg->iiscon |= (1<<0);
	}else		
		return -EINVAL;
	DPRINTK("open success\n");
	return 0;
}

static int player_close(struct inode *inode, struct file *file){
	atomic_inc(&dspopen);
	iis_reg->iiscon |= (0<<0);
	dma2_reg->dmasktrig =  (1<<2) |(0<<1);//�ر�DMA
	playing_count=0;
	recording_count=0;
	DPRINTK("close success\n");
	return 0;
}
static int player_write(struct file *file,const char __user *buf,size_t len,loff_t *ppos){
	playing_count++;
	if(playing_count==1){//��һ�μ������ݣ��뻺����0
		if(copy_from_user(output_stream->fbuffer[output_stream->write].buffer, buf, len)==0){
			output_stream->write = !output_stream->write;
		}
		else
			DPRINTK("copy error\n");
		start_dma_tx();
	}else{
		if(copy_from_user(output_stream->fbuffer[output_stream->write].buffer, buf, len)==0){
			output_stream->write = !output_stream->write;
		}
		else
			DPRINTK("copy error\n");
		down(&ev_lock);
		ev_dma=0;//�ȴ�����������DMA���  ��������
		up(&ev_lock);
		wait_event_interruptible(dma_waitq, ev_dma);
	}
	//DPRINTK("player_write\n");
	return 0;
}
static int player_read(struct file *file, char __user *buf,size_t len,loff_t *ppos){
	recording_count++;
	if(recording_count==1){//��һ��DMA���ݣ��뻺����0
		start_dma_rx();//����DMA
	}else{
		down(&ev_lock);
		ev_dma=0;
		up(&ev_lock);
		wait_event_interruptible(dma_waitq, ev_dma);
		if(copy_to_user(buf, input_stream->fbuffer[input_stream->read].buffer, len)==0){
			input_stream->read = !input_stream->read;
		}
		else
			DPRINTK("copy error\n");
	}
	//DPRINTK("player_read\n");
	return 1;
}
static int mixer_open(struct inode *inode, struct file *file){
	if(!atomic_dec_and_test(&mixopen)){
		atomic_inc(&mixopen);//ԭ�ӱ�������1
		return -EBUSY;
	}
	return 0;
}
static int mixer_close(struct inode *inode, struct file *file){
	atomic_inc(&mixopen);
	return 0;
}
static int mixer_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg){
	int ret;
	long volume;
	switch(cmd){
		case VOLUME_MUTE:
			send_l3_address(0x14); //00010100 = 0x14
			/*  DATA0
			 *  bit[7:6] 10 ��ֵ���λ��  ȥ����  ����
			 *  bit[2]   1  ����
		 	 */	
		 	send_l3_data(0x84); //10100100 = 0x84
		break;
		case VOLUME_UNMUTE:
			send_l3_address(0x14); //00010100 = 0x14
			/*  DATA0
			 *  bit[7:6] 10 ��ֵ���λ��  ȥ����  ����
			 *  bit[2]   0  ������
		 	 */	
		 	send_l3_data(0x80); //10100000 = 0x80		
		break;
		case VOLUME_UP_DOWN://��������   ��ֵԽС  ����Խ��
			ret = get_user(volume,(long *)arg);
			if (ret)
				return ret;
			send_l3_address(0x14); //00010100 = 0x14
			volume = volume & 0xFF;
			send_l3_data(volume); //10100000 = 0x80		
		break;
		default:
			return -EINVAL;
	}
	return 0;
}
static struct file_operations player_mix_fops ={
	.owner   = THIS_MODULE,
	.open    = mixer_open,
	.release = mixer_close,
	.ioctl   = mixer_ioctl
};
static struct file_operations player_dsp_fops ={
	.owner   = THIS_MODULE,
	.open    = player_open,
	.release = player_close,
	.write   = player_write,
	.read    = player_read,
	
};
static int uda1341_init(void){
	dsp_major     = register_chrdev(0,"uda1341_drv", &player_dsp_fops);
	dsp_audio_cls = class_create(THIS_MODULE,"player_dsp");
	dsp_cls_dev   = class_device_create(dsp_audio_cls, NULL, MKDEV(dsp_major,0), NULL, "dsp");
	mix_major     = register_chrdev(0,"uda1341_drv", &player_mix_fops);
	mix_audio_cls = class_create(THIS_MODULE,"player_mix");
	mix_cls_dev   = class_device_create(mix_audio_cls, NULL, MKDEV(mix_major,0), NULL, "mix");
	iis_reg  = ioremap(IIS_BASS_ADDR, sizeof(struct iis_regs));//IO�ڴ���ӳ��
	dma1_reg = ioremap(DMA1_BASE_ADDR, sizeof(struct dma_regs));
	dma2_reg = ioremap(DMA2_BASE_ADDR, sizeof(struct dma_regs));
	gpbcon   = ioremap(GPB_BASS_ADDR, sizeof(unsigned long));
	gpbdat   = gpbcon+1;
	gpbup    = gpbdat+1;
	gpecon   = ioremap(GPE_BASS_ADDR, sizeof(unsigned long));
	gpedat   = gpecon+1;
	gpeup    = gpedat+1;
	//���ʱ����Դ
	iis_clock = clk_get(NULL, "iis");
	if (iis_clock == NULL){		
		DPRINTK("Failed to find clock source\n");		
		return -ENOENT;	
	}
	//ʹ��ʱ��
	clk_enable(iis_clock);
	
	init_l3_gpio();
	init_iis_gpio();

	init_iis();
	init_uda1341();  //��ʼ��UDA1341  ��ͨ��L3�������üĴ���
	output_stream = init_double_buffer();
	input_stream  = init_double_buffer();
	if(request_irq(IRQ_DMA1, dma_irq_rx,0, "dma_irq_rx", (void*)1)){//ע��¼����DMA�ж�
		DPRINTK("can not request irq for dma\n");
		return -EBUSY;
	}
	if(request_irq(IRQ_DMA2, dma_irq_tx,0, "dma_irq_tx", (void*)2)){//ע�Ქ�ŵ�DMA�ж�
		DPRINTK("can not request irq for dma\n");
		return -EBUSY;
	}
	return 0;
}
static void uda1341_exit(void){
	output_stream = free_double_buffer(output_stream);
	input_stream = free_double_buffer(input_stream);
	free_irq(IRQ_DMA2, (void*)2);
	free_irq(IRQ_DMA1, (void*)1);
	class_device_unregister(dsp_cls_dev);	
	class_destroy(dsp_audio_cls);
	class_device_unregister(mix_cls_dev);	
	class_destroy(mix_audio_cls);
	unregister_chrdev(dsp_major,"uda1341_drv");
	unregister_chrdev(mix_major,"uda1341_drv");
	clk_disable(iis_clock);
	iounmap(iis_reg);
	iounmap(dma2_reg);
	iounmap(dma1_reg);
	iounmap(gpbcon);
	iounmap(gpecon);
}

module_init(uda1341_init);
module_exit(uda1341_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TaoJiaJun");
