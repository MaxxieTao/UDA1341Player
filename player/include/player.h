#ifndef _PLAYER_H
#define _PLAYER_H
#define ONE_TENTH_SEC   17640
#define ONE_SEC   176400
#define TEN_SEC   1764000
#define SIZE ONE_TENTH_SEC   //16*1024


static unsigned char audio_buffer[SIZE]; 
extern node_t *play_node ;//音频节点
extern node_t *record_node ;//音频节点
extern flag_t *flags;
extern pthread_mutex_t  player_mutex; //全局互斥锁对象

int recording_thread(list_t *plist);
void playing_thread(list_t *plist);

#endif /* _PLAYER_H */
