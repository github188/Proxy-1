#include "BackupModule.h"

#include "DataCache.h"
#include "DBIOThread.h"
#include "RotationBackup.h"
#include "RotationFullBackup.h"
#include "ResumeTransfer.h"
#include "ClearDirThread.h"

CBackupModule::CBackupModule(
		int maxBackupNum, 
		const string& strDir/*="./backup"*/, 
		const string& strDBname/*=record.db*/, 
		const string& strFullDBname/*="record_full.db"*/)
{
    m_pDataCache = new CDataCache;
    m_pDBIOThread = new CDBIOThread(strDBname, maxBackupNum);
    m_pRotationBackup = new CRotationBackup(m_pDBIOThread, strDir);
    m_pRotationFullBackup = new CRotationFullBackup(strFullDBname, maxBackupNum, strDir);
    m_pResumeTransfer = new CResumeTransfer(m_pDBIOThread);
    m_pClearDirThread = new CClearDirThread(strDir);
}


CBackupModule::~CBackupModule()
{
    delete m_pDataCache;
    m_pDataCache = NULL;

    delete m_pDBIOThread;
    m_pDBIOThread = NULL;

    delete m_pRotationBackup;
    m_pRotationBackup = NULL;

    delete m_pRotationFullBackup;
    m_pRotationFullBackup = NULL;

    delete m_pResumeTransfer;
    m_pResumeTransfer = NULL;

    delete m_pClearDirThread;
    m_pClearDirThread = NULL;
}

void CBackupModule::Start()
{
    m_pDataCache->SetParent(this);
    m_pDataCache->Start();

    m_pDBIOThread->Start();

    m_pResumeTransfer->SetParent(this);
    m_pResumeTransfer->Start();
    m_pClearDirThread->Start();
}

