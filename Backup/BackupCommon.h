#ifndef BACKUP_COMMON_H
#define BACKUP_COMMON_H

#include <stdio.h>
#include <string>
using namespace std;

class CCar;

struct Record
{
    int primaryKeyID;
    string strImageTime;
    string strImageFile;
    string strRedBeginTime;
    string strVideoBeginTime;
    string strVideoEndTime;
    string strVideoFile;
    CCar* pCar;
};

#endif //BACKUP_COMMON_H

