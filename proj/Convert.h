
#ifndef CONVERT_H
#define CONVERT_H

#include "../config/config.h"

/* Convert_empty : 一个空的转换规则
 * 作用： 将数据源的指针和大小复制到结果中
 */
inline int Convert_empty(const char *srcdata, int size,
		const char *&result, int &retsize)
{
	result = srcdata;
	retsize = size;
	return retsize;
}

/* Convert: 转换数据
 * rule: 转换的规则，默认是一个空规则
 * srcdata, size: 源数据的内容和长度
 * result, retsize [out] : 转换后的数据内容和长度
 */
inline int Convert(enum CVT_RULE rule, 
		const char *srcdata, int size, 
		const char *&result, int &retsize) 
{
	switch(rule) {
		case RULE_EMPTY:
		default:
			return Convert_empty(srcdata, size, result, retsize);
	}
}

#endif //end of CONVERT_H

