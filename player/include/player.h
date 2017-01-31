#ifndef _PLAYER_H
#define _PLAYER_H
#define ONE_TENTH_SEC   17640
#define ONE_SEC   176400
#define TEN_SEC   1764000
#define SIZE ONE_TENTH_SEC   //16*1024


static unsigned char audio_buffer[SIZE]; 
extern node_t *play_node ;//��Ƶ�ڵ�
extern node_t *record_node ;//��Ƶ�ڵ�
extern flag_t *flags;
extern pthread_mutex_t  player_mutex; //ȫ�ֻ���������

int recording_thread(list_t *plist);
void playing_thread(list_t *plist);

#endif /* _PLAYER_H */
