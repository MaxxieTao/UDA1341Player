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
    u8    is_continue;    //是否允许播放音频 
	u8    is_pause;       //是否为暂停状态 
	u8    is_allow;      //播放完毕一个音频是否允许继续 
	u8    mode;          //模式   0-播放   1-录音
	u8    list_mode;     //0-播放列表   1-录音列表   2-录音中
	char  shift_mode; //0-顺序循环播放  1-随机播放   2-单曲循环
	u32   play_id;      
	off_t curpos;     //文件偏移  用于恢复暂停的文件和快进快退
}flag_t;
#endif  //_BASE_H

