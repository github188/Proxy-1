#include "DBIOThread.h"

#include "../include/Car.h"
#include "Database.h"
#include "../Log/Log.h"

CDBIOThread::CDBIOThread(const string& strDbName, int maxBackupNum):
m_pDatabase(NULL),
m_nMaxBackupNum(maxBackupNum),
m_bQuit(false),
m_cond(m_mutex)
{
    m_pDatabase = new CDatabase(strDbName);
}


CDBIOThread::~CDBIOThread()
{
    m_bQuit = true;

    m_mutex.Lock();

    while (!m_lCmdList.empty())
    {
        DBCmd cmd = m_lCmdList.front();
        m_lCmdList.pop_front();

        delete cmd.record.pCar;
    }

    m_cond.Wake();
    m_mutex.Unlock();

    Join();

    delete m_pDatabase;
}

void CDBIOThread::Run()
{
    m_bQuit = false;
    while (!m_bQuit)
    {
        DBCmd cmd = GetDBCmd();
        switch (cmd.cmdType)
        {
        case INSERT_RECORD:
            InsertRecord(cmd);
            break;
        case DELETE_RECORD:
            DeleteRecord(cmd);
            break;
        case GET_RECORD:
            GetFirstRecord(cmd);
            break;
        default:
            break;
        }

        usleep(100*1000);
    }
}

CDBIOThread::DBCmd CDBIOThread::GetDBCmd()
{
    CMutexLock lock(m_mutex);
    if (m_lCmdList.empty())
    {
        m_cond.Wait();
    }

    DBCmd cmd;
    if (!m_lCmdList.empty())
    {
        cmd = m_lCmdList.front();
        m_lCmdList.pop_front();
    }

    return cmd;
}

void CDBIOThread::InsertRecord(const Record& record, DelFileCb delCb, void* param)
{
    DBCmd cmd;
    cmd.cmdType = INSERT_RECORD;
    cmd.record = record;
    cmd.record.pCar = new CCar(*(record.pCar));
    cmd.getCb = NULL;
    cmd.delCb = delCb;
    cmd.param = param;

    CMutexLock lock(m_mutex);
    if (m_lCmdList.empty())
    {
        m_lCmdList.push_back(cmd);
        m_cond.Wake();
    }
    else
    {
        m_lCmdList.push_back(cmd);
    }
}


void CDBIOThread::DeleteRecord(int id)
{
    DBCmd cmd;
    cmd.cmdType = DELETE_RECORD;
    cmd.record.primaryKeyID = id;

    CMutexLock lock(m_mutex);
    if (m_lCmdList.empty())
    {
        m_lCmdList.push_back(cmd);
        m_cond.Wake();
    }
    else
    {
        m_lCmdList.push_back(cmd);
    }
}


void CDBIOThread::GetFirstRecord(GetRecordCb getCb, void* param/*=NULL*/)
{
    DBCmd cmd;
    cmd.cmdType = GET_RECORD;
    cmd.getCb = getCb;
    cmd.param = param;

    CMutexLock lock(m_mutex);
    if (m_lCmdList.empty())
    {
        m_lCmdList.push_back(cmd);
        m_cond.Wake();
    }
    else
    {
        m_lCmdList.push_back(cmd);
    }
}


void CDBIOThread::InsertRecord(const DBCmd& cmd)
{
    m_pDatabase->InsertRecord(cmd.record);

    delete cmd.record.pCar;

    int total = m_pDatabase->GetTotal();
    if (total > m_nMaxBackupNum)
    {
        for (int i = total-m_nMaxBackupNum; i > 0; i--)
        {
            string strImageFile, strVideoFile;
            m_pDatabase->DeleteHeadRecord(strImageFile, strVideoFile);

            if (cmd.delCb != NULL)
            {
                (*cmd.delCb)(strImageFile, strVideoFile, cmd.param);
            }
        }
    }
}


void CDBIOThread::DeleteRecord(const DBCmd& cmd)
{
    m_pDatabase->DeleteRecord(cmd.record.primaryKeyID);
}


void CDBIOThread::GetFirstRecord(DBCmd& cmd)
{
    cmd.record.pCar = new CCar;
    m_pDatabase->GetFirstRecord(cmd.record);
    if (cmd.getCb != NULL)
    {
        (*cmd.getCb)(cmd.record, cmd.param);
    }
    delete cmd.record.pCar;
}





