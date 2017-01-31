#include <iconv.h>
/**********************************************************************
 * 函数名称： utf8_to_unc
 * 功能描述： UFT格式转换成UNICODE格式
 * 输入参数： sourcebuf   源字符串
 *			  sourcelen   源长度
 *			  destbuf     目标字符串
 *  		  destlen     目标长度
 * 输出参数： 无
 * 返 回 值： 0   成功
             -1   失败
 ***********************************************************************/
static int utf8_to_unc(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen) {   
	iconv_t cd;   
	if( (cd = iconv_open("UCS-2","UTF-8")) ==0 )     
	      return -1;  
	memset(destbuf,0,destlen);   
	const char **source = &sourcebuf;   
	char **dest = &destbuf;
	if(-1 == iconv(cd,(char **)source,&sourcelen,dest,&destlen))     
	     return -1;   
	iconv_close(cd);   
	return 0;   
}





/**********************************************************************
 * 函数名称： unc_to_gbk
 * 功能描述： UNICODE格式转换成GBK格式
 * 输入参数： sourcebuf   源字符串
 *			  sourcelen   源长度
 *			  destbuf     目标字符串
 *  		  destlen     目标长度
 * 输出参数： 无
 * 返 回 值： 0   成功
             -1   失败
 ***********************************************************************/
static int unc_to_gbk(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen) {   
	iconv_t cd;   
	if( (cd = iconv_open("GBK", "UCS-2")) ==0 )     
	      return -1;  
	memset(destbuf,0,destlen);   
	const char **source = &sourcebuf;   
	char **dest = &destbuf;
	if(-1 == iconv(cd,(char **)source,&sourcelen,dest,&destlen))     
	     return -1;   
	iconv_close(cd);   
	return 0;   
}





/**********************************************************************
 * 函数名称： utf8_to_gbk
 * 功能描述： UTF-8格式转换成GBK格式
 * 输入参数： sourcebuf   源字符串
 *			  sourcelen   源长度
 *			  destbuf     目标字符串
 *  		  destlen     目标长度
 * 输出参数： 无
 * 返 回 值： 0   成功
             -1   失败
 ***********************************************************************/
int utf8_to_gbk(const char *src_buf,size_t src_len,char *dest_buf,size_t dest_len){
	char unc_buffer[256];
	memset(unc_buffer,'\0',sizeof(unc_buffer));
	if(utf8_to_unc(src_buf, src_len, unc_buffer, sizeof(unc_buffer)) !=0 )
		return -1;
	if(unc_to_gbk(unc_buffer, sizeof(unc_buffer), dest_buf, dest_len) !=0)
		return -1;
	return 0;
}

