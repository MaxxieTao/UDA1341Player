#include <iconv.h>
/**********************************************************************
 * �������ƣ� utf8_to_unc
 * ���������� UFT��ʽת����UNICODE��ʽ
 * ��������� sourcebuf   Դ�ַ���
 *			  sourcelen   Դ����
 *			  destbuf     Ŀ���ַ���
 *  		  destlen     Ŀ�곤��
 * ��������� ��
 * �� �� ֵ�� 0   �ɹ�
             -1   ʧ��
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
 * �������ƣ� unc_to_gbk
 * ���������� UNICODE��ʽת����GBK��ʽ
 * ��������� sourcebuf   Դ�ַ���
 *			  sourcelen   Դ����
 *			  destbuf     Ŀ���ַ���
 *  		  destlen     Ŀ�곤��
 * ��������� ��
 * �� �� ֵ�� 0   �ɹ�
             -1   ʧ��
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
 * �������ƣ� utf8_to_gbk
 * ���������� UTF-8��ʽת����GBK��ʽ
 * ��������� sourcebuf   Դ�ַ���
 *			  sourcelen   Դ����
 *			  destbuf     Ŀ���ַ���
 *  		  destlen     Ŀ�곤��
 * ��������� ��
 * �� �� ֵ�� 0   �ɹ�
             -1   ʧ��
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

