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
 * �������ƣ� recording_thread
 * ���������� ¼����Ƶ���߳�
 * ��������� play_nodeָ��,
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
extern int recording_thread(list_t *plist){
	char temp_name[32];
	char combine_name[256];
	char gbk_name[256];
	int fd_dsp;
	FILE *wav; 
	u32 loop_count=0;
	u8 one_sec_count=0;

	flags->is_allow=1;//1-����¼��   0-����¼��
	signal(SIGCONT,SIG_DFL);//�źŰ󶨱�Ŵ�����
	signal(SIGTSTP,SIG_DFL);
	memset(temp_name,'\0',sizeof(temp_name));
	memset(combine_name,'\0',sizeof(combine_name));
	memset(gbk_name,'\0',sizeof(gbk_name));
	memset(audio_buffer,0,sizeof(audio_buffer));

	snprintf(temp_name,sizeof(temp_name),"%s",get_rtc_time());
	snprintf(combine_name,sizeof(combine_name),"%s%s","/player/record_list/",temp_name);//ƴ���ַ���������·��+�ļ���
	wav=fopen(combine_name,"w"); 
	printf("temp_name=%s\n",temp_name);
	refresh_record_name(temp_name);
	fd_dsp = open("/dev/dsp", O_RDONLY); 
	if (fd_dsp < 0){
		printf("can't open\n");
		return 0;
	}
	while(flags->is_allow){
		read(fd_dsp,audio_buffer,sizeof(audio_buffer)); //¼����Ƶ�ļ�		
		fwrite(audio_buffer,sizeof(audio_buffer),1,wav);//���ļ���д�뻺����  		
		loop_count++;
		if(one_sec_count == 9){//ȡ9��ԭ������Ϊ  ��Լÿ10����ӳ�1��
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
	pthread_exit(NULL);//�����߳�
}





/**********************************************************************
 * �������ƣ� playing_thread
 * ���������� �������ֵ��߳�
 * ��������� play_nodeָ��,
 * ��������� ��
 * �� �� ֵ�� ��
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
	while(flags->is_allow){      //�������Զ�������һ�� 

		flags->is_continue = 1;//����flag  1Ϊ��������  0��������
		curpos = 0;
		loop_count = 0;
		one_sec_count = 0;
		memset(temp_name,'\0',sizeof(temp_name));
		if(type == 0){
			sprintf(temp_name,"%s%s","/player/play_list/",play_node->file_name); //ƴ���ַ���������·��+�ļ���
			printf("now playing: %s\n",play_node->gbk_name);
		}
		else{
			sprintf(temp_name,"%s%s","/player/record_list/",record_node->file_name); 
			printf("now playing: %s\n",record_node->gbk_name);
		}
		
		wav = NULL;
		//printf("1.flags->curpos=%d\n",flags->curpos);
		if((wav = fopen(temp_name,"r")) == NULL){            //�����ļ���ʧ����
			printf("the audio file is missing\n");
			error_missing_the_file();
			pthread_exit(NULL);
		}
		fd_dsp = open("/dev/dsp", O_WRONLY);        //������������O_WRONLY����ģʽ
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
					fread(audio_buffer,sizeof(audio_buffer),1,wav);//���ļ������뻺����  
					write(fd_dsp,audio_buffer,sizeof(audio_buffer));   //������Ƶ�ļ�
					loop_count++;
					if(one_sec_count == 9){//ȡ9��ԭ������Ϊ  ��Լÿ1����ӳ�0.1��
						//printf("loop_count=%d\n\n",loop_count);
						refresh_time(loop_count);
						one_sec_count = 0;
					}
					else
						one_sec_count++;
				}else{    //��������  
					shift_audio_handle(plist);
					refresh_audio_info(plist);
					
					pthread_mutex_lock(&player_mutex); 
					flags->curpos  = 0;
					pthread_mutex_unlock(&player_mutex);     //����
					//printf("normal ending\n");
					
					break;
				}
			}
			           
			if(flags->is_pause == 1){
				//printf("cutting\n");       //�и� 
				pthread_mutex_lock(&player_mutex); 
				flags->curpos  = ftell(wav);
				pthread_mutex_unlock(&player_mutex);     //����
	
			}
			fclose(wav);
			//printf("2.flags->curpos=%d\n",flags->curpos);			
		}
		close(fd_dsp);
	}
	//close(fd_dsp);	

	pthread_exit(NULL);//�����߳�
}

/* 2016.3.20��ʼ  
 * bug�޸�:
 * ��ʾ��ǰ������������                        --�������߳���ʾ                          ���
 * ���Ʋ����Զ�������һ��                      --�޸��߳��߼�                            ���
 * ��ͣʱ�и�Ӧ��ֱ�Ӳ����е�����Ŀ            --�޸��߼�                                ���
 * ������ȷ�и�                                --�޸��߼�                                ���
 * ֻ���ڲ���ģʽ���ܵ���¼��                  --�޸��߼�                                ���
 * ¼��ģʽ�е�����¼���б�  ������������      --�޸��߼�                                ���
 * ����������δ��װ��������������С���Ǻ�    --������������Ĭ�ϴ�С  ���������С����  ���
 * ������֮ͣ��������                          --��ͣ����ͣ�̸߳�Ϊ��¼λ��  ֹͣ�߳�
 * ���һ��ʼ�ǿ�Ŀ¼  ������Ž��˳�����
 * ���ɨ������б��ȫ�����  �����߳���
   ��ͣѭ��  �̲߳�֪����ʱ����
 * �����еĸ��� ͻȻ��ɾ��  ���޷���������
   ��Ҫ�ֶ����ֹͣ
 * ������Ŵ���
 * �л��б��ģʽˢ�½���
 
 # ��������:
 # ���ӳ�����                                --�޸Ŀ�� �˳�while(1)�Զ�����           ���	
 # ����¼����ʾ�б�                            --�޸��߼�                                ���
 # �����ֲ����б�ת����¼���б�                --�޸��߼�                                ���
 # �����б�ֱ������һ��һ����                --�޸��߼�                                ���
 # �������                                    --�����������������س���ʱ��              ���
 # �����ļ�����ɨ������
 # ��������10������ ��һ�������и����ز����׸�  
   10���������вŻ������и�                    --�����ļ�ƫ�Ƽ���ʱ��                    ���
 # ����ǰ  ������ɾ��  �����Զ���������        --��ǰ�ļ��Ƿ�����ж�                  ���
 */

