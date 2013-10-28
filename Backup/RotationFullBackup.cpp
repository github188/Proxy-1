#include "RotationFullBackup.h"

#include "BackupCommon.h"
#include "../Tool/dir.h"
#include "../Tool/file.h"
#include "../Tool/timeclass.h"
#include "../Tool/stltool.h"
#include "../include/Car.h"
#include "BackupUtils.h"
#include "Database.h"

void CRotationFullBackup::DelFile(const string& strImageFile, const string& strVideoFile)
{
    //printf("DelFileCb filename = %s\n", strImageFile.c_str());

    vector<string> imageFiles, videoFiles;
    Split(strImageFile, ";", imageFiles);
    Split(strVideoFile, ";", videoFiles);

    CFile file;
    for (size_t i=0; i<imageFiles.size(); i++)
    {
        file.RemoveFile(imageFiles[i]);
    }

    for (size_t i = 0; i < videoFiles.size(); i++)
    {
        file.RemoveFile(videoFiles[i]);
    }
}

CRotationFullBackup::CRotationFullBackup(const string& fullBackupDbname, int maxBackupNum, const string& strDir/*="./backup"*/):
m_strBackupDir(strDir),
m_nMaxBackupNum(maxBackupNum)
{
    CDir dir;
    dir.MakeDir(m_strBackupDir, true);

    m_pDatabase = new CDatabase(fullBackupDbname);
}


CRotationFullBackup::~CRotationFullBackup()
{
    delete m_pDatabase;
}


void CRotationFullBackup::Backup(const CCar* pCar)
{
    assert(pCar != NULL);

    vector<string> imageFiles, videoFiles;
    CreateLocalFilename(m_strBackupDir, pCar, imageFiles, videoFiles);

    CFile file;
    char tmp[256];
    int i = 0;

    Record record;
    record.primaryKeyID = -1;
    //image
    for (i=0; i<pCar->imageNum; i++)
    {
        file.WriteFile(pCar->image[i], pCar->imageLen[i], imageFiles[i].c_str());

        record.strImageFile += imageFiles[i] + ";";

        snprintf(tmp, 256, "%ld,%ld;", pCar->time[i].tv_sec, pCar->time[i].tv_usec);
        record.strImageTime += tmp;
    }

    //video
    for (i=0; i<pCar->videoNum; i++)
    {
        file.WriteFile(pCar->videoData[i], pCar->videoLen[i], videoFiles[i].c_str());

        record.strVideoFile += videoFiles[i] + ";";

        snprintf(tmp, 256, "%ld,%ld;", pCar->tmVideoBegin[i].tv_sec, pCar->tmVideoBegin[i].tv_usec);
        record.strVideoBeginTime += tmp;

        snprintf(tmp, 256, "%ld,%ld;", pCar->tmVideoEnd[i].tv_sec, pCar->tmVideoEnd[i].tv_usec);
        record.strVideoEndTime += tmp;
    }

    snprintf(tmp, 256, "%ld,%ld;", pCar->tmRedBegin.tv_sec, pCar->tmRedBegin.tv_usec);
    record.strRedBeginTime += tmp;

    record.pCar = const_cast<CCar*>(pCar);

    InsertRecord(record);
}


void CRotationFullBackup::InsertRecord(const Record& record)
{
    //硬盘空间判断



    m_pDatabase->InsertRecord(record);

    int total = m_pDatabase->GetTotal();
    if (total > m_nMaxBackupNum)
    {
        int deleteNum = total-m_nMaxBackupNum;
        deleteNum = (deleteNum>100?100:deleteNum);
        for (int i = deleteNum; i > 0; i--)
        {
            string strImageFile, strVideoFile;
            m_pDatabase->DeleteHeadRecord(strImageFile, strVideoFile);

            DelFile(strImageFile, strVideoFile);
        }
    }
}

