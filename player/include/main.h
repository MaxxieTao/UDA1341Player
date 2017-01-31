#ifndef _MAIN_H
#define _MAIN_H
typedef struct  button_key_time{
	unsigned char key;
	u32 time;
}button_val_t;
oled_val_t   *oled_val = NULL;
node_t *play_node   = NULL;//音频节点
node_t *record_node = NULL;//音频节点
flag_t *flags       = NULL;
int fd_oled;
pthread_mutex_t  player_mutex; //全局互斥锁对象
#endif  //_MAIN_H
