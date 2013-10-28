#ifndef DBIOTHREAD_H
#define DBIOTHREAD_H

#include <stdio.h>
#include <list>
#include <string>
using namespace std;

#include "BackupCommon.h"
#include "../Thread/AbstractThread.h"
#include "../Thread/Mutex.h"

//如果id为-1，则认为没有数据可取了
typedef void (*GetRecordCb)(const Record& record, void* param);
typedef void (*DelFileCb)(const string& strImageFile, const string& strVideoFile, void* param);

class CDatabase;

class CDBIOThread : public CAbstractThread
{
public:
    enum DBCmdType{GET_RECORD, INSERT_RECORD, DELETE_RECORD};

    CDBIOThread(const string& strDbName, int maxBackupNum);
    ~CDBIOThread();

    void InsertRecord(const Record& record, DelFileCb delCb, void* param);
    void DeleteRecord(int id);
    void GetFirstRecord(GetRecordCb getCb, void* param = NULL);

protected:
    struct DBCmd
    {
        DBCmdType cmdType;
        Record record;
        GetRecordCb getCb;
        DelFileCb delCb;
        void* param;
    };

    void Run();

private:
    void Quit() { m_bQuit = true; }
    DBCmd GetDBCmd();

    void InsertRecord(const DBCmd& cmd);
    void DeleteRecord(const DBCmd& cmd);
    void GetFirstRecord(DBCmd& cmd);

private:
    CDatabase* m_pDatabase;
    int m_nMaxBackupNum;
    bool m_bQuit;

    CMutex m_mutex;
    CCondition m_cond;
    list<DBCmd> m_lCmdList;
};

#endif //DBIOTHREAD_H
