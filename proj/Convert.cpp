
/* 这里实现转换接口函数Convert，使用了一些宏来定义支持哪些转换
 * 这个地方的宏可能在Makefile中被定义，所以Makefile在定义支持一个转换
 * 时需要把它对应的.o文件链接进来，实现这里extern所指定的函数
 */

#include "Convert.h"

#ifdef CONVERT_ZJ
extern int Convert_zj(const char *srcdata, int size, 
		const char **ret, int *retsize, 
		CSession *psession, struct TCPConn con);
#endif 

#ifdef CONVERT_SF
extern int Convert_sf(const char *srcdata, int size, 
		const char **ret, int *retsize, 
		CSession *psession, struct TCPConn con);
#endif

int Convert(enum CVT_RULE rule, 
		const char *srcdata, int size, 
		const char **result, int *retsize, 
		CSession* psession, struct TCPConn con) 
{
	switch(rule) {

#ifdef CONVERT_ZJ
		case RULE_ZJ:
			return Convert_zj(srcdata, size, result, retsize, psession, con);
#endif

#ifdef CONVERT_SF
		case RULE_SF:
			return Convert_sf(srcdata, size, result, retsize, psession, con);
#endif

		case RULE_EMPTY:
		default:
			return Convert_empty(srcdata, size, result, retsize);
	}
}

