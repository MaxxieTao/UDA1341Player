#include <time.h>
#include "rtc.h"
/**********************************************************************
 * 函数名称： get_rtc_time
 * 功能描述： 获取当前时间
 * 输入参数： 无
 * 输出参数： 年+月+日+时+分+秒
 * 返 回 值： 无
 ***********************************************************************/
extern const char* get_rtc_time(){
	static char temp_name[16];
	time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	/*printf("OS date/time(UTC): %d/%d/%d %02d:%02d:%02d\n",
                p->tm_mday, p->tm_mon + 1, p->tm_year + 1900,
                p->tm_hour, p->tm_min, p->tm_sec);*/
	sprintf(temp_name,"%04d%02d%02d%02d%02d%02d",p->tm_year + 1900,p->tm_mon + 1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	return temp_name;
}





/**********************************************************************
 * 函数名称： get_shuffer_num
 * 功能描述： 获取随机数
 * 输入参数： 无
 * 输出参数： 随机数
 * 返 回 值： 无
 ***********************************************************************/
extern unsigned int get_shuffer_num(){
	unsigned int temp_num=0;
	time_t timep;
	struct tm *p;
	
	time(&timep);
	p = gmtime(&timep);

    temp_num = (p->tm_sec*128)+rand();
	return temp_num;
}
