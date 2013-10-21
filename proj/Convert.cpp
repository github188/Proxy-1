
#include "Convert.h"

#ifdef CONVERT_ZJ
int Convert_zj(enum CVT_RULE rule, const char *srcdata, 
		int size const char *&ret, int &retsize);
#endif 

int Convert(enum CVT_RULE rule, 
		const char *srcdata, int size, 
		const char *&result, int &retsize) 
{
	switch(rule) {
#ifdef CONVERT_ZJ
		case RULE_ZJ:
			break;
#endif
		case RULE_EMPTY:
		default:
			return Convert_empty(srcdata, size, result, retsize);
	}
}

#ifdef CONVERT_ZJ
int Convert_zj(enum CVT_RULE rule, const char *srcdata, 
		int size const char *&ret, int &retsize)
{

}
#endif 
