#ifndef _FONT_H
#define _FONT_H

static int utf8_to_unc(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
static int unc_to_gbk(const char *sourcebuf,size_t sourcelen,char *destbuf,size_t destlen);
extern int utf8_to_gbk(const char *src_buf,size_t src_len,char *dest_buf,size_t dest_len);
#endif //_FONT_H

