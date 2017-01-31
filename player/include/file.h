#ifndef _FILE_H
#define _FILE_H

typedef struct node{  //��Ƶ��Ϣ
	u16 id;
	off_t file_size;
	u8  type;             //0-���������ļ�  1-¼�������ļ�
	char file_name[256];
	char gbk_name[256];
	struct node *prev;
	struct node *next;
}node_t;

typedef struct list{  //��Ƶ�б�
	u16  size;
	char   name[8];
	node_t *front;
	node_t *rear;
}list_t;

static off_t get_file_size(char *filename);
static int is_empty(list_t *plist);
extern list_t *init_list();   
extern node_t *add_node(list_t *plist,char *name,char *gbk_name,u8 type);
extern void free_list(list_t *plist);
extern int creat_file_list(list_t *plist,char *name,u8 type);
extern void print_list(list_t *plist);
#endif /* _FILE_H */

