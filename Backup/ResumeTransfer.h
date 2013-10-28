#ifndef RESUME_TRANSFER_H
#define RESUME_TRANSFER_H

#include <stdio.h>
#include <string>
#include <list>
using namespace std;

#include "BackupCommon.h"
#include "../Thread/AbstractThread.h"
#include "../Thread/Mutex.h"

class CBackupModule;
class CDBIOThread;

class CResumeTransfer : public CAbstractThread
{
public:
    CResumeTransfer(CDBIOThread* pDBIOThread);
    ~CResumeTransfer();

    void SetParent(CBackupModule* pBackupModule) { m_pBackupModule = pBackupModule; }
    void OnResumeTransfer(const Record& record);

    static void GetRecordCb(const Record& record, void* param);
    void GetRecordCb(const Record& record);

protected:
    void Quit() { m_bQuit = true; }
    void Run();

private:
    void ParseTime(const string& str, struct timeval& tv);
    void GetFileData(const char* filename, char* data, int& len);

private:
    CBackupModule* m_pBackupModule;
    bool m_bQuit;
    CDBIOThread* m_pDBIOThread;
    char* m_pJpeg;
    char* m_pVideoData;
};

#endif //RESUME_TRANSFER_H


