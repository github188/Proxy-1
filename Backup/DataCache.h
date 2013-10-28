#ifndef KISE_BACKUP_SOCK_DATACACHE_H
#define KISE_BACKUP_SOCK_DATACACHE_H

#include <list>
#include <string>
using namespace std;

#include "BackupCommon.h"
#include "../Thread/Mutex.h"
#include "../Thread/AbstractThread.h"

class CCar;
class CBackupModule;

class CDataCache : public CAbstractThread
{
public:
    CDataCache();
    ~CDataCache();

    void SetParent(CBackupModule* pBackupModule) { m_pBackupModule = pBackupModule; }

    void Append(const CCar* pCar);
    void Append(const Record& record);

    bool Remove(const char* packetID);

    bool Exist(int primaryKeyID);

protected:
    void Quit() { m_bQuit = false; }
    void Run();

private:
    void Backup(const CCar* pCar);

    struct Data
    {
        time_t t; //进入datacache的时间
        bool backupData; //标识是备份数据还是实时数据
        Record record;
    };

private:
    const static int TIME_OUT = 5;//second
    bool m_bQuit;
    CBackupModule* m_pBackupModule;
    CMutex m_mutex;
    CCondition m_cond;
    list<Data> m_lDataList;
};


#endif //KISE_BACKUP_SOCK_DATACACHE_H

