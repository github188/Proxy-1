#ifndef TIMECLASS_H
#define TIMECLASS_H

//#include "String2Type.h"

#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

int DiffTime(struct timeval btv, struct timeval etv);
bool SetTime(time_t t);

class CTime
{
public:
	CTime();
	~CTime();

	CTime(const time_t &t);
	CTime(const struct timeval &tv);
    string GetTimeString();
	time_t UnixTime(int year, int month, int day, int hour, int minute, int second);
    static string getRandStr()
    {
        char sr[5];
        srand((int)time(0));
        int s=rand()%1000;
        sprintf(sr,"%.3d",s);
        string rr = sr;
        return rr;
    }

	int Year();
	int Month();
	int Day();

	int Hour();
	int Minute();
	int Second();

	int MilliSecond();

private:
	time_t m_t;
	struct timeval m_tv;
};

#endif //TIMECLASS_H


