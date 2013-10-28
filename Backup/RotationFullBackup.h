#ifndef ROTATION_FULL_BACKUP_H
#define ROTATION_FULL_BACKUP_H

#include <string>
#include <vector>
using namespace std;

#include "DBIOThread.h"

class CCar;
class CDatabase;
struct Record;

class CRotationFullBackup
{
public:
    CRotationFullBackup(const string& fullBackupDbname, int maxBackupNum, const string& strDir="./backup");
    ~CRotationFullBackup();

    void Backup(const CCar* pCar);

    void DelFile(const string& strImageFile, const string& strVideoFile);
private:
    void InsertRecord(const Record& record);
private:
    string m_strBackupDir;
    int m_nMaxBackupNum;
    CDatabase* m_pDatabase;
};

#endif //ROTATION_FULL_BACKUP_H
