#ifndef _MAIN_H
#define _MAIN_H
typedef struct  button_key_time{
	unsigned char key;
	u32 time;
}button_val_t;
oled_val_t   *oled_val = NULL;
node_t *play_node   = NULL;//��Ƶ�ڵ�
node_t *record_node = NULL;//��Ƶ�ڵ�
flag_t *flags       = NULL;
int fd_oled;
pthread_mutex_t  player_mutex; //ȫ�ֻ���������
#endif  //_MAIN_H
