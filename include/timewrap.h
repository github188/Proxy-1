
#ifndef TIME_WRAP_H
#define TIME_WRAP_H

#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

/* 根据年月日时分秒毫秒转换一个 timeval 结构体
 * 如依次传递 2013 10 15 9 45 23 999
 */
inline struct timeval get_timeval_by_date(int yy, int mm, int dd, 
		int hh, int min, int sec, int ms )
{
	struct tm temp;
	struct timeval ret;
	temp.tm_year = yy-1900;
	temp.tm_mon = mm-1;
	temp.tm_mday = dd;
	temp.tm_hour = hh;
	temp.tm_min = min;
	temp.tm_sec = sec;

	ret.tv_sec = mktime(&temp);
	ret.tv_usec = ms * 1000;
	return ret;
}

inline void get_date_by_timeval_net(struct timeval val, short *yy, short *mm,
		short *dd, short *hh, short *min, short *sec, short *ms)
{
	struct tm *temp;
	temp = localtime( &(val.tv_sec) );

	*yy =htons( (short) temp->tm_year + 1900 );
	*mm =htons( (short) temp->tm_mon + 1 );
	*dd = htons( (short) temp->tm_mday);
	*hh = htons( (short) temp->tm_hour);
	*min = htons( (short) temp->tm_min);
	*sec = htons( (short) temp->tm_sec);

	*ms = htons( (short) (val.tv_usec / 1000) );
}


#endif
