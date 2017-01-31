#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "base.h"
#include "file.h"
#include "player.h"
#include "handle.h"
/**********************************************************************
 * �������ƣ� init_flags
 * ���������� ��ʼ��flags
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
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
 * �������ƣ� get_list_type
 * ���������� ��ȡ�б�����
 * ��������� plist   �б�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� 0   �����б�
 *			  1   ¼���б�
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
 * �������ƣ� shuffer_handle
 * ���������� ���������ѡ����һ��Ҫ���ŵ���Ƶ
 * ��������� plist   �б�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void shuffer_handle(list_t *plist){  //���������ѡȡ�ڵ�
	u8 type;
	u32 shuffle_num;
	type = get_list_type(plist);
	shuffle_num = get_shuffer_num();           
	shuffle_num = shuffle_num % (plist->size); //������֤��������ظ����Լ�
	if(type==0){
		while(shuffle_num){
			play_node = play_node->next;       //ͨ��ѭ��shuffle_num��  ����ı���һ����Ƶ
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
 * �������ƣ� shift_audio_handle
 * ���������� ���ģʽ����
 * ��������� plist   �б�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
void shift_audio_handle(list_t *plist){
	u8 type;
	type = get_list_type(plist);
	if(flags->shift_mode == 0){//˳��ѭ��
		if(type==0)
			play_node = play_node->next;
		else
			record_node = record_node->next;
	}else if(flags->shift_mode == 1){//�������
		shuffer_handle(plist);
	}
	else if(flags->shift_mode == 2){ //��������
		;
	}
}





/**********************************************************************
 * �������ƣ� stop_handle
 * ���������� ֹͣ����  
 *            ��Ӧ������K1�����ζ̰�
 * ��������� pid   �̺߳�
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
void stop_handle(pthread_t *pid){
	pthread_kill(*pid, SIGCONT);       
	flags->is_allow    = 0;
	flags->is_continue = 0;
	flags->is_pause    = 0;
	printf("stop ok1\n");
	pthread_join(*pid,NULL);  //��������  ֱ���߳̽���  �������̵߳���Դ
	
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
 * �������ƣ� play_handle
 * ���������� ���Ŵ���  ���ڲ�����Ƶ
 * ��������� pid   �̺߳�
 *            plist �б�ṹ��ָ��
 * ��������� 0  ����
 *           -1  ʧ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static int play_handle(pthread_t *pid,list_t *plist){
	flags->is_continue = 1;
	flags->is_allow    = 1;
	pthread_mutex_lock(&player_mutex); 
	if(flags->is_pause == 0) //��֮ǰδ��ͣ  ����ƫ����
		flags->curpos = 0;
	else{
		flags->is_pause = 0;
		flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;//Ϊ�˱�֤��ʱ��׼ȷ ��1:52:03  ����Ϊ1:52:00
	}
	pthread_mutex_unlock(&player_mutex);     //����
	if(pthread_create(pid,NULL,(void *)playing_thread,(void *)plist) != 0)  //���������߳�
		return -1;
	return 0;
}





/**********************************************************************
 * �������ƣ� record_handle
 * ���������� ¼������   ����¼����Ƶ
 * ��������� pid   �̺߳�
 *            plist �б�ṹ��ָ��
 * ��������� 0  ����
 *           -1  ʧ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static int record_handle(pthread_t *pid,list_t *plist){
	flags->is_allow = 1;                                                       
	flags->is_pause = 0;  //ȡ����ͣ
	if(pthread_create(pid,NULL,(void *)recording_thread,(void *)plist) !=0)//����¼���߳�   
		return -1;
	return 0;
}





/**********************************************************************
 * �������ƣ� pause_handle
 * ���������� ��ͣ����  ������ͣ��Ƶ(��ʵ��Ϊֹͣ�����Ǽ�¼���ļ�ƫ��)
 * ��������� pid   �̺߳�
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
void pause_handle(pthread_t *pid){
	flags->is_pause    = 1; 
	flags->is_continue = 0;
	flags->is_allow    = 0;
	pthread_join(*pid,NULL);//��������  ֱ���߳̽���  �������̵߳���Դ
}





/**********************************************************************
 * �������ƣ� resume_handle
 * ���������� �������Ŵ���   
 *			  ���ڼ���������Ƶ(��ʵ��ֻ������pause��־  �������Ż�����play_handle���� )
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void resume_handle(){
	flags->is_pause    = 0; 
	flags->is_continue = 1;
	flags->is_allow    = 1;
}





/**********************************************************************
 * �������ƣ� skip_handle
 * ���������� �����˴δ���
 *            һ�����ڲ�����һ�ײ�����
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
static void skip_handle(){
	flags->is_continue = 0;
	flags->is_allow    = 1;
	pthread_mutex_lock(&player_mutex); 
	flags->curpos      = 0;
	pthread_mutex_unlock(&player_mutex);     //����
}




/**********************************************************************
 * �������ƣ� play_pause_handle
 * ���������� ���š�¼������ͣ�Ĵ���������������
 *            ��Ӧ������K2�����ζ̰�
 * ��������� pid          ���̺�
 *            play_list    �����б�
 *            record_list  ¼���б�
 * ��������� ��
 * �� �� ֵ�� 0  �ɹ�
             -1  ʧ��
 ***********************************************************************/
int play_pause_handle(pthread_t *pid,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode == 0){                                               //����ģʽ
		if(flags->list_mode == 0){                                      //�����б�
			if(play_list->size>0){                                      //�ǿ��б�
				if(flags->is_allow == 0){                               //��������
					ret = play_handle(pid,play_list);
					if(ret < 0)
						return ret;
					flags->is_pause == 0;
				}else{
					pause_handle(pid);
					DPRINTF("pause\n");
				}
			}
		}else{                                                           //¼���б�
			DPRINTF("record_list=%d\n",record_list->size);
			if(record_list->size > 0){                                   //�ǿ��б�
				if(flags->is_allow == 0){                                //�������� 
					ret = play_handle(pid,record_list); 
					if(ret < 0)
						return ret;
					flags->is_pause == 0;
				}else{
					if(flags->is_pause == 0){                            //��ͣ
						pause_handle(pid);
						DPRINTF("pause\n");
					}
				}
			}
		}
	}
	else{                                                              //¼��ģʽ
		if(flags->is_allow == 0){                                      //����¼��
			ret = record_handle(pid,record_list);
			if(ret < 0)
				return ret;
		}else{
			if(flags->is_pause == 0){                                //��ͣ
				flags->is_pause = 1;                               
				pthread_kill(*pid, SIGTSTP);                         //������ͣ�����ź�
				DPRINTF("pause\n"); 
			}else{                                                   //����
				flags->is_pause = 0;	                                                  
				pthread_kill(*pid, SIGCONT);       
				DPRINTF("continue\n");		
			}			
		}
	}
	return 0;
} 




/**********************************************************************
 * �������ƣ� prev_handle
 * ���������� ������һ�ף����ܲ���˳����Σ�ֻ�����л���������������
 *            ��Ӧ������K3�����ζ̰�
 * ��������� pid          ���̺�
 *            play_list    �����б�
 *            record_list  ¼���б�
 * ��������� ��
 * �� �� ֵ�� 0  �ɹ�
             -1  ʧ��
 ***********************************************************************/
int prev_handle(pthread_t *pid,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode == 0){
		if(flags->list_mode == 0){                             //�����б�
			if(play_list->size > 0){                           //�ǿ��б�
				if(flags->is_pause == 1){                     //��ͣʱ�и� 
					stop_handle(pid);
					play_node = play_node->prev;
					ret = play_handle(pid,play_list); 
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                //ֹͣ״̬���Ϸ�
						play_node = play_node->prev;
					}else{                                   //����״̬���и�
						pause_handle(pid);
						if(flags->curpos > TEN_SEC){	     //����ʱ�����10�룬���²��ŵ�ǰ��Ƶ
							flags->is_pause = 0;
							ret = play_handle(pid,play_list);
							if(ret < 0)
								return ret;
						}else{                               //����ʱ��С��10�룬�����и�
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
		else{     //¼���б�          
			if(record_list->size > 0){
				if(flags->is_pause == 1){                 //��ͣʱ�и�
					stop_handle(pid);
					record_node = record_node->prev;
					ret = play_handle(pid,record_list);
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                    //ֹͣ״̬���Ϸ�
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
 * �������ƣ� next_handle
 * ���������� ������һ�ף���Ϊ�������ģʽ�����������һ����Ƶ��������������
 *            ��Ӧ������K4�����ζ̰�
 * ��������� pid          ���̺�
 *            play_list    �����б�
 *            record_list  ¼���б�
 * ��������� ��
 * �� �� ֵ�� 0  �ɹ�
             -1  ʧ��
 ***********************************************************************/
int next_handle(pthread_t *pid, list_t *play_list, list_t *record_list){
	int ret;
	if(flags->mode == 0){
		if(flags->list_mode == 0){                           //�����б�
			if(play_list->size > 0){                         //�ǿ��б�
				if(flags->is_pause == 1){                   //��ͣʱ�и�
		 			stop_handle(pid);                       
					if(flags->shift_mode == 1){              //�������ģʽ   ���ѡ��
						shift_audio_handle(play_list);
					}
					else{
						play_node = play_node->next;	
					}
					ret = play_handle(pid,play_list); 
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                    //ֹͣ״̬�·�
						play_node = play_node->next;
					}else{                                      //����ʱ�и�
						skip_handle();
						if(flags->shift_mode == 1)             //�������ģʽ   ���ѡ��
							shift_audio_handle(play_list);
						else{
							play_node=play_node->next;
						}
					}
				}
			}
		}
		else{                       //¼���б�
			if(record_list->size > 0){
				if(flags->is_pause == 1){                  //��ͣʱ�и�  
		 			stop_handle(pid);		
					if(flags->shift_mode == 1)
						shift_audio_handle(record_list);
					else
						record_node = record_node->next;	 
					ret = play_handle(pid,record_list); 
					if(ret < 0)
						return ret;
				}else{
					if(flags->is_allow == 0){                    //ֹͣ״̬���Ϸ�
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
 * �������ƣ� backward_handle
 * ���������� ���ˣ�������������
 *            ��Ӧ������K3������
 * ��������� pid          ���̺�
 *            offset       ����ʱ�䳤��
 *            play_list    �����б�
 *            record_list  ¼���б�
 * ��������� ��
 * �� �� ֵ�� 0  �ɹ�
             -1  ʧ��
 ***********************************************************************/
int backward_handle(pthread_t *pid,off_t offset,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode==0){                                            //����ģʽ
		if(flags->list_mode == 0){                                //�����б�
			if(flags->is_allow == 1){                            //����״̬��
				pause_handle(pid);                              //��ͣ  ����
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos-offset*5000;      //����1�� ��Լ����5��
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;  //ȡ������
				pthread_mutex_unlock(&player_mutex);     //����
				ret = play_handle(pid,play_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}else{
			if(flags->is_allow == 1){             //����״̬��
				pause_handle(pid);
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos-offset*5000;
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;
				pthread_mutex_unlock(&player_mutex);     //����
				ret = play_handle(pid,record_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}
	}
}

/**********************************************************************
 * �������ƣ� backward_handle
 * ���������� ���ˣ�������������
 *            ��Ӧ������K4������
 * ��������� pid          ���̺�
 *            offset       ����ʱ�䳤��
 *            play_list    �����б�
 *            record_list  ¼���б�
 * ��������� ��
 * �� �� ֵ�� 0  �ɹ�
             -1  ʧ��
 ***********************************************************************/
int forward_handle(pthread_t *pid,off_t offset,list_t *play_list,list_t *record_list){
	int ret;
	if(flags->mode==0){                            //����ģʽ
		if(flags->list_mode == 0){                 //�����б�
			if(flags->is_allow == 1){             //����״̬��
				pause_handle(pid);
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos+offset*5000;      //����1�� ��Լ����5��
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;
				pthread_mutex_unlock(&player_mutex);     //����
				ret = play_handle(pid,play_list);
				flags->is_pause == 0;
				if(ret < 0)
					return ret;
			}
		}else{
			if(flags->is_allow == 1){             //����״̬��
				pause_handle(pid);
				pthread_mutex_lock(&player_mutex); 
				flags->curpos = flags->curpos+offset*5000;
				flags->curpos = (flags->curpos / ONE_SEC) * ONE_SEC;
				pthread_mutex_unlock(&player_mutex);     //����
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
	if(flags->mode==0){                       //����ģʽ
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
	if(flags->mode==0){                       //����ģʽ
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
	if(flags->mode == 0){                     //����ģʽ 
		if(flags->list_mode == 0){            //�����б�
			flags->list_mode = 1;            //����¼���б�
			printf("play record list\n");
			print_list(record_list);   
		}else{                                //¼���б�
			flags->list_mode = 0; 
			printf("play music list\n");
			print_list(play_list);	
		}
	}else{                                       //¼��ģʽ���벥��ģʽ��¼���б�
		flags->mode = 0;                         //����ģʽ     
		flags->list_mode = 1;                    //ˢ��¼���б�
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

