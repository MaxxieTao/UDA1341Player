#ifndef _UI_H
#define _UI_H

#define BIG_CHAR  0
#define TINY_CHAR 1
#define GRAPHIC   2
#define CLEAN_N   3
#define CLEAN_ALL 4
typedef struct oled_type_data{
	u8 page; //第几页
	u8 col;  //第几列
	u8 type; //类型
	u8 text[32];
}oled_val_t;
extern oled_val_t *oled_val;
extern flag_t *flags;
extern int fd_oled;
extern node_t *play_node;//音频节点
extern node_t *record_node;//音频节点
extern void error_missing_the_file();
extern void error_missing_the_btn();
extern void error_missing_the_mix();
extern void error_init_list();
extern void error_create_list();
extern void error_other();
extern void init_oled_type_data(oled_val_t *p);
static oled_val_t *package_oled_val(u8 type, u8 page, u8 col, const u8 *text);
extern void refresh_status();
static void refresh_cur_count(list_t *plist);
static void refresh_split();
static void refresh_clean_2_to_7();
static void refresh_time_min(u32 min);
static void refresh_time_colon();
static void refresh_time_sec(u32 sec);
extern void refresh_time(u32 count);
static  void refresh_all_count(list_t *plist);
extern void refresh_shift_mode();
extern void refresh_volume(u8 volume);
static void refresh_audio_name(list_t *plist);
extern void refresh_record_name(const u8 *text);
extern void refresh_mode();
extern void refresh_list_mode();
extern void refresh_audio_info(list_t *plist);
extern void init_ui(list_t *plist,int volume);
extern void refresh_prev_handle(list_t *play_list,list_t *record_list);
extern void refresh_next_handle(list_t *play_list,list_t *record_list);
extern void refresh_select_mode_handle(list_t *play_list,list_t *record_list);
extern void refresh_refresselect_list_handle(list_t *play_list,list_t *record_list);
#endif //_UI_H
