
#include "Convert.h"

#ifdef CONVERT_ZJ
extern int Convert_zj(const char *srcdata, int size, 
		const char *&ret, int &retsize, CSession *psession);
#endif 

int Convert(enum CVT_RULE rule, 
		const char *srcdata, int size, 
		const char *&result, int &retsize, CSession* psession) 
{
	switch(rule) {
#ifdef CONVERT_ZJ
		case RULE_ZJ:
			return Convert_zj(srcdata, size, result, retsize, psession);
#endif
		case RULE_EMPTY:
		default:
			return Convert_empty(srcdata, size, result, retsize);
	}
}

