//******************************************************************
//修正GetProfileString函数最后有可能多取了一个\n bug
//新增加LocalFilename2RemoteFilename和RemoteFilename2LocalFilename函数by jychen
//*******************************************************************
#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>
using namespace std;

#include "pthread.h"

class CPtrList
{
public:
    CPtrList();
    ~CPtrList();

    bool AddTail(void* ptr);
    void* RemoveHead();
    int GetCount();

    void* GetAt(int index);
    void* RemoveAt(int index);

    bool InsertAt(void* ptr, int index);

    void* m_ptr[1024];
    int m_nCount;

    pthread_mutex_t m_mutex;
};

bool GetProfileString(char key[], char value[], char str[], int strlen, const char filename[]);
bool PutProfileString(char key[], char value[], const char str[], const char filename[]);
int min(int a1, int a2);
int max(int a1, int a2);

string LocalFilename2RemoteFilename(const string &strLocalFile, string &strRemoteFile);
string RemoteFilename2LocalFilename(const string &strRemoteFile, string &strLocalFile);

#endif
