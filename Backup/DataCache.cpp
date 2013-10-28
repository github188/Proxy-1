#include "DataCache.h"

#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

#include "../include/Car.h"
#include "BackupModule.h"
#include "ResumeTransfer.h"
#include "RotationFullBackup.h"
#include "RotationBackup.h"
#include "../Log/Log.h"

CDataCache::CDataCache():
m_bQuit(false),
m_pBackupModule(NULL),
m_cond(m_mutex)
{}

CDataCache::~CDataCache()
{
    m_bQuit = true;

    m_mutex.Lock();
    while (!m_lDataList.empty())
    {
        Data data = m_lDataList.front();
        m_lDataList.pop_front();

        delete data.record.pCar;
    }
    m_cond.Wake();
    m_mutex.Unlock();

    Join();
}


//pCar为实时数据
void CDataCache::Append(const CCar* pCar)
{
    assert(pCar != NULL);

    if (m_pBackupModule!=NULL && m_pBackupModule->GetRotationFullBackup()!=NULL)
    {
        m_pBackupModule->GetRotationFullBackup()->Backup(pCar);
    }

    Data data;
    data.t = time(NULL);
    data.backupData = false;
    data.record.primaryKeyID = -1;
    data.record.pCar = new CCar(*pCar);
    data.record.pCar->CreatePacketID(pCar, data.record.pCar->packetID);

    CMutexLock lock(m_mutex);
    if (m_lDataList.empty())
    {
        m_lDataList.push_back(data);
        m_cond.Wake();
    }
    else
    {
        m_lDataList.push_back(data);
    }
}


//record为备份数据
void CDataCache::Append(const Record& record)
{
    Data data;
    data.t = time(NULL);
    data.backupData = true;
    data.record = record;
    data.record.pCar = new CCar(*(record.pCar));
    data.record.pCar->CreatePacketID(record.pCar, data.record.pCar->packetID);

    CMutexLock lock(m_mutex);
    if (m_lDataList.empty())
    {
        m_lDataList.push_back(data);
        m_cond.Wake();
    }
    else
    {
        m_lDataList.push_back(data);
    }
}


//根据packetID删除缓存中的数据，如果此数据为备份数据，还得清理备份记录
bool CDataCache::Remove(const char* packetID)
{
    Record record;
    bool removeBackupData = false;

    m_mutex.Lock();

    list<Data>::iterator itBegin = m_lDataList.begin();
    list<Data>::iterator itEnd = m_lDataList.end();
    for (; itBegin != itEnd; itBegin++)
    {
        if (itBegin->record.pCar != NULL && strcmp(itBegin->record.pCar->packetID.c_str(), packetID) == 0)
        {
            if (!itBegin->backupData)
            {
                if (itBegin->record.pCar != NULL)
                {
                    delete itBegin->record.pCar;
                }
            }
            else
            {
                removeBackupData = true;
                record = itBegin->record;
            }
            itBegin = m_lDataList.erase(itBegin);
            break;
        }
    }

    m_mutex.Unlock();

    if (removeBackupData)
    {
        if (m_pBackupModule != NULL && m_pBackupModule->GetResumeTransfer() != NULL)
        {
            m_pBackupModule->GetResumeTransfer()->OnResumeTransfer(record);
        }

        if (record.pCar != NULL)
        {
            delete record.pCar;
        }
    }

    return removeBackupData;
}


bool CDataCache::Exist(int primaryKeyID)
{
    bool bExist = false;

    CMutexLock lock(m_mutex);

    list<Data>::iterator itBegin = m_lDataList.begin();
    list<Data>::iterator itEnd = m_lDataList.end();
    for (; itBegin != itEnd; itBegin++)
    {
        if (itBegin->record.primaryKeyID == primaryKeyID)
        {
            bExist = true;
            break;
        }
    }

    return bExist;
}

void CDataCache::Run()
{
    m_bQuit = false;
    while (!m_bQuit)
    {
        m_mutex.Lock();
        if (m_lDataList.empty())
        {
            m_cond.Wait();
        }

        time_t t = time(NULL);
        list<Data>::iterator itbegin = m_lDataList.begin();
        list<Data>::iterator itend = m_lDataList.end();
        for (; itbegin!=itend; itbegin++)
        {
            if (abs(t - itbegin->t) < TIME_OUT)
            {
                break;
            }

            if (!itbegin->backupData)
            {
                Backup(itbegin->record.pCar);
            }

            if (itbegin->record.pCar != NULL)
            {
                delete itbegin->record.pCar;
            }
            itbegin = m_lDataList.erase(itbegin);
        }

        m_mutex.Unlock();

        sleep(10);
    }
}

void CDataCache::Backup(const CCar* pCar)
{
    if (m_pBackupModule != NULL && m_pBackupModule->GetRotationBackup() != NULL)
    {
        m_pBackupModule->GetRotationBackup()->Backup(pCar);
    }
}

