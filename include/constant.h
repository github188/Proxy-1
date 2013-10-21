
#ifndef CONSTANT_H
#define CONSTANT_H

/* 定义支持的转换规则 */
enum CVT_RULE {
	RULE_EMPTY = 0 
#ifdef CONVERT_ZJ
		, RULE_ZJ = 1
#endif 
#ifdef CONVERT_SF
		, RULE_SF = 2
#endif
};


#endif
