
#include "timeclass.h"


//返回毫秒
//a-b
int DiffTime(struct timeval a, struct timeval b)
{
    struct timeval tv;

    if (a.tv_usec > b.tv_usec)
    {
        tv.tv_usec = a.tv_usec - b.tv_usec;
        tv.tv_sec = a.tv_sec - b.tv_sec;
    }
    else
    {
        tv.tv_sec = a.tv_sec - 1 - b.tv_sec;
        tv.tv_usec = a.tv_usec + 1000000 - b.tv_usec;
    }

    return tv.tv_sec * 1000 + (int) (tv.tv_usec / 1000);
}

bool SetTime(time_t t)
{
    int ret = stime((time_t*)&t);
    return (ret==-1)?false:true;
}


CTime::CTime()
{

}

CTime::~CTime()
{

}

CTime::CTime(const time_t &t)
{
	m_t = t;
	m_tv.tv_sec = 0;
	m_tv.tv_usec = 0;
}

CTime::CTime(const struct timeval &tv)
{
	m_tv = tv;
	m_t = 0;
}

string CTime::GetTimeString()
{
    char timeString [20];
    sprintf(timeString,"%.4d%.2d%.2d%.2d%.2d%.2d%.3d",
        this->Year(),
        this->Month(),
        this->Day(),
        this->Hour(),
        this->Minute(),
        this->Second(),
        this->MilliSecond());
    string st = string(timeString);
    return st;
}

time_t CTime::UnixTime(int year, int month, int day, int hour, int minute, int second)
{
	struct tm tmTemp;
	tmTemp.tm_year = year-1900;
	tmTemp.tm_mon = month-1;
	tmTemp.tm_mday = day;
	tmTemp.tm_hour = hour;
	tmTemp.tm_min = minute;
	tmTemp.tm_sec = second;

	return mktime(&tmTemp);
}

int CTime::Year()
{
	time_t temp;
	if (m_t != 0)
	{
		temp = m_t;
	}
	else
	{
		temp = m_tv.tv_sec;
	}

	return localtime(&temp)->tm_year+1900;
}

int CTime::Month()
{
	time_t temp;
	if (m_t != 0)
	{
		temp = m_t;
	}
	else
	{
		temp = m_tv.tv_sec;
	}

	return localtime(&temp)->tm_mon+1;
}

int CTime::Day()
{
	time_t temp;
	if (m_t != 0)
	{
		temp = m_t;
	}
	else
	{
		temp = m_tv.tv_sec;
	}

	return localtime(&temp)->tm_mday;
}

int CTime::Hour()
{
	time_t temp;
	if (m_t != 0)
	{
		temp = m_t;
	}
	else
	{
		temp = m_tv.tv_sec;
	}

	return localtime(&temp)->tm_hour;
}

int CTime::Minute()
{
	time_t temp;
	if (m_t != 0)
	{
		temp = m_t;
	}
	else
	{
		temp = m_tv.tv_sec;
	}

	return localtime(&temp)->tm_min;
}

int CTime::Second()
{
	time_t temp;
	if (m_t != 0)
	{
		temp = m_t;
	}
	else
	{
		temp = m_tv.tv_sec;
	}

	return localtime(&temp)->tm_sec;
}

int CTime::MilliSecond()
{
	return m_tv.tv_usec/1000;
}


