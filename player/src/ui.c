#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include "base.h"
#include "graphy.h"
#include "file.h"
#include "handle.h"
#include "ui.h"
/**********************************************************************
 * �������ƣ� error_missing_the_file
 * ���������� ��ʾ�ļ���ʧ��ʾ
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void error_missing_the_file(){           
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 36, "�ļ���ʧ");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 8, "���ֶ������豸");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� error_missing_the_btn
 * ���������� ��ʾ��ʧ����������ʾ
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void error_missing_the_btn(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "����������ʧ");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 8, "�ƽ�����Ա����");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� error_missing_the_mix
 * ���������� ��ʾ��ʧ��Ƶ������ʾ
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void error_missing_the_mix(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "��Ƶ������ʧ");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 8, "�ƽ�����Ա����");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� error_init_list
 * ���������� ��ʾ�����б������ʾ
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void error_init_list(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "�����б����");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 24, "10�������");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� error_create_list
 * ���������� ��ʾ�����б������ʾ
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void error_create_list(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "�����б����");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 24, "10�������");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� error_other
 * ���������� ��ʾ����������ʾ
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void error_other(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 36, "ϵͳ�쳣");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 24, "10�������");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}
/**********************************************************************
 * �������ƣ� init_oled_type_data
 * ���������� ��ʼ��oled����
 * ��������� oled_val_t�ṹ��ָ��,
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void init_oled_type_data(oled_val_t *p){
	memset(p->text,0,32);
	p->page = 0;
	p->col  = 0;
	p->type = 0;
}





/**********************************************************************
 * �������ƣ� package_oled_val
 * ���������� ���oled����
 * ��������� type��ʾ����
 *            page��ʾ��
 *            col ��ʾ��
 *            text��ʾ���ı�
 * ��������� ��
 * �� �� ֵ�� oled_val_t�ṹ��ָ��
 ***********************************************************************/
static oled_val_t *package_oled_val(u8 type, u8 page, u8 col, const u8 *text){
	u8 i;
	memset(oled_val->text,'\0',sizeof(oled_val->text));
	oled_val->type = type;
	oled_val->page = page;
	oled_val->col  = col;
	memcpy(oled_val->text,text,sizeof(oled_val->text));	
	return oled_val;
}





/**********************************************************************
 * �������ƣ� refresh_status
 * ���������� ���²���״̬    ֹͣ  ��ͣ  ����ͼ��
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_status(){//ֹͣ ��ͣ  ����
	if(flags->is_allow == 0){      //ֹͣ״̬
		if(flags->is_pause == 1)   //��ͣģʽ
			oled_val = package_oled_val(GRAPHIC, 0, 1, pause_graphy);
		else
			oled_val = package_oled_val(GRAPHIC, 0, 1, stop_graphy);
	}else{                          //����״̬
		if(flags->is_pause == 1)   //��ͣģʽ
			oled_val = package_oled_val(GRAPHIC, 0, 1, pause_graphy);
		else
			oled_val = package_oled_val(GRAPHIC, 0, 1, play_graphy);
		
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_cur_count
 * ���������� ���µ�ǰ�����ǵڼ�����Ƶ    ֹͣ  ��ͣ  ����ͼ��
 * ��������� list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_cur_count(list_t *plist){ 
	u8 type;
	char temp_buffer[4];
	type = get_list_type(plist);
	if(plist->size > 0){
		if(type == 0){  //�����б�
			snprintf(temp_buffer,sizeof(temp_buffer),"%03d",play_node->id);  
			oled_val = package_oled_val(BIG_CHAR, 0, 23, temp_buffer);

		}else{          //¼���б�
			snprintf(temp_buffer,sizeof(temp_buffer),"%03d",record_node->id);  
			oled_val = package_oled_val(BIG_CHAR, 0, 23, temp_buffer);
		}
	}else{                //�б�Ϊ��
		oled_val = package_oled_val(BIG_CHAR, 0, 23, "000");//��Ƶ����Ϊ0
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_split
 * ���������� ��ʾ�ָ���ͼ��
 * ��������� list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_split(){            //�ָ���
	oled_val = package_oled_val(BIG_CHAR, 0, 47, "/");
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_clean_2_to_7
 * ���������� �����2����7��
 * ��������� list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_clean_2_to_7(){
	u8 line;
	for(line = 2;line < 7;line++)
		ioctl(fd_oled,CLEAN_N,&line); 
}





/**********************************************************************
 * �������ƣ� refresh_time_min
 * ���������� ���²���ʱ�������
 * ��������� ����
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_time_min(u32 min){
	u8 min_buffer[4];
	snprintf(min_buffer,sizeof(min_buffer),"%02d",min);  
	oled_val = package_oled_val(BIG_CHAR, 4, 0, min_buffer);	
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_time_colon
 * ���������� ����ð��ͼ��
 * ��������� ����
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_time_colon(){            //ð��
	oled_val = package_oled_val(BIG_CHAR, 4, 16, ":");
	write(fd_oled,oled_val,sizeof(oled_val_t));
}




/**********************************************************************
 * �������ƣ� refresh_time_sec
 * ���������� ���²���ʱ��������
 * ��������� ����
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_time_sec(u32 sec){
	u8 sec_buffer[4];
	snprintf(sec_buffer,sizeof(sec_buffer),"%02d",sec);  
	oled_val = package_oled_val(BIG_CHAR, 4, 24, sec_buffer);	
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_time
 * ���������� ���²���ʱ��  ���ݽ���ѭ���Ĵ���  ����ʱ��
 * ��������� ѭ������
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_time(u32 count){// 1 sec = 10 count
	u32 min;
	u32 sec;
	u32 total_sec;
	total_sec = count/10;  //����һ��������
	if(total_sec < 60){
		sec = total_sec;
		min = 0;
	}else{
		min = total_sec / 60;
		sec = total_sec % 60;
	}
	refresh_time_min(min);
	refresh_time_colon();
	refresh_time_sec(sec);
}





/**********************************************************************
 * �������ƣ� refresh_all_count
 * ���������� ������Ƶ����
 * ��������� plist�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_all_count(list_t *plist){  //��Ƶ����
	char temp_buffer[4];
	if(plist->size > 0){
		snprintf(temp_buffer,sizeof(temp_buffer),"%03d",plist->size);  
		oled_val = package_oled_val(BIG_CHAR, 0, 55, temp_buffer);	
	}else{                //�б�Ϊ��
		oled_val = package_oled_val(BIG_CHAR, 0, 55, "000");//��Ƶ����Ϊ0
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_shift_mode
 * ���������� ���²���˳��ͼ��    ˳��  ���  ����ѭ��
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_shift_mode(){  //
	if(flags->shift_mode == 0){
		oled_val = package_oled_val(GRAPHIC, 0, 87, loop_graphy);
	}else if(flags->shift_mode == 1){
		oled_val = package_oled_val(GRAPHIC, 0, 87, shuffle_graphy);
	}else if(flags->shift_mode == 2){
		oled_val = package_oled_val(GRAPHIC, 0, 87, repeat_graphy);
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * �������ƣ� refresh_volume
 * ���������� ��������ͼ��
 * ��������� volume����
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_volume(u8 volume){
	if(volume < 10){  //�������
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume7_graphy);
	}else if(volume >=10 && volume < 20){
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume6_graphy);
	}else if(volume >=20 && volume < 30){
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume5_graphy);
	}else if(volume >=30 && volume < 40){
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume4_graphy);
	}else if(volume >=40 && volume < 50){
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume3_graphy);
	}else if(volume >=50 && volume < 60){
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume2_graphy);
	}else if(volume >=60 && volume < 70){  //������С
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume1_graphy);
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}






/**********************************************************************
 * �������ƣ� refresh_audio_name
 * ���������� ������Ƶ����
 * ��������� plist�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void refresh_audio_name(list_t *plist){
	u8 type;
	char temp_buffer[4];
	type = get_list_type(plist);
	if(plist->size > 0){
		if(type == 0){  //�����б�
			oled_val = package_oled_val(BIG_CHAR, 2, 0, play_node->gbk_name);	
		}else{          //¼���б�
			oled_val = package_oled_val(BIG_CHAR, 2, 0, record_node->gbk_name);
		}
	}else{                //�б�Ϊ��
		oled_val = package_oled_val(BIG_CHAR, 3, 50, "NO LIST");
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� refresh_record_name
 * ���������� ��ʾ¼��ʱ���ļ���
 * ��������� text�ļ���
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_record_name(const u8 *text){
	oled_val = package_oled_val(BIG_CHAR, 2, 0,text);	
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * �������ƣ� refresh_mode
 * ���������� ���²���ģʽ    ����   ¼��
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_mode(){
	int line;
	if(flags->mode==0){ //����ģʽ
		oled_val = package_oled_val(TINY_CHAR, 8, 1, "P");         
		write(fd_oled,oled_val,sizeof(oled_val_t));
		oled_val = package_oled_val(TINY_CHAR, 8, 6, " MODE");
		write(fd_oled,oled_val,sizeof(oled_val_t));
	}else{               //¼��ģʽ
		line = 7;
		ioctl(fd_oled,CLEAN_N,&line);
		oled_val = package_oled_val(BIG_CHAR, 0, 23, "000");
		write(fd_oled,oled_val,sizeof(oled_val_t));
		oled_val = package_oled_val(BIG_CHAR, 0, 55, "000");//¼��ģʽ��  ��ʾ����Ϊ0
		write(fd_oled,oled_val,sizeof(oled_val_t));
		oled_val = package_oled_val(TINY_CHAR, 8, 1, "R");
		write(fd_oled,oled_val,sizeof(oled_val_t));
		oled_val = package_oled_val(TINY_CHAR, 8, 6, " MODE");
		write(fd_oled,oled_val,sizeof(oled_val_t));
		refresh_clean_2_to_7();
		refresh_time(0);
	}
}





/**********************************************************************
 * �������ƣ� refresh_list_mode
 * ���������� ���²����б�    �����б�   ¼���б�
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_list_mode(){
	if(flags->list_mode==0){ //�����б�
		oled_val = package_oled_val(TINY_CHAR, 8, 93, "M");
		write(fd_oled,oled_val,sizeof(oled_val_t));
	}else{                  //¼���б�
		oled_val = package_oled_val(TINY_CHAR, 8, 93, "R");
		write(fd_oled,oled_val,sizeof(oled_val_t));
	}
	oled_val = package_oled_val(TINY_CHAR, 8, 98, " LIST");
	write(fd_oled,oled_val,sizeof(oled_val_t));
}




/**********************************************************************
 * �������ƣ� refresh_audio_info
 * ���������� ���µ�ǰ��Ƶid ������  ��Ƶ����   
 * ��������� plist�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_audio_info(list_t *plist){//
	u8 type,line;

	// 1 ��� 2 3 4 5 6��
	refresh_clean_2_to_7();
	// 2 ˢ�µ�ǰ�ǵڼ�����Ƶ(�ѿ���Ϊ��  plist����  )
	refresh_cur_count(plist);
	// 3 ˢ����Ƶ����(�ѿ���Ϊ�� plist���� )
	refresh_all_count(plist);
	// 4 ˢ���ļ�����(�ѿ���Ϊ�� plist���� )
	refresh_audio_name(plist);
	// 5 ��ʾ��ʼʱ��
	refresh_time(0);
}




/**********************************************************************
 * �������ƣ� init_ui
 * ���������� ��ʼ������
 * ��������� plist�ṹ��ָ��
 *            volume ����
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void init_ui(list_t *plist,int volume){
	u8 i,type;
	char temp_buffer[4];
	// 1 ��ʾ����״̬ ֹͣ�����š���ͣ  
	refresh_status();  
	// 2 ��ʾ��ǰ�ǵڼ�����Ƶ  
	refresh_cur_count(plist);
	// 3 ��ʾ�ָ���
	refresh_split();
	// 4 ��ʾ��Ƶ����
	refresh_all_count(plist);
	// 5 ��ʾ����ģʽ  ˳��  ���  ����ѭ��
	refresh_shift_mode();
	// 6 ��ʾ��ǰ����
	refresh_volume(volume);
	// 7 ��ʾ��ǰ��Ƶ����
	refresh_audio_name(plist);
	// 8 ��ʾ����ģʽ
	refresh_mode();
	// 9 ��ʾ�����б�
	refresh_list_mode();
	// 10 ��ʾ��ʼʱ��
	refresh_time(0);
}





/**********************************************************************
 * �������ƣ� refresh_prev_handle
 * ���������� ������ǰ�и�����н���   ����������Ƶ��Ϣ�͵�ǰ״̬
 * ��������� play_list   list_t�ṹ��ָ��
 *            record_list list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_prev_handle(list_t *play_list,list_t *record_list){
	if(flags->mode==0){        //����ģʽ��
		if(flags->list_mode==0)//�����б�ˢ����Ƶ��Ϣ
			refresh_audio_info(play_list);
		else
			refresh_audio_info(record_list);
		refresh_status();
	}
}
/**********************************************************************
 * �������ƣ� refresh_prev_handle
 * ���������� ��������и�����н���   ����������Ƶ��Ϣ�͵�ǰ״̬
 * ��������� play_list   list_t�ṹ��ָ��
 *            record_list list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_next_handle(list_t *play_list,list_t *record_list){
	if(flags->mode==0){        //����ģʽ��
		if(flags->list_mode==0)//�����б�ˢ����Ƶ��Ϣ
			refresh_audio_info(play_list);
		else
			refresh_audio_info(record_list);
		refresh_status();
	}
}
/**********************************************************************
 * �������ƣ� refresh_select_mode_handle
 * ���������� �����л�ģʽ�����н���   ������ǰ״̬  ��ǰ����ģʽ  ��ǰ��Ƶ��Ϣ  ��ǰ�б�ģʽ
 * ��������� play_list   list_t�ṹ��ָ��
 *            record_list list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_select_mode_handle(list_t *play_list,list_t *record_list){
	refresh_status();
	refresh_mode();             //����ģʽ��Ϣ
	if(flags->mode == 0){      //��¼��������ģʽ
		if(flags->list_mode == 0){  //�����б�
			refresh_audio_info(play_list);  
		}else{
			refresh_audio_info(record_list);
		}
		refresh_list_mode();
	}
}




/**********************************************************************
 * �������ƣ� refresh_refresselect_list_handle
 * ���������� �����л�ģʽ�����н���   ������ǰ״̬  ��ǰ�б�ģʽ  ��ǰ��Ƶ��Ϣ  ��ǰ����ģʽ
 * ��������� play_list   list_t�ṹ��ָ��
 *            record_list list_t�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern void refresh_refresselect_list_handle(list_t *play_list,list_t *record_list){
	refresh_status();            //����״̬
	refresh_list_mode();         //�����б�
	if(flags->list_mode == 0)    //������Ƶ��Ϣ
		refresh_audio_info(play_list);
	else
		refresh_audio_info(record_list);				
	if(flags->mode == 0)               //¼��ģʽ������ģʽ�������ģʽ��Ϣ
		refresh_mode();
}
