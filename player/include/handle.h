#ifndef _HANDLE_H
#define _HANDLE_H

#define VOLUME_MUTE      0
#define VOLUME_UNMUTE    1
#define VOLUME_UP_DOWN   2


extern flag_t *flags;
extern int fd_oled;
extern node_t *play_node;//音频节点
extern node_t *record_node;//音频节点
extern pthread_mutex_t  player_mutex; //全局互斥锁对象

extern void init_flags();
extern u8 get_list_type(list_t *plist);
static void shuffer_handle(list_t *plist);  //根据随机数选取节点
extern void shift_audio_handle(list_t *plist);
extern void stop_handle(pthread_t *pid);
static int play_handle(pthread_t *pid,list_t *plist);
static int record_handle(pthread_t *pid,list_t *plist);
static void pause_handle(pthread_t *pid);
static void resume_handle();
extern int play_pause_handle(pthread_t *pid,list_t *play_list,list_t *record_list);
static void skip_handle();
extern int prev_handle(pthread_t *pid,list_t *play_list,list_t *record_list);
extern int next_handle(pthread_t *pid, list_t *play_list, list_t *record_list);
extern int backward_handle(pthread_t *pid,off_t offset,list_t *play_list,list_t *record_list);
extern int forward_handle(pthread_t *pid,off_t offset,list_t *play_list,list_t *record_list);
extern int devolume_handle(int fd_mix,int volume);
extern int upvolume_handle(int fd_mix,int volume);
extern void select_mode_handle(pthread_t *pid);
extern void select_list_handle(pthread_t *pid,list_t *play_list,list_t *record_list);
extern void shift_mode_handle();
#endif  //_HANDLE_H
