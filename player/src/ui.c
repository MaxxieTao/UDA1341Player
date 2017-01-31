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
 * 函数名称： error_missing_the_file
 * 功能描述： 显示文件丢失提示
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void error_missing_the_file(){           
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 36, "文件丢失");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 8, "请手动重启设备");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： error_missing_the_btn
 * 功能描述： 显示丢失按键驱动提示
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void error_missing_the_btn(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "按键驱动丢失");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 8, "移交技术员处理");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： error_missing_the_mix
 * 功能描述： 显示丢失音频驱动提示
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void error_missing_the_mix(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "音频驱动丢失");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 8, "移交技术员处理");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： error_init_list
 * 功能描述： 显示创建列表错误提示
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void error_init_list(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "创建列表错误");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 24, "10秒后重启");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： error_create_list
 * 功能描述： 显示加载列表错误提示
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void error_create_list(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 16, "加载列表错误");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 24, "10秒后重启");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： error_other
 * 功能描述： 显示其他错误提示
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void error_other(){          
	refresh_clean_2_to_7();
	oled_val = package_oled_val(BIG_CHAR, 2, 36, "系统异常");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
	oled_val = package_oled_val(BIG_CHAR, 4, 24, "10秒后重启");
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}
/**********************************************************************
 * 函数名称： init_oled_type_data
 * 功能描述： 初始化oled数据
 * 输入参数： oled_val_t结构体指针,
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void init_oled_type_data(oled_val_t *p){
	memset(p->text,0,32);
	p->page = 0;
	p->col  = 0;
	p->type = 0;
}





/**********************************************************************
 * 函数名称： package_oled_val
 * 功能描述： 打包oled数据
 * 输入参数： type显示类型
 *            page显示行
 *            col 显示列
 *            text显示的文本
 * 输出参数： 无
 * 返 回 值： oled_val_t结构体指针
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
 * 函数名称： refresh_status
 * 功能描述： 更新播放状态    停止  暂停  播放图标
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_status(){//停止 暂停  播放
	if(flags->is_allow == 0){      //停止状态
		if(flags->is_pause == 1)   //暂停模式
			oled_val = package_oled_val(GRAPHIC, 0, 1, pause_graphy);
		else
			oled_val = package_oled_val(GRAPHIC, 0, 1, stop_graphy);
	}else{                          //播放状态
		if(flags->is_pause == 1)   //暂停模式
			oled_val = package_oled_val(GRAPHIC, 0, 1, pause_graphy);
		else
			oled_val = package_oled_val(GRAPHIC, 0, 1, play_graphy);
		
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * 函数名称： refresh_cur_count
 * 功能描述： 更新当前播放是第几个音频    停止  暂停  播放图标
 * 输入参数： list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_cur_count(list_t *plist){ 
	u8 type;
	char temp_buffer[4];
	type = get_list_type(plist);
	if(plist->size > 0){
		if(type == 0){  //播放列表
			snprintf(temp_buffer,sizeof(temp_buffer),"%03d",play_node->id);  
			oled_val = package_oled_val(BIG_CHAR, 0, 23, temp_buffer);

		}else{          //录音列表
			snprintf(temp_buffer,sizeof(temp_buffer),"%03d",record_node->id);  
			oled_val = package_oled_val(BIG_CHAR, 0, 23, temp_buffer);
		}
	}else{                //列表为空
		oled_val = package_oled_val(BIG_CHAR, 0, 23, "000");//音频数量为0
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * 函数名称： refresh_split
 * 功能描述： 显示分隔符图标
 * 输入参数： list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_split(){            //分隔符
	oled_val = package_oled_val(BIG_CHAR, 0, 47, "/");
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * 函数名称： refresh_clean_2_to_7
 * 功能描述： 清除第2到第7行
 * 输入参数： list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_clean_2_to_7(){
	u8 line;
	for(line = 2;line < 7;line++)
		ioctl(fd_oled,CLEAN_N,&line); 
}





/**********************************************************************
 * 函数名称： refresh_time_min
 * 功能描述： 更新播放时间分钟数
 * 输入参数： 分钟
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_time_min(u32 min){
	u8 min_buffer[4];
	snprintf(min_buffer,sizeof(min_buffer),"%02d",min);  
	oled_val = package_oled_val(BIG_CHAR, 4, 0, min_buffer);	
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * 函数名称： refresh_time_colon
 * 功能描述： 更新冒号图标
 * 输入参数： 分钟
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_time_colon(){            //冒号
	oled_val = package_oled_val(BIG_CHAR, 4, 16, ":");
	write(fd_oled,oled_val,sizeof(oled_val_t));
}




/**********************************************************************
 * 函数名称： refresh_time_sec
 * 功能描述： 更新播放时间秒钟数
 * 输入参数： 秒数
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_time_sec(u32 sec){
	u8 sec_buffer[4];
	snprintf(sec_buffer,sizeof(sec_buffer),"%02d",sec);  
	oled_val = package_oled_val(BIG_CHAR, 4, 24, sec_buffer);	
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * 函数名称： refresh_time
 * 功能描述： 更新播放时间  根据进入循环的次数  计算时间
 * 输入参数： 循环次数
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_time(u32 count){// 1 sec = 10 count
	u32 min;
	u32 sec;
	u32 total_sec;
	total_sec = count/10;  //计算一共多少秒
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
 * 函数名称： refresh_all_count
 * 功能描述： 更新音频总数
 * 输入参数： plist结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_all_count(list_t *plist){  //音频总数
	char temp_buffer[4];
	if(plist->size > 0){
		snprintf(temp_buffer,sizeof(temp_buffer),"%03d",plist->size);  
		oled_val = package_oled_val(BIG_CHAR, 0, 55, temp_buffer);	
	}else{                //列表为空
		oled_val = package_oled_val(BIG_CHAR, 0, 55, "000");//音频数量为0
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}





/**********************************************************************
 * 函数名称： refresh_shift_mode
 * 功能描述： 更新播放顺序图标    顺序  随机  单曲循环
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
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
 * 函数名称： refresh_volume
 * 功能描述： 更新音量图标
 * 输入参数： volume音量
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_volume(u8 volume){
	if(volume < 10){  //声音最大
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
	}else if(volume >=60 && volume < 70){  //声音最小
		oled_val = package_oled_val(GRAPHIC, 0, 110, volume1_graphy);
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));
}






/**********************************************************************
 * 函数名称： refresh_audio_name
 * 功能描述： 更新音频名称
 * 输入参数： plist结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void refresh_audio_name(list_t *plist){
	u8 type;
	char temp_buffer[4];
	type = get_list_type(plist);
	if(plist->size > 0){
		if(type == 0){  //播放列表
			oled_val = package_oled_val(BIG_CHAR, 2, 0, play_node->gbk_name);	
		}else{          //录音列表
			oled_val = package_oled_val(BIG_CHAR, 2, 0, record_node->gbk_name);
		}
	}else{                //列表为空
		oled_val = package_oled_val(BIG_CHAR, 3, 50, "NO LIST");
	}
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： refresh_record_name
 * 功能描述： 显示录音时的文件名
 * 输入参数： text文件名
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_record_name(const u8 *text){
	oled_val = package_oled_val(BIG_CHAR, 2, 0,text);	
	write(fd_oled,oled_val,sizeof(oled_val_t));	
}





/**********************************************************************
 * 函数名称： refresh_mode
 * 功能描述： 更新播放模式    播放   录音
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_mode(){
	int line;
	if(flags->mode==0){ //播放模式
		oled_val = package_oled_val(TINY_CHAR, 8, 1, "P");         
		write(fd_oled,oled_val,sizeof(oled_val_t));
		oled_val = package_oled_val(TINY_CHAR, 8, 6, " MODE");
		write(fd_oled,oled_val,sizeof(oled_val_t));
	}else{               //录音模式
		line = 7;
		ioctl(fd_oled,CLEAN_N,&line);
		oled_val = package_oled_val(BIG_CHAR, 0, 23, "000");
		write(fd_oled,oled_val,sizeof(oled_val_t));
		oled_val = package_oled_val(BIG_CHAR, 0, 55, "000");//录音模式下  显示总数为0
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
 * 函数名称： refresh_list_mode
 * 功能描述： 更新播放列表    音乐列表   录音列表
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_list_mode(){
	if(flags->list_mode==0){ //音乐列表
		oled_val = package_oled_val(TINY_CHAR, 8, 93, "M");
		write(fd_oled,oled_val,sizeof(oled_val_t));
	}else{                  //录音列表
		oled_val = package_oled_val(TINY_CHAR, 8, 93, "R");
		write(fd_oled,oled_val,sizeof(oled_val_t));
	}
	oled_val = package_oled_val(TINY_CHAR, 8, 98, " LIST");
	write(fd_oled,oled_val,sizeof(oled_val_t));
}




/**********************************************************************
 * 函数名称： refresh_audio_info
 * 功能描述： 更新当前音频id 总数量  音频名称   
 * 输入参数： plist结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_audio_info(list_t *plist){//
	u8 type,line;

	// 1 清空 2 3 4 5 6行
	refresh_clean_2_to_7();
	// 2 刷新当前是第几个音频(已考虑为空  plist类型  )
	refresh_cur_count(plist);
	// 3 刷新音频总数(已考虑为空 plist类型 )
	refresh_all_count(plist);
	// 4 刷新文件名称(已考虑为空 plist类型 )
	refresh_audio_name(plist);
	// 5 显示初始时间
	refresh_time(0);
}




/**********************************************************************
 * 函数名称： init_ui
 * 功能描述： 初始化界面
 * 输入参数： plist结构体指针
 *            volume 音量
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void init_ui(list_t *plist,int volume){
	u8 i,type;
	char temp_buffer[4];
	// 1 显示播放状态 停止、播放、暂停  
	refresh_status();  
	// 2 显示当前是第几个音频  
	refresh_cur_count(plist);
	// 3 显示分隔符
	refresh_split();
	// 4 显示音频总数
	refresh_all_count(plist);
	// 5 显示播放模式  顺序  随机  单曲循环
	refresh_shift_mode();
	// 6 显示当前音量
	refresh_volume(volume);
	// 7 显示当前音频名称
	refresh_audio_name(plist);
	// 8 显示播放模式
	refresh_mode();
	// 9 显示播放列表
	refresh_list_mode();
	// 10 显示初始时间
	refresh_time(0);
}





/**********************************************************************
 * 函数名称： refresh_prev_handle
 * 功能描述： 更新向前切歌的所有界面   包括更新音频信息和当前状态
 * 输入参数： play_list   list_t结构体指针
 *            record_list list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_prev_handle(list_t *play_list,list_t *record_list){
	if(flags->mode==0){        //播放模式下
		if(flags->list_mode==0)//根据列表刷新音频信息
			refresh_audio_info(play_list);
		else
			refresh_audio_info(record_list);
		refresh_status();
	}
}
/**********************************************************************
 * 函数名称： refresh_prev_handle
 * 功能描述： 更新向后切歌的所有界面   包括更新音频信息和当前状态
 * 输入参数： play_list   list_t结构体指针
 *            record_list list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_next_handle(list_t *play_list,list_t *record_list){
	if(flags->mode==0){        //播放模式下
		if(flags->list_mode==0)//根据列表刷新音频信息
			refresh_audio_info(play_list);
		else
			refresh_audio_info(record_list);
		refresh_status();
	}
}
/**********************************************************************
 * 函数名称： refresh_select_mode_handle
 * 功能描述： 更新切换模式后所有界面   包括当前状态  当前播放模式  当前音频信息  当前列表模式
 * 输入参数： play_list   list_t结构体指针
 *            record_list list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_select_mode_handle(list_t *play_list,list_t *record_list){
	refresh_status();
	refresh_mode();             //更新模式信息
	if(flags->mode == 0){      //从录音到播放模式
		if(flags->list_mode == 0){  //音乐列表
			refresh_audio_info(play_list);  
		}else{
			refresh_audio_info(record_list);
		}
		refresh_list_mode();
	}
}




/**********************************************************************
 * 函数名称： refresh_refresselect_list_handle
 * 功能描述： 更新切换模式后所有界面   包括当前状态  当前列表模式  当前音频信息  当前播放模式
 * 输入参数： play_list   list_t结构体指针
 *            record_list list_t结构体指针
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
extern void refresh_refresselect_list_handle(list_t *play_list,list_t *record_list){
	refresh_status();            //更新状态
	refresh_list_mode();         //更新列表
	if(flags->list_mode == 0)    //更新音频信息
		refresh_audio_info(play_list);
	else
		refresh_audio_info(record_list);				
	if(flags->mode == 0)               //录音模式到播放模式还需更新模式信息
		refresh_mode();
}
