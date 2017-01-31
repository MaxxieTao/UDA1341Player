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
#include "rtc.h"
#include "file.h"
#include "player.h"

/**********************************************************************
 * 函数名称： recording_thread
 * 功能描述： 录制音频的线程
 * 输入参数： play_node指针,
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern int recording_thread(list_t *plist){
	char temp_name[32];
	char combine_name[256];
	char gbk_name[256];
	int fd_dsp;
	FILE *wav; 
	u32 loop_count=0;
	u8 one_sec_count=0;

	flags->is_allow=1;//1-正常录制   0-结束录制
	signal(SIGCONT,SIG_DFL);//信号绑定编号处理函数
	signal(SIGTSTP,SIG_DFL);
	memset(temp_name,'\0',sizeof(temp_name));
	memset(combine_name,'\0',sizeof(combine_name));
	memset(gbk_name,'\0',sizeof(gbk_name));
	memset(audio_buffer,0,sizeof(audio_buffer));

	snprintf(temp_name,sizeof(temp_name),"%s",get_rtc_time());
	snprintf(combine_name,sizeof(combine_name),"%s%s","/player/record_list/",temp_name);//拼接字符串，绝对路径+文件名
	wav=fopen(combine_name,"w"); 
	printf("temp_name=%s\n",temp_name);
	refresh_record_name(temp_name);
	fd_dsp = open("/dev/dsp", O_RDONLY); 
	if (fd_dsp < 0){
		printf("can't open\n");
		return 0;
	}
	while(flags->is_allow){
		read(fd_dsp,audio_buffer,sizeof(audio_buffer)); //录制音频文件		
		fwrite(audio_buffer,sizeof(audio_buffer),1,wav);//将文件流写入缓冲区  		
		loop_count++;
		if(one_sec_count == 9){//取9的原因是因为  大约每10秒会延迟1秒
			//printf("loop_count=%d\n\n",loop_count);
			refresh_time(loop_count);
			one_sec_count = 0;
		}
		else
			one_sec_count++;
	}
	fclose(wav);
	close(fd_dsp);
	utf8_to_gbk(temp_name,strlen(temp_name), gbk_name, sizeof(gbk_name));
	record_node = add_node(plist, temp_name, gbk_name,1);
	//record_node = add_node(plist, temp_name, temp_name,1);
	plist->front = record_node;
	pthread_exit(NULL);//结束线程
}





/**********************************************************************
 * 函数名称： playing_thread
 * 功能描述： 播放音乐的线程
 * 输入参数： play_node指针,
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void playing_thread(list_t *plist){
	char temp_name[256];
	int fd_dsp=0;
	FILE *wav; 
	off_t curpos;
	u32 shuffle_num,loop_count=0;
	u8 type,one_sec_count=0;
	
	//memset(audio_buffer,0,sizeof(audio_buffer));

	type = get_list_type(plist);
	while(flags->is_allow){      //结束后自动播放下一首 

		flags->is_continue = 1;//播放flag  1为正常播放  0结束播放
		curpos = 0;
		loop_count = 0;
		one_sec_count = 0;
		memset(temp_name,'\0',sizeof(temp_name));
		if(type == 0){
			sprintf(temp_name,"%s%s","/player/play_list/",play_node->file_name); //拼接字符串，绝对路径+文件名
			printf("now playing: %s\n",play_node->gbk_name);
		}
		else{
			sprintf(temp_name,"%s%s","/player/record_list/",record_node->file_name); 
			printf("now playing: %s\n",record_node->gbk_name);
		}
		
		wav = NULL;
		//printf("1.flags->curpos=%d\n",flags->curpos);
		if((wav = fopen(temp_name,"r")) == NULL){            //处理文件丢失问题
			printf("the audio file is missing\n");
			error_missing_the_file();
			pthread_exit(NULL);
		}
		fd_dsp = open("/dev/dsp", O_WRONLY);        //打开声卡驱动，O_WRONLY播放模式
		if(fd_dsp < 0){
			printf("can not open dsp!\n");
			pthread_exit(NULL);
		}	
		else{
			fseek(wav,flags->curpos, SEEK_SET);
			if(flags->curpos>=0)
				loop_count = flags->curpos / ONE_TENTH_SEC;
			else
				loop_count = 0;
			while(flags->is_continue){
				if(!feof(wav)){
					fread(audio_buffer,sizeof(audio_buffer),1,wav);//将文件流读入缓冲区  
					write(fd_dsp,audio_buffer,sizeof(audio_buffer));   //播放音频文件
					loop_count++;
					if(one_sec_count == 9){//取9的原因是因为  大约每1秒会延迟0.1秒
						//printf("loop_count=%d\n\n",loop_count);
						refresh_time(loop_count);
						one_sec_count = 0;
					}
					else
						one_sec_count++;
				}else{    //正常结束  
					shift_audio_handle(plist);
					refresh_audio_info(plist);
					
					pthread_mutex_lock(&player_mutex); 
					flags->curpos  = 0;
					pthread_mutex_unlock(&player_mutex);     //解锁
					//printf("normal ending\n");
					
					break;
				}
			}
			           
			if(flags->is_pause == 1){
				//printf("cutting\n");       //切歌 
				pthread_mutex_lock(&player_mutex); 
				flags->curpos  = ftell(wav);
				pthread_mutex_unlock(&player_mutex);     //解锁
	
			}
			fclose(wav);
			//printf("2.flags->curpos=%d\n",flags->curpos);			
		}
		close(fd_dsp);
	}
	//close(fd_dsp);	

	pthread_exit(NULL);//结束线程
}

/* 2016.3.20开始  
 * bug修复:
 * 显示当前音乐速率稍慢                        --部分由线程显示                          完成
 * 控制不能自动播放下一首                      --修改线程逻辑                            完成
 * 暂停时切歌应该直接播放切到的曲目            --修改逻辑                                完成
 * 不能正确切歌                                --修改逻辑                                完成
 * 只有在播放模式才能调整录音                  --修改逻辑                                完成
 * 录音模式切到播放录音列表  不能正常播放      --修改逻辑                                完成
 * 重启程序但是未重装驱动音乐声音大小不吻合    --重启程序设置默认大小  控制最大最小音量  完成
 * 反复暂停之后有杂音                          --暂停由暂停线程改为记录位置  停止线程
 * 如果一开始是空目录  点击播放将退出程序
 * 如果扫描完毕列表后全部清空  将在线程中
   不停循环  线程不知道何时跳出
 * 播放中的歌曲 突然被删除  将无法正常播放
   需要手动点击停止
 * 随机播放处理
 * 切换列表和模式刷新界面
 
 # 功能增加:
 # 增加出错处理                                --修改框架 退出while(1)自动重启           完成	
 # 播放录音显示列表                            --修改逻辑                                完成
 # 从音乐播放列表转换到录音列表                --修改逻辑                                完成
 # 两个列表分别控制上一首一下首                --修改逻辑                                完成
 # 快进快退                                    --长按按键由驱动返回长按时间              完成
 # 根据文件类型扫描音乐
 # 歌曲播放10秒以上 第一次向上切歌向重播这首歌  
   10秒以内再切才会向上切歌                    --根据文件偏移计算时间                    完成
 # 播放前  歌曲被删除  可以自动跳过歌曲        --打开前文件是否存在判断                  完成
 */

