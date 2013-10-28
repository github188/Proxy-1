#include "ResumeTransfer.h"

#include "DataCache.h"
#include "BackupModule.h"
#include "../include/Car.h"
#include "DBIOThread.h"
#include "../Tool/dir.h"
#include "../Tool/file.h"
#include "../Tool/stltool.h"
#include "../Log/Log.h"

CResumeTransfer::CResumeTransfer(CDBIOThread* pDBIOThread):
m_pBackupModule(NULL),
m_bQuit(false),
m_pDBIOThread(pDBIOThread),
m_pJpeg(NULL)
{
    m_pJpeg = new char[2*1024*1024];

    m_pVideoData = new char[8*1024*1024];
}


CResumeTransfer::~CResumeTransfer()
{
    m_bQuit = true;
    Join();

    delete[] m_pJpeg;
    m_pJpeg = NULL;

    m_pBackupModule = NULL;
    m_pDBIOThread = NULL;
}


//void CResumeTransfer::GetJpeg(const char* filename, char* jpeg, int& len)
//{
//    len = 0;
//
//    FILE* fp = NULL;
//    fp = fopen(filename, "rb");
//    if (fp != NULL)
//    {
//        fseek(fp, 0, SEEK_END);
//        len = ftell(fp);
//        fseek(fp, 0, SEEK_SET);
//
//        fread(jpeg, len, 1, fp);
//
//        fclose(fp);
//    }
//}

void CResumeTransfer::GetFileData(const char* filename, char* data, int& len)
{
    len = 0;

    FILE* fp = NULL;
    fp = fopen(filename, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        int offset = 0;
        int totalRead = 0;
        while (totalRead < len)
        {
            offset = len-4096? 4096:len-4096;
            fread(data+totalRead, offset, 1, fp);
            totalRead += offset;
        }

        fclose(fp);
    }
}


void CResumeTransfer::OnResumeTransfer(const Record& record)
{
    printf("transfer ok, primary id = %d, localFile = %s\n", record.primaryKeyID, record.strImageFile.c_str());

    /*
    // 不再删除图片及视频文件
    CFile file;

    //remove image
    vector<string> files;
    Split(record.strImageFile, ";", files);

    for (size_t i = 0; i < files.size(); i++)
    {
        int ret = file.RemoveFile(files[i]);
        if (ret == -1)
        {
            ERRLOG("remove file %s error\n", record.strImageFile.c_str());
        }
    }

    //remove video
    if (record.pCar->videoNum > 0)
    {
        vector < string > videoFiles;
        Split(record.strVideoFile, ";", videoFiles);

        for (size_t i = 0; i < videoFiles.size(); i++)
        {
            int ret = file.RemoveFile(videoFiles[i]);
            if (ret == -1)
            {
                ERRLOG("remove file %s error\n", record.strVideoFile.c_str());
            }
        }
    }
    */

    if (m_pBackupModule != NULL && m_pBackupModule->GetDBIOThread() != NULL)
    {
        m_pDBIOThread->DeleteRecord(record.primaryKeyID);
    }

    if (m_pBackupModule != NULL && m_pBackupModule->GetDBIOThread() != NULL)
    {
        m_pDBIOThread->GetFirstRecord(GetRecordCb, this);
    }

}



void CResumeTransfer::GetRecordCb(const Record& record, void* param)
{
    CResumeTransfer* pThis = (CResumeTransfer*)param;
    pThis->GetRecordCb(record);
}


void CResumeTransfer::ParseTime(const string& str, struct timeval& tv)
{
    vector < string > strTimes;
    Split(str, ",", strTimes);
    assert(strTimes.size() == 2);
    tv.tv_sec = atoi(strTimes[0].c_str());
    tv.tv_usec = atoi(strTimes[1].c_str());
}


void CResumeTransfer::GetRecordCb(const Record& record)
{
    //printf("get record from db, id=%d, imageFile=%s\n", record.primaryKeyID, record.strImageFile.c_str());
    if (record.primaryKeyID == -1)
    {
        //printf("GetRecordCb id = %d\n", record.primaryKeyID);
        return;
    }

    bool bExist = false;
    if (m_pBackupModule!=NULL && m_pBackupModule->GetDataCache()!=NULL)
    {
        bExist = m_pBackupModule->GetDataCache()->Exist(record.primaryKeyID);
    }

    if (bExist)
    {
        return;
    }


    int i=0;
    Record tmpRecord = record;
    tmpRecord.pCar = new CCar(*(record.pCar));

    //image
    vector<string> files, strTimes;
    Split(record.strImageFile, ";", files);
    Split(record.strImageTime, ";", strTimes);

    assert(files.size() == strTimes.size());

    int len = 0;
    bool bFileExist = false;
    for (size_t i=0; i<files.size(); i++)
    {
        //printf("file name = %s\n", files[i].c_str());

        //parse time
        ParseTime(strTimes[i], tmpRecord.pCar->time[i]);

        //read jpeg to memory and copy to pCar
        GetFileData(files[i].c_str(), m_pJpeg, len);
        if (len > 0)
        {
            tmpRecord.pCar->imageLen[i] = len;
            tmpRecord.pCar->image[i] = new char[len];
            memcpy(tmpRecord.pCar->image[i], m_pJpeg, len);
            bFileExist = true;
        }
        else
        {
            tmpRecord.pCar->imageNum--;
            tmpRecord.pCar->imageLen[i] = 0;
            tmpRecord.pCar->image[i] = NULL;

            WARNLOG("image file %s has not exist!\n", files[i].c_str());
        }
    }


    //video
    vector<string> redBeginTimes, videoBeginTimes, videoEndTimes, videoFiles;
    Split(record.strRedBeginTime, ";", redBeginTimes);
    ParseTime(redBeginTimes[0], tmpRecord.pCar->tmRedBegin);

    if (record.pCar->videoNum > 0)
    {
        Split(record.strVideoBeginTime, ";", videoBeginTimes);
        Split(record.strVideoEndTime, ";", videoEndTimes);
        Split(record.strVideoFile, ";", videoFiles);
    }

    for (i=0; i<record.pCar->videoNum; i++)
    {
        //parse time
        ParseTime(videoBeginTimes[i], tmpRecord.pCar->tmVideoBegin[i]);
        ParseTime(videoEndTimes[i], tmpRecord.pCar->tmVideoEnd[i]);

        GetFileData(videoFiles[i].c_str(), m_pVideoData, len);
        if (len > 0)
        {
            tmpRecord.pCar->videoLen[i] = len;
            tmpRecord.pCar->videoData[i] = new char[len];
            memcpy(tmpRecord.pCar->videoData[i], m_pVideoData, len);
            bFileExist = true;
        }
        else
        {
            tmpRecord.pCar->videoNum--;
            tmpRecord.pCar->videoLen[i] = 0;
            tmpRecord.pCar->videoData[i] = NULL;

            WARNLOG("video file %s has not exist!\n", videoFiles[i].c_str());
        }
    }


    if (bFileExist)
    {
        if (m_pBackupModule != NULL && m_pBackupModule->GetDataCache() != NULL \
                        && m_pBackupModule->GetCallBack() != NULL)
        {
            m_pBackupModule->GetDataCache()->Append(tmpRecord);
            (m_pBackupModule->GetCallBack())(tmpRecord.pCar);
        }
    }
    else
    {
        m_pDBIOThread->DeleteRecord(tmpRecord.primaryKeyID);
    }

    delete tmpRecord.pCar;
}

void CResumeTransfer::Run()
{
    m_bQuit = false;
    while (!m_bQuit)
    {
        if (m_pBackupModule != NULL && m_pBackupModule->GetDBIOThread() != NULL)
        {
            m_pBackupModule->GetDBIOThread()->GetFirstRecord(GetRecordCb, this);
        }

        sleep(10);
    }
}
