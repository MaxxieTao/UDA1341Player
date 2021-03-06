/*  图形:播放  */
const unsigned char play_graphy[32]={
0X00,0X00,0X00,0X00,0X00,0X00,0XFE,0XFC,0XF8,0XF0,0XE0,0XC0,0X80,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X3F,0X1F,0X0F,0X07,0X03,0X01,0X00,0X00,0X00,0X00};
/*  图形:暂停  */
const unsigned char pause_graphy[32]={
0X00,0X00,0XFC,0XFC,0XFC,0XFC,0X00,0X00,0X00,0X00,0XFC,0XFC,0XFC,0XFC,0X00,0X00,
0X00,0X00,0X3F,0X3F,0X3F,0X3F,0X00,0X00,0X00,0X00,0X3F,0X3F,0X3F,0X3F,0X00,0X00};
/*  图形:停止   */
const unsigned char stop_graphy[32]={
0X00,0X00,0XFC,0XFC,0XFC,0X0C,0X6C,0X6C,0X6C,0X6C,0X6C,0XFC,0XFC,0XFC,0X00,0X00,
0X00,0X00,0X1F,0X1F,0X1F,0X1B,0X1B,0X1B,0X1B,0X1B,0X18,0X1F,0X1F,0X1F,0X00,0X00};
/*  图形:顺序循环   */
const  unsigned char loop_graphy[32]={
0X00,0X00,0XFC,0X04,0X04,0X04,0X04,0X04,0X04,0X04,0X04,0X1F,0X0E,0XE4,0X00,0X00,
0X00,0X00,0X27,0X70,0XF8,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X3F,0X00,0X00};
/*  图形:单曲循环   */
const  unsigned char repeat_graphy[32]={
0X00,0X00,0XFC,0X04,0X04,0X04,0X04,0X24,0XF4,0X04,0X04,0X1F,0X0E,0XE4,0X00,0X00,
0X00,0X00,0X27,0X70,0XF8,0X20,0X28,0X28,0X2F,0X28,0X28,0X20,0X20,0X3F,0X00,0X00};
/*  图形:随机播放  */
const  unsigned char shuffle_graphy[32]={
0X00,0X02,0X06,0X0C,0X18,0X30,0X60,0XC0,0X60,0X30,0X1C,0X04,0X1F,0X0E,0X04,0X00,
0X00,0X40,0X60,0X30,0X18,0X0C,0X06,0X03,0X06,0X0C,0X38,0X20,0XF8,0X70,0X20,0X00};
/*  图形:音量1   */
const  unsigned char volume1_graphy[32]={
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X07,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
/*  图形:音量2    */
const  unsigned char volume2_graphy[32]={
0X00,0X00,0X00,0X80,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X07,0X00,0X07,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
/*  图形:音量3   */
const  unsigned char volume3_graphy[32]={
0X00,0X00,0X00,0X80,0X00,0XC0,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
/*  图形:音量4    */
const  unsigned char volume4_graphy[32]={
0X00,0X00,0X00,0X80,0X00,0XC0,0X00,0XE0,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};
/*  图形:音量5   */
const  unsigned char volume5_graphy[32]={
0X00,0X00,0X00,0X80,0X00,0XC0,0X00,0XE0,0X00,0XF0,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X00,0X00,0X00,0X00,0X00};
/*  图形:音量6   */
const  unsigned char volume6_graphy[32]={
0X00,0X00,0X00,0X80,0X00,0XC0,0X00,0XE0,0X00,0XF0,0X00,0XF8,0X00,0X00,0X00,0X00,
0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X00,0X00,0X00};
/*  图形:音量7   */
const  unsigned char volume7_graphy[32]={
0X00,0X00,0X00,0X80,0X00,0XC0,0X00,0XE0,0X00,0XF0,0X00,0XF8,0X00,0XFC,0X00,0X00,
0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X07,0X00,0X00};

