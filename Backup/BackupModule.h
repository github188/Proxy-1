#ifndef BACKUP_H
#define BACKUP_H

#include <stdio.h>
#include <string>

#include "../include/Car.h"

using namespace std;

class CDataCache;
class CDBIOThread;
class CRotationBackup;
class CRotationFullBackup;
class CResumeTransfer;
class CClearDirThread;

typedef void (*BackupCallBack)(CCar *);

class CBackupModule
{
public:
    // strDBname为断点续传DB文件名
    // strFullDBname为全备份DB文件名
    CBackupModule( int maxBackupNum, 
			const string& strDir="./backup", 
			const string& strDBname="record.db", 
			const string& strFullDBname="record_full.db");
    ~CBackupModule();

    void Start();

    void SetSession(){}

    CDataCache* GetDataCache() { return m_pDataCache; }
    CRotationBackup* GetRotationBackup() { return m_pRotationBackup; }
    CRotationFullBackup* GetRotationFullBackup() { return m_pRotationFullBackup; }
    CResumeTransfer* GetResumeTransfer() { return m_pResumeTransfer; }
    CDBIOThread* GetDBIOThread() { return m_pDBIOThread; }

	BackupCallBack GetCallBack(void) { return m_pCallback; }
	void SetCallBack(BackupCallBack callback) { m_pCallback = callback; }

private:
    CBackupModule(const CBackupModule& rhs);
    CBackupModule& operator=(const CBackupModule& rhs);

private:
    CDataCache* m_pDataCache;
    CDBIOThread* m_pDBIOThread;
    CRotationBackup* m_pRotationBackup;
    CRotationFullBackup* m_pRotationFullBackup;
    CResumeTransfer* m_pResumeTransfer;
    CClearDirThread* m_pClearDirThread;

	BackupCallBack m_pCallback;
};

#endif //BACKUP_H

