#ifndef ROTATION_BACKUP_H
#define ROTATION_BACKUP_H

#include <string>
#include <vector>
using namespace std;

#include "DBIOThread.h"

class CCar;

class CRotationBackup
{
public:
    CRotationBackup(CDBIOThread* pDBIOThread, const string& strDir="./backup");
    ~CRotationBackup();

    void Backup(const CCar* pCar);

    static void DelFileCb(const string& strImageFile, const string& strVideoFile, void* param);
//private:
    //void CreateLocalFilename(const CCar* pCar, vector<string>& imageFiles, vector<string>& videoFiles);

private:
    string m_strBackupDir;

    CDBIOThread* m_pDBIOThread;
};

#endif //ROTATION_BACKUP_H
