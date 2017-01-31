#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "base.h"
#include "file.h"
#include "player.h"
#include "handle.h"
/**********************************************************************
 * 函数名称： init_flags
 * 功能描述： 初始化flags
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void init_flags(){
	flags->is_allow    = 0;
	flags->is_continue = 0;
	flags->is_pause    = 0;
	flags->list_mode   = 0;
	flags->mode        = 0;
	flags->shift_mode  = 0;
	flags->curpos      = 0;
}





/**********************************************************************
 * 函数名称： get_list_type
 * 功能描述： 获取列表类型
 * 输入参数： plist   列表结构体指针
 * 输出参数： 无
 * 返 回 值： 0   播放列表
 *			  1   录音列表
 ***********************************************************************/
u8 get_list_type(list_t *plist){
	u8 type;
	if(strcmp(plist->name,"play")==0)
		type = 0;
	else 
		type = 1;
	return type;
}





/**********************************************************************
 * 函数名称： shuffer_handle
 * 功能描述： 根据随机数选出下一个要播放的音频
 * 输入参数： plist   列表结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void shuffer_handle(list_t *plist){  //根据随机数选取节点
	u8 type;
	u32 shuffle_num;
	type = get_list_type(plist);
	shuffle_num = get_shuffer_num();           
	shuffle_num = shuffle_num % (plist->size); //尽量保证随机不会重复到自己
	if(type==0){
		while(shuffle_num){
			play_node = play_node->next;       //通过循环shuffle_num次  随机改变下一个音频
			shuffle_num--;
		}
	}
	else{
		while(shuffle_num){
			record_node = record_node->next;
			shuffle_num--;
		}
	}
}





/**********************************************************************
 * 函数名称： shift_audio_handle
 * 功能描述： 随机模式处理
 * 输入参数： plist   列表结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void shift_audio_handle(list_t *plist){
	u8 type;
	type = get_list_type(plist);
	if(flags->shift_mode == 0){//顺序循环
		if(type==0)
			play_node = play_node->next;
		else
			record_node = record_node->next;
	}else if(flags->shift_mode == 1){//随机播放
		shuffer_handle(plist);
	}
	else if(flags->shift_mode == 2){ //单曲播放
		;
	}
}





/**********************************************************************
 * 函数名称： stop_handle
 * 功能描述： 停止处理  
 *            对应按键的K1，单次短按
 * 输入参数： pid   线程号
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void stop_handle(pthread_t *pid){
	pthread_kill(*pid, SIGCONT);       
	flags->is_allow    = 0;
	flags->is_continue = 0;
	flags->is_pause    = 0;
	printf("stop ok1\n");
	pthread_join(*pid,NULL);  //阻塞进程  直接线程结束  并回收线程的资源
	
	int kill_rc = pthread_kill(*pid,0);

	if(kill_rc == ESRCH)
	printf("the specified thread did not exists or already quit\n");
	else if(kill_rc == EINVAL)
	printf("signal is invalid\n");
	else
	printf("the specified thread is alive\n");
	printf("stop ok2\n");
	pthread_mutex_lock(&player_mutex); 
	flags->curpos = 0;
	pthread_mutex_unlock(&player_mutex);
	
}





/**********************************************************************
 * 函数名称： play_handle
 * 功能描述： 播放处理  用于播放音频
 * 输入参数： pid   线程号
 *            plist 列表结构体指针
 * 输出参数： 0  正常
 *           -1  失败
 * 返 回 值： 无
 ***********************************************************************/
static int play_handle(pthread_t *pid,list_t *plist){
	flags->is_continue = 1;
	flags->is_allow    = 1;
	pthread_mutex_lock(&player_mutex); 
	if(flags->is_pause == 0) //若之前未暂停  重置偏移量
		flags->curpos = 0;
	else{
		flags->is_pause = 0;
		flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;//为了保证计时的准确 如1:52:03  将变为1:52:00
	}
	pthread_mutex_unlock(&player_mutex);     //解锁
	if(pthread_create(pid,NULL,(void *)playing_thread,(void *)plist) != 0)  //创建播放线程
		return -1;
	return 0;
}





/**********************************************************************
 * 函数名称： record_handle
 * 功能描述： 录音处理   用于录制音频
 * 输入参数： pid   线程号
 *            plist 列表结构体指针
 * 输出参数： 0  正常
 *           -1  失败
 * 返 回 值： 无
 ***********************************************************************/
static int record_handle(pthread_t *pid,list_t *plist){
	flags->is_allow = 1;                                                       
	flags->is_pause = 0;  //取消暂停
	if(pthread_create(pid,NULL,(void *)recording_thread,(void *)plist) !=0)//创建录音线程   
		return -1;
	return 0;
}





/**********************************************************************
 * 函数名称： pause_handle
 * 功能描述： 暂停处理  用于暂停音频(其实质为停止，但是记录了文件偏移)
 * 输入参数： pid   线程号
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void pause_handle(pthread_t *pid){
	flags->is_pause    = 1; 
	flags->is_continue = 0;
	flags->is_allow    = 0;
	pthread_join(*pid,NULL);//阻塞进程  直接线程结束  并回收线程的资源
}





/**********************************************************************
 * 函数名称： resume_handle
 * 功能描述： 继续播放处理   
 *			  用于继续播放音频(其实质只是修正pause标志  继续播放还是由play_handle处理 )
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void resume_handle(){
	flags->is_pause    = 0; 
	flags->is_continue = 1;
	flags->is_allow    = 1;
}





/**********************************************************************
 * 函数名称： skip_handle
 * 功能描述： 跳过此次处理
 *            一般用于播放下一首操作中
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void skip_handle(){
	flags->is_continue = 0;
	flags->is_allow    = 1;
	pthread_mutex_lock(&player_mutex); 
	flags->curpos      = 0;
	pthread_mutex_unlock(&player_mutex);     //解锁
}




/**********************************************************************
 * 函数名称： play_pause_handle
 * 功能描述： 播放、录音、暂停的处理，供主函数调用
 *            对应按键的K2，单次短按
 * 输入参数： pid          进程号
 *            play_list    播放列表
 *            record_list  录音列表
 * 输出参数： 无
 * 返 回 值： 0  成功
             -1  失败
 ***********************************************************************/
int play_pause_handle(pthread_t *pid,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode == 0){                                               //播放模式
		if(flags->list_mode == 0){                                      //音乐列表
			if(play_list->size>0){                                      //非空列表
				if(flags->is_allow == 0){                               //开启播放
					ret = play_handle(pid,play_list);
					if(ret < 0)
						return ret;
					flags->is_pause == 0;
				}else{
					pause_handle(pid);
					DPRINTF("pause\n");
				}
			}
		}else{                                                           //录音列表
			DPRINTF("record_list=%d\n",record_list->size);
			if(record_list->size > 0){                                   //非空列表
				if(flags->is_allow == 0){                                //开启播放 
					ret = play_handle(pid,record_list); 
					if(ret < 0)
						return ret;
					flags->is_pause == 0;
				}else{
					if(flags->is_pause == 0){                            //暂停
						pause_handle(pid);
						DPRINTF("pause\n");
					}
				}
			}
		}
	}
	else{                                                              //录音模式
		if(flags->is_allow == 0){                                      //开启录音
			ret = record_handle(pid,record_list);
			if(ret < 0)
				return ret;
		}else{
			if(flags->is_pause == 0){                                //暂停
				flags->is_pause = 1;                               
				pthread_kill(*pid, SIGTSTP);                         //发送暂停进程信号
				DPRINTF("pause\n"); 
			}else{                                                   //继续
				flags->is_pause = 0;	                                                  
				pthread_kill(*pid, SIGCONT);       
				DPRINTF("continue\n");		
			}			
		}
	}
	return 0;
} 




/**********************************************************************
 * 函数名称： prev_handle
 * 功能描述： 播放上一首，不管播放顺序如何，只向上切换，供主函数调用
 *            对应按键的K3，单次短按
 * 输入参数： pid          进程号
 *            play_list    播放列表
 *            record_list  录音列表
 * 输出参数： 无
 * 返 回 值： 0  成功
             -1  失败
 ***********************************************************************/
int prev_handle(pthread_t *pid,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode == 0){
		if(flags->list_mode == 0){                             //音乐列表
			if(play_list->size > 0){                           //非空列表
				if(flags->is_pause == 1){                     //暂停时切歌 
					stop_handle(pid);
					play_node = play_node->prev;
					ret = play_handle(pid,play_list); 
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                //停止状态下上翻
						play_node = play_node->prev;
					}else{                                   //播放状态下切歌
						pause_handle(pid);
						if(flags->curpos > TEN_SEC){	     //播放时间大于10秒，重新播放当前音频
							flags->is_pause = 0;
							ret = play_handle(pid,play_list);
							if(ret < 0)
								return ret;
						}else{                               //播放时间小于10秒，向上切歌
							play_node = play_node->prev;
							flags->is_pause = 0;
							ret = play_handle(pid,play_list);
							if(ret < 0)
								return ret;
						}
					}
				}
			}
		}
		else{     //录音列表          
			if(record_list->size > 0){
				if(flags->is_pause == 1){                 //暂停时切歌
					stop_handle(pid);
					record_node = record_node->prev;
					ret = play_handle(pid,record_list);
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                    //停止状态下上翻
						record_node = record_node->prev;
						printf("now playing: %s\n",record_node->gbk_name);			
					}else{
						pause_handle(pid);
						if(flags->curpos > TEN_SEC){	
							flags->is_pause = 0;
							ret = play_handle(pid,record_list);
							if(ret < 0)
								return ret;
						}else{
							record_node = record_node->prev;
							flags->is_pause = 0;
							ret = play_handle(pid,record_list);
							if(ret < 0)
								return ret;
						}
					}	
				}
			}
		}
	}
	return 0;
}




/**********************************************************************
 * 函数名称： next_handle
 * 功能描述： 播放下一首，若为随机播放模式，随机播放下一个音频，供主函数调用
 *            对应按键的K4，单次短按
 * 输入参数： pid          进程号
 *            play_list    播放列表
 *            record_list  录音列表
 * 输出参数： 无
 * 返 回 值： 0  成功
             -1  失败
 ***********************************************************************/
int next_handle(pthread_t *pid, list_t *play_list, list_t *record_list){
	int ret;
	if(flags->mode == 0){
		if(flags->list_mode == 0){                           //音乐列表
			if(play_list->size > 0){                         //非空列表
				if(flags->is_pause == 1){                   //暂停时切歌
		 			stop_handle(pid);                       
					if(flags->shift_mode == 1){              //随机播放模式   随机选歌
						shift_audio_handle(play_list);
					}
					else{
						play_node = play_node->next;	
					}
					ret = play_handle(pid,play_list); 
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                    //停止状态下翻
						play_node = play_node->next;
					}else{                                      //播放时切歌
						skip_handle();
						if(flags->shift_mode == 1)             //随机播放模式   随机选歌
							shift_audio_handle(play_list);
						else{
							play_node=play_node->next;
						}
					}
				}
			}
		}
		else{                       //录音列表
			if(record_list->size > 0){
				if(flags->is_pause == 1){                  //暂停时切歌  
		 			stop_handle(pid);		
					if(flags->shift_mode == 1)
						shift_audio_handle(record_list);
					else
						record_node = record_node->next;	 
					ret = play_handle(pid,record_list); 
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                    //停止状态下上翻
						record_node = record_node->next;
					}else{
						skip_handle();
						if(flags->shift_mode == 1)
							shift_audio_handle(record_list);
						else
							record_node = record_node->next;
					}
				}
			}
		}
	}
	return 0;
}




/**********************************************************************
 * 函数名称： backward_handle
 * 功能描述： 快退，供主函数调用
 *            对应按键的K3，长按
 * 输入参数： pid          进程号
 *            offset       长按时间长度
 *            play_list    播放列表
 *            record_list  录音列表
 * 输出参数： 无
 * 返 回 值： 0  成功
             -1  失败
 ***********************************************************************/
int backward_handle(pthread_t *pid,off_t offset,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode==0){                                            //播放模式
		if(flags->list_mode == 0){                                //音乐列表
			if(flags->is_allow == 1){                            //播放状态下
				pause_handle(pid);                              //暂停  保存
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos-offset*5000;      //长按1秒 大约后退5秒
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;  //取整到秒
				pthread_mutex_unlock(&player_mutex);     //解锁
				ret = play_handle(pid,play_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}else{
			if(flags->is_allow == 1){             //播放状态下
				pause_handle(pid);
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos-offset*5000;
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;
				pthread_mutex_unlock(&player_mutex);     //解锁
				ret = play_handle(pid,record_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}
	}
}

/**********************************************************************
 * 函数名称： backward_handle
 * 功能描述： 快退，供主函数调用
 *            对应按键的K4，长按
 * 输入参数： pid          进程号
 *            offset       长按时间长度
 *            play_list    播放列表
 *            record_list  录音列表
 * 输出参数： 无
 * 返 回 值： 0  成功
             -1  失败
 ***********************************************************************/
int forward_handle(pthread_t *pid,off_t offset,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode==0){                            //播放模式
		if(flags->list_mode == 0){                 //音乐列表
			if(flags->is_allow == 1){             //播放状态下
				pause_handle(pid);
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos+offset*5000;      //长按1秒 大约后退5秒
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;
				pthread_mutex_unlock(&player_mutex);     //解锁
				ret = play_handle(pid,play_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}else{
			if(flags->is_allow == 1){             //播放状态下
				pause_handle(pid);
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos+offset*5000;
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;
				pthread_mutex_unlock(&player_mutex);     //解锁
				ret = play_handle(pid,record_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}
	}
}
int devolume_handle(int fd_mix,int volume){
	int ret;
	if(flags->mode==0){                       //播放模式
		if(volume>=63){
			printf("volume min\n");
			return volume;
		}
		volume = volume+3;
		ret = ioctl(fd_mix,VOLUME_UP_DOWN,&volume);
		if(ret < 0)
			return ret;
	}
	return volume;
}
int upvolume_handle(int fd_mix,int volume){
	int ret;
	if(flags->mode==0){                       //播放模式
		if(volume<=0){
			printf("volume max\n");
			return volume;
		}
		volume = volume-3;
		ret = ioctl(fd_mix,VOLUME_UP_DOWN,&volume);		
		if(ret < 0)
			return ret;
	}
	return volume;
}
void select_mode_handle(pthread_t *pid){
	stop_handle(pid);
	flags->mode = !(flags->mode);
	if(flags->mode == 0)
		printf("play mode\n");
	else 
		printf("record mode\n");
}
void select_list_handle(pthread_t *pid,list_t *play_list,list_t *record_list){
	int ret;
	stop_handle(pid);
	if(flags->mode == 0){                     //播放模式 
		if(flags->list_mode == 0){            //音乐列表
			flags->list_mode = 1;            //进入录音列表
			printf("play record list\n");
			print_list(record_list);   
		}else{                                //录音列表
			flags->list_mode = 0; 
			printf("play music list\n");
			print_list(play_list);	
		}
	}else{                                       //录音模式进入播放模式的录音列表
		flags->mode = 0;                         //播放模式     
		flags->list_mode = 1;                    //刷新录音列表
		printf("play record list\n");
		printf("record_list->size=%d\n",record_list->size);
		print_list(record_list);
	}
}
void shift_mode_handle(){
	if(flags->shift_mode == 2)
		flags->shift_mode = 0;
	else
		flags->shift_mode += 1;
}

