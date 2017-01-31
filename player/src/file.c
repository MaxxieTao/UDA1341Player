#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>  
#include <math.h>
#include "base.h"
#include "file.h"
#include "font.h"

/**********************************************************************
 * 函数名称： get_file_size
 * 功能描述： 计算文件大小
 * 输入参数： 相对路径下文件名
 * 输出参数： 无
 * 返 回 值： 文件大小(B)
 ***********************************************************************/
static off_t get_file_size(char *filename)  
{  
    struct stat statbuf;  
	off_t size;
    stat(filename,&statbuf);//通过文件名filename获取文件信息，并保存在statbuf中
    size = statbuf.st_size;  
    return size;  
} 





/**********************************************************************
 * 函数名称： init_list
 * 功能描述： 创建音频文件链表
 * 输入参数： 无 
 * 输出参数： 无
 * 返 回 值： list_t结构体指针
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
 * 函数名称： is_empty
 * 功能描述： 判断链表是否为空
 * 输入参数： list_t链表结构体指针
 * 输出参数： 无
 * 返 回 值： 1-为空   
 *            0-不为空
 ***********************************************************************/
static int is_empty(list_t *plist){  
	if(NULL == plist->front && NULL == plist->rear && plist->size ==0)
		return 1;
	else 
		return 0;
}





/**********************************************************************
 * 函数名称： add_node
 * 功能描述： 添加音频文件节点到链表
 * 输入参数： plist     链表结构体指针
 *            name      UTF文件名
 *			  gbk_name  GBK文件名
 *            type      类型
 * 输出参数： 无
 * 返 回 值： node_t结构体指针
 ***********************************************************************/
node_t *add_node(list_t *plist,char *name,char *gbk_name,u8 type){
	char temp_name[256];
	double size;
	node_t *pnode = (node_t *)malloc(sizeof(node_t));
	memset(pnode->file_name,'\0',sizeof(pnode->file_name));
	memset(pnode->gbk_name,'\0',sizeof(pnode->gbk_name));
	memset(temp_name,'\0',sizeof(temp_name));
	if(NULL != pnode){
		if(is_empty(plist)!=1){   //不为空
			plist->rear->next  = pnode;
			plist->front->prev = pnode;
			pnode->next = plist->front;
			pnode->prev = plist->rear;
			plist->rear = pnode;
		}else{                    //为空
			plist->front = pnode;
			plist->rear  = pnode;
			pnode->next  = pnode;
			pnode->prev  = pnode;
		}
		plist->size++;                     //更新音频数量
		pnode->id = plist->size;           //更新节点id
		pnode->type = type;
		strncpy(pnode->file_name,name,strlen(name));     //更新节点文件名
		strncpy(pnode->gbk_name,gbk_name,strlen(gbk_name));
		if(type==0)
			snprintf(temp_name,sizeof(temp_name),"%s%s","/player/play_list/",name);
		else
			snprintf(temp_name,sizeof(temp_name),"%s%s","/player/record_list/",name);	
		size = (double)get_file_size(temp_name);   //计算文件大小
		size = ceil(size/1024);             //转换为KB 向上取整
		pnode->file_size = (off_t)(size);
	}
	else 
		return NULL;
	return pnode;
}





/**********************************************************************
 * 函数名称： free_node
 * 功能描述： 销毁列表中的内容
 * 输入参数： 无 
 * 输出参数： 无
 * 返 回 值： list_t结构体指针
 ***********************************************************************/
void free_list(list_t *plist){
	int i,size;
	node_t *pnode;
	size=plist->size;                            //获取音频数量
	for(i=0;i<size;i++){ 
		pnode = plist->front;
		plist->front = plist->front->next;         //更新头节点
		free(pnode);
		pnode = NULL;
	}
	plist->size = 0;
	plist = NULL;
}





/**********************************************************************
 * 函数名称： creat_file_list
 * 功能描述： 遍历将目录下的所有文件，添加到链表中
 * 输入参数： list_t链表结构体指针,
 *            path文件所处路径
 *            type文件类型
 * 输出参数： 无
 * 返 回 值： 0  正常
 *			 -1  错误
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
    if ((dir=opendir(path)) == NULL)      //判断路径是否存在
    {
        perror("Open dir error...");
        return -1;
    }else{
		while(ptr = readdir(dir)){//将来可能更改为判断后缀名
			
			if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)     //去除.和..目录
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
 * 函数名称： print_list
 * 功能描述： 串口调试下打印列表
 * 输入参数： plist   列表结构体指针
 * 输出参数： 无
 * 返 回 值： 无
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

