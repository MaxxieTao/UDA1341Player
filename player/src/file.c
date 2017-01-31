#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>  
#include <math.h>
#include "base.h"
#include "file.h"
#include "font.h"

/**********************************************************************
 * �������ƣ� get_file_size
 * ���������� �����ļ���С
 * ��������� ���·�����ļ���
 * ��������� ��
 * �� �� ֵ�� �ļ���С(B)
 ***********************************************************************/
static off_t get_file_size(char *filename)  
{  
    struct stat statbuf;  
	off_t size;
    stat(filename,&statbuf);//ͨ���ļ���filename��ȡ�ļ���Ϣ����������statbuf��
    size = statbuf.st_size;  
    return size;  
} 





/**********************************************************************
 * �������ƣ� init_list
 * ���������� ������Ƶ�ļ�����
 * ��������� �� 
 * ��������� ��
 * �� �� ֵ�� list_t�ṹ��ָ��
 ***********************************************************************/
list_t *init_list(){   
	list_t *plist =(list_t *)malloc(sizeof(list_t));
	if( NULL != plist){
		plist->front = NULL;
		plist->rear = NULL;
		plist->size = 0;
		memset(plist->name,0,sizeof(plist->name));
	}else{
		printf("audio_list_t malloc error\n");
		return NULL;
	}
	return plist;
}





/**********************************************************************
 * �������ƣ� is_empty
 * ���������� �ж������Ƿ�Ϊ��
 * ��������� list_t����ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� 1-Ϊ��   
 *            0-��Ϊ��
 ***********************************************************************/
static int is_empty(list_t *plist){  
	if(NULL == plist->front && NULL == plist->rear && plist->size ==0)
		return 1;
	else 
		return 0;
}





/**********************************************************************
 * �������ƣ� add_node
 * ���������� �����Ƶ�ļ��ڵ㵽����
 * ��������� plist     ����ṹ��ָ��
 *            name      UTF�ļ���
 *			  gbk_name  GBK�ļ���
 *            type      ����
 * ��������� ��
 * �� �� ֵ�� node_t�ṹ��ָ��
 ***********************************************************************/
node_t *add_node(list_t *plist,char *name,char *gbk_name,u8 type){
	char temp_name[256];
	double size;
	node_t *pnode = (node_t *)malloc(sizeof(node_t));
	memset(pnode->file_name,'\0',sizeof(pnode->file_name));
	memset(pnode->gbk_name,'\0',sizeof(pnode->gbk_name));
	memset(temp_name,'\0',sizeof(temp_name));
	if(NULL != pnode){
		if(is_empty(plist)!=1){   //��Ϊ��
			plist->rear->next  = pnode;
			plist->front->prev = pnode;
			pnode->next = plist->front;
			pnode->prev = plist->rear;
			plist->rear = pnode;
		}else{                    //Ϊ��
			plist->front = pnode;
			plist->rear  = pnode;
			pnode->next  = pnode;
			pnode->prev  = pnode;
		}
		plist->size++;                     //������Ƶ����
		pnode->id = plist->size;           //���½ڵ�id
		pnode->type = type;
		strncpy(pnode->file_name,name,strlen(name));     //���½ڵ��ļ���
		strncpy(pnode->gbk_name,gbk_name,strlen(gbk_name));
		if(type==0)
			snprintf(temp_name,sizeof(temp_name),"%s%s","/player/play_list/",name);
		else
			snprintf(temp_name,sizeof(temp_name),"%s%s","/player/record_list/",name);	
		size = (double)get_file_size(temp_name);   //�����ļ���С
		size = ceil(size/1024);             //ת��ΪKB ����ȡ��
		pnode->file_size = (off_t)(size);
	}
	else 
		return NULL;
	return pnode;
}





/**********************************************************************
 * �������ƣ� free_node
 * ���������� �����б��е�����
 * ��������� �� 
 * ��������� ��
 * �� �� ֵ�� list_t�ṹ��ָ��
 ***********************************************************************/
void free_list(list_t *plist){
	int i,size;
	node_t *pnode;
	size=plist->size;                            //��ȡ��Ƶ����
	for(i=0;i<size;i++){ 
		pnode = plist->front;
		plist->front = plist->front->next;         //����ͷ�ڵ�
		free(pnode);
		pnode = NULL;
	}
	plist->size = 0;
	plist = NULL;
}





/**********************************************************************
 * �������ƣ� creat_file_list
 * ���������� ������Ŀ¼�µ������ļ�����ӵ�������
 * ��������� list_t����ṹ��ָ��,
 *            path�ļ�����·��
 *            type�ļ�����
 * ��������� ��
 * �� �� ֵ�� 0  ����
 *			 -1  ����
 ***********************************************************************/
int creat_file_list(list_t *plist,char *name,u8 type)
{
	DIR *dir;
	node_t *ret;
	char path[32];
	char gbk_name[256];
	struct dirent *ptr;
	if(strcmp(name,"play")==0){
		strcpy(plist->name,name);
		sprintf(path,"%s","/player/play_list/");
	}else{
		strcpy(plist->name,name);
		sprintf(path,"%s","/player/record_list/");
	}
    if ((dir=opendir(path)) == NULL)      //�ж�·���Ƿ����
    {
        perror("Open dir error...");
        return -1;
    }else{
		while(ptr = readdir(dir)){//�������ܸ���Ϊ�жϺ�׺��
			
			if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)     //ȥ��.��..Ŀ¼
				continue;
			else
				memset(gbk_name,'\0',sizeof(gbk_name));
				utf8_to_gbk(ptr->d_name,strlen(ptr->d_name), gbk_name, sizeof(gbk_name));
				ret = add_node(plist,ptr->d_name,gbk_name,type);
				if(ret == NULL){
					return -1;
				}
		}
		closedir(dir);
	}
	return 0;
}





/**********************************************************************
 * �������ƣ� print_list
 * ���������� ���ڵ����´�ӡ�б�
 * ��������� plist   �б�ṹ��ָ��
 * ��������� ��
 * �� �� ֵ�� ��
 ***********************************************************************/
void print_list(list_t *plist){
	node_t *pnode;
	int i;
	pnode = plist->front;
	for(i=0;i<plist->size;i++){
		printf("ID=%d\tname=%s\tsize=%d\n",pnode->id,pnode->gbk_name,pnode->file_size);
		pnode = pnode->next;
	}
}

