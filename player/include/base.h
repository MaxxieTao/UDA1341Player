#ifndef _BASE_H
#define _BASE_H

#define DEBUG
#ifdef DEBUG
#define DPRINTF printf
#else
#define DPRINTF( x... )

#endif
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef struct flag{
    u8    is_continue;    //�Ƿ���������Ƶ 
	u8    is_pause;       //�Ƿ�Ϊ��ͣ״̬ 
	u8    is_allow;      //�������һ����Ƶ�Ƿ�������� 
	u8    mode;          //ģʽ   0-����   1-¼��
	u8    list_mode;     //0-�����б�   1-¼���б�   2-¼����
	char  shift_mode; //0-˳��ѭ������  1-�������   2-����ѭ��
	u32   play_id;      
	off_t curpos;     //�ļ�ƫ��  ���ڻָ���ͣ���ļ��Ϳ������
}flag_t;
#endif  //_BASE_H

