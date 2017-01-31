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
#include "file.h"
#include "handle.h"
#include "ui.h"
#include "main.h"

int main(){    
	int ret,fd_btn,fd_mix,i;
	unsigned char choose;
	char temp_buffer[4];
	int volume = 30;
	off_t audio_offset = 0;
	u32 diff_jiffies = 0;
	button_val_t button_val[1];
	u8 old_volume; 
	pthread_t pid; //�̺߳�
	list_t *play_list = NULL;
	list_t *record_list = NULL;
	srand( (unsigned)time( NULL )); 

	//��ʼ��button_val
	button_val[0].key=0;
	button_val[0].time=0;

	//��ʼ��������
	ret = pthread_mutex_init(&player_mutex, NULL); //init mutex 
	if (ret != 0){
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
	
	//��ʼ��flags
	flags = (flag_t *)malloc(sizeof(flag_t));
	init_flags(flags);

	//��ʼ��oled_val
	oled_val = (oled_val_t *)malloc(sizeof(oled_val_t));
	init_oled_type_data(oled_val);

	//����ʾ������
	fd_oled = open("/dev/oled",O_RDWR); 
	if(fd_oled < 0){
		DPRINTF("can not open oled!\n");
		return -1;
	}	

	//����Ƶ��������
	fd_mix = open("/dev/mix",O_RDWR); 
	if(fd_mix < 0){
		DPRINTF("can not open mix!\n");
		error_missing_the_mix();
		sleep(10);
		close(fd_oled);
		return -1;
	}
	
	//�򿪰�������
	fd_btn = open("/dev/btn",O_RDWR); 
	if(fd_btn < 0){
		DPRINTF("can not open btn!\n");
		error_missing_the_btn();
		sleep(10);
		close(fd_mix);
		close(fd_oled);
		return -1;
	}	
	
	ioctl(fd_mix,VOLUME_UP_DOWN,&volume);      //����Ĭ������
	ioctl(fd_oled,CLEAN_ALL,NULL);
	
	//������������
	play_list = init_list();
	if(play_list == NULL){
		DPRINTF("play_list error");
		error_init_list();
		sleep(10);
		return -1;
	}
	
	//����¼������
	record_list = init_list();
	if(record_list == NULL){
		DPRINTF("record_list error");
		error_init_list();
		sleep(10);
		return -1;
	}
	//��play_list���ļ���ӵ������б���
	ret = creat_file_list(play_list,"play",0);
	//��record_list���ļ���ӵ�¼���б���
	ret = creat_file_list(record_list,"record",1);
	if(ret < 0){
		error_create_list();
		sleep(10);
		return -1;
	}
	
	play_node = play_list->front;
	record_node = record_list->front;
	
	print_list(play_list);
	init_ui(play_list,volume);
	while( ret >= 0 ){
		read(fd_btn,button_val,sizeof(button_val));        //����  ��ȡ��ֵ
		choose       = button_val[0].key;
		diff_jiffies = button_val[0].time;  //1s = 200jiff
		printf("choose:%hhu\n",choose);
		printf("time:%d\n",diff_jiffies);
		switch(choose){
			case 1:  //ֹͣ
				if(diff_jiffies < 200){
					stop_handle(&pid);
					refresh_status();
				}
				else{
					shift_mode_handle();
					refresh_shift_mode();
				}
			break;
			case 2:  //����/��ͣ
				ret = play_pause_handle(&pid,play_list,record_list);
				refresh_status();
			break;
			case 3:  //��һ��
				if(diff_jiffies<40){
					ret = prev_handle(&pid,play_list,record_list);
					refresh_prev_handle(play_list,record_list);
				}
				else
					backward_handle(&pid, diff_jiffies,play_list,record_list);
			break;
			case 4:  //��һ��
				if(diff_jiffies<40){
					ret = next_handle(&pid,play_list,record_list);
					refresh_next_handle(play_list,record_list);
				}
				else
					forward_handle(&pid, diff_jiffies,play_list,record_list);
			break;
			case 5:	  //��������   volume���� ��������(����ԭ��)
				old_volume = volume;
				volume = devolume_handle(fd_mix,volume);
				if(volume/10 > old_volume/10)   //���ⲻ��Ҫ�ĸ���
					refresh_volume(volume);
			break;
			case 6:   //��������  volume���� ��������(����ԭ��)
				old_volume = volume;
				volume = upvolume_handle(fd_mix,volume);
				if(volume/10 < old_volume/10) //���ⲻ��Ҫ�ĸ���
					refresh_volume(volume);
			break;
			case 7:   //ģʽ�л�
				select_mode_handle(&pid);
				refresh_select_mode_handle(play_list, record_list);
			break;
			case 8:   //�б��л���ˢ��
				select_list_handle(&pid,play_list, record_list);
				refresh_refresselect_list_handle(play_list, record_list);
			break;
			default:
			break;
		}
	}	
	free_list(play_list);
	free_list(record_list);
	close(fd_mix);
	close(fd_btn);
	error_other();
	sleep(10);
	close(fd_oled);
	system("reboot");
	printf("BYE\n");
    return 0;
}

