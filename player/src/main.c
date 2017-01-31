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
	pthread_t pid; //线程号
	list_t *play_list = NULL;
	list_t *record_list = NULL;
	srand( (unsigned)time( NULL )); 

	//初始化button_val
	button_val[0].key=0;
	button_val[0].time=0;

	//初始化互斥锁
	ret = pthread_mutex_init(&player_mutex, NULL); //init mutex 
	if (ret != 0){
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
	
	//初始化flags
	flags = (flag_t *)malloc(sizeof(flag_t));
	init_flags(flags);

	//初始化oled_val
	oled_val = (oled_val_t *)malloc(sizeof(oled_val_t));
	init_oled_type_data(oled_val);

	//打开显示屏驱动
	fd_oled = open("/dev/oled",O_RDWR); 
	if(fd_oled < 0){
		DPRINTF("can not open oled!\n");
		return -1;
	}	

	//打开音频控制驱动
	fd_mix = open("/dev/mix",O_RDWR); 
	if(fd_mix < 0){
		DPRINTF("can not open mix!\n");
		error_missing_the_mix();
		sleep(10);
		close(fd_oled);
		return -1;
	}
	
	//打开按键驱动
	fd_btn = open("/dev/btn",O_RDWR); 
	if(fd_btn < 0){
		DPRINTF("can not open btn!\n");
		error_missing_the_btn();
		sleep(10);
		close(fd_mix);
		close(fd_oled);
		return -1;
	}	
	
	ioctl(fd_mix,VOLUME_UP_DOWN,&volume);      //设置默认音量
	ioctl(fd_oled,CLEAN_ALL,NULL);
	
	//创建播放链表
	play_list = init_list();
	if(play_list == NULL){
		DPRINTF("play_list error");
		error_init_list();
		sleep(10);
		return -1;
	}
	
	//创建录音链表
	record_list = init_list();
	if(record_list == NULL){
		DPRINTF("record_list error");
		error_init_list();
		sleep(10);
		return -1;
	}
	//将play_list下文件添加到播放列表中
	ret = creat_file_list(play_list,"play",0);
	//将record_list下文件添加到录音列表中
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
		read(fd_btn,button_val,sizeof(button_val));        //阻塞  读取键值
		choose       = button_val[0].key;
		diff_jiffies = button_val[0].time;  //1s = 200jiff
		printf("choose:%hhu\n",choose);
		printf("time:%d\n",diff_jiffies);
		switch(choose){
			case 1:  //停止
				if(diff_jiffies < 200){
					stop_handle(&pid);
					refresh_status();
				}
				else{
					shift_mode_handle();
					refresh_shift_mode();
				}
			break;
			case 2:  //播放/暂停
				ret = play_pause_handle(&pid,play_list,record_list);
				refresh_status();
			break;
			case 3:  //上一首
				if(diff_jiffies<40){
					ret = prev_handle(&pid,play_list,record_list);
					refresh_prev_handle(play_list,record_list);
				}
				else
					backward_handle(&pid, diff_jiffies,play_list,record_list);
			break;
			case 4:  //下一首
				if(diff_jiffies<40){
					ret = next_handle(&pid,play_list,record_list);
					refresh_next_handle(play_list,record_list);
				}
				else
					forward_handle(&pid, diff_jiffies,play_list,record_list);
			break;
			case 5:	  //降低音量   volume增加 音量降低(驱动原因)
				old_volume = volume;
				volume = devolume_handle(fd_mix,volume);
				if(volume/10 > old_volume/10)   //避免不必要的更新
					refresh_volume(volume);
			break;
			case 6:   //增加音量  volume降低 音量增加(驱动原因)
				old_volume = volume;
				volume = upvolume_handle(fd_mix,volume);
				if(volume/10 < old_volume/10) //避免不必要的更新
					refresh_volume(volume);
			break;
			case 7:   //模式切换
				select_mode_handle(&pid);
				refresh_select_mode_handle(play_list, record_list);
			break;
			case 8:   //列表切换与刷新
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

