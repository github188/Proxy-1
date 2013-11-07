
#ifndef CONVERT_H
#define CONVERT_H

#include "../config/config.h"
#include "../Network/Session.h"

#ifdef KISE_DEBUG
#include <arpa/inet.h>
#endif 

/* Convert_empty : 一个空的转换规则
 * 作用： 将数据源的指针和大小复制到结果中
 */
inline int Convert_empty(const char *srcdata, int size,
		const char *&result, int &retsize)
{
#ifdef KISE_DEBUG
	int type = ntohl( *( (int*)srcdata ) ) & 0x0fffffff;
	printf("In empty convert, type is: %d\n", type);
#endif
	result = srcdata;
	retsize = size;
	return retsize;
}

/* Convert: 转换数据
 * rule: 转换的规则，默认是一个空规则
 * srcdata, size: 源数据的内容和长度
 * result, retsize [out] : 转换后的数据内容和长度
 * psession : 数据来源的会话
 * RETURN: int值是转换后的数据长度，数据存储在result中，长度为retsize
 */
int Convert(enum CVT_RULE rule, 
		const char *srcdata, int size, 
		const char *&result, int &retsize, 
		CSession* psession, struct TCPConn con);

#endif //end of CONVERT_H

