#include "Log.h"

#include "../Tool/dir.h"
#include "../Tool/file.h"

namespace Kise{
namespace Log{

CLog* CLog::m_pLog = NULL;

CLog& CLog::Instance()
{
    if (m_pLog == NULL)
    {
        static CLog log;
        m_pLog = &log;
    }

    return *m_pLog;
}

CLog::CLog()
{
    m_strLogPath = "./Log/Main/";

    m_nMaxFileSize = 4*1024*1024;
    m_nLogDayNum = 7;
    m_nRecordIndex = 0;

    for (int i=0; i<RECORD_NUM; i++)
    {
        m_pRecord[i] = new char[RECORD_LEN];
    }

    m_pTimer = new CTimer;
    m_pTimer->RegisterTimeoutCb(Timeout, this);
}


CLog::~CLog()
{
    m_mutex.Lock();
    for (int i = 0; i < RECORD_NUM; i++)
    {
        delete[] m_pRecord[i];
    }
    m_nRecordIndex = 0;
    m_mutex.Unlock();

    delete m_pTimer;
}


void CLog::Timeout(void* param)
{
    CLog* pThis = (CLog*)param;
    pThis->Timeout();
}


void CLog::Timeout()
{
    CMutexLock lock(m_mutex);
    WriteLog(m_pRecord, m_nRecordIndex);
    m_nRecordIndex = 0;
}


void CLog::FormatString(char* buf, Level level, \
        const char* file, const int line, const char* func, \
        const char* format, va_list ap)
{
    assert(buf != NULL);
    assert(file != NULL);
    assert(func != NULL);

    time_t seconds;
    struct tm* ptime;

    seconds = time((time_t*)(NULL));
    ptime = localtime(&seconds);


    const char *strLevel;
    switch (level)
    {
    case PrintLevel:
        strLevel = "msg";
        break;
    case WarnLevel:
        strLevel = "warn";
        break;
    case ErrLevel:
        strLevel = "err";
        break;
    case TraceLevel:
        strLevel = "trace";
        break;
    default:
        strLevel = "???";
        break;
    }

    //{时间}{日志等级}{文件名}{行号}{函数名}日志信息
    sprintf(buf,"{time:%04d-%02d-%02d %02d:%02d:%02d}{level:%s}{file:%s}{line:%d}{func:%s}", \
            ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday, \
            ptime->tm_hour, ptime->tm_min, ptime->tm_sec, \
            strLevel, file, line, func);
    int headLen = strlen(buf);
    vsprintf(buf+headLen, format, ap);

    return;
}


void CLog::Log(Level lev, const char* file, const int line, const char* func, \
        const char* format, ...)
{
    char buf[RECORD_LEN];

    va_list ap;
    va_start(ap, format);

    FormatString(buf, lev, file, line, func, format, ap);

    va_end(ap);

    Log(buf);
}


void CLog::Log(const char* msg)
{
    assert(msg != NULL);

    CMutexLock lock(m_mutex);
    assert(m_nRecordIndex < RECORD_NUM);

    strcpy(m_pRecord[m_nRecordIndex++], msg);

    if (m_nRecordIndex == 1)
    {
        m_pTimer->Start(TIMEOUT);
        return;
    }

    if (m_nRecordIndex == RECORD_NUM)
    {
        WriteLog(m_pRecord, RECORD_NUM);
        m_nRecordIndex = 0;
        return;
    }
}


void CLog::WriteLog(char* msg[], int num)
{
    if (num == 0)
    {
        return;
    }

    //建立目录
    CDir dir;
    dir.MakeDir(m_strLogPath, true);


    //文件数目超过最大记录天数，则删除第一个文件
    list<string> fileList, dirList;
    dir.ReadDirByVersionsort(m_strLogPath, fileList, dirList);
    if (fileList.size() > (unsigned)m_nLogDayNum)
    {
        CFile file;
        file.RemoveFile(fileList.front());
    }


    //构建文件名:目录+2011-04-14_index.log
    string filename = GetLogFilename();

    //写记录
    FILE* fp = fopen(filename.c_str(), "a+");
    if (fp != NULL)
    {
        for (int i=0; i<num; i++)
        {
            int len = fwrite(msg[i], strlen(msg[i]), 1, fp);
            if (len == -1)
            {
                perror("fwrite:");
            }
            //printf("log msg[%d] = %s, wlen = %d\n", i, msg[i], len);
        }
        fclose(fp);
    }
}


long CLog::FileSize(const char* filename)
{
    long filesize = 0;

    FILE* fp = fopen(filename, "r");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fclose(fp);
    }

    return filesize;
}


string CLog::GetLogFilename()
{
    static int index = 0;
    char filename[256];

    time_t seconds;
    struct tm* ptime;

    seconds = time((time_t*) (NULL));
    ptime = localtime(&seconds);

    //优先检查当前记录的文件，文件大小超过了再去顺序检查其它的
    sprintf(filename,"%s/%04d-%02d-%02d_%d.log", m_strLogPath.c_str(), \
                    ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday, index);

    long filesize = FileSize(filename);
    if (filesize < m_nMaxFileSize)
    {
        return filename;
    }


    for (int i=0; i<10000; i++)
    {
        sprintf(filename,"%s/%04d-%02d-%02d_%d.log", m_strLogPath.c_str(), \
                ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday, i);

        filesize = FileSize(filename);

        if (filesize < m_nMaxFileSize)
        {
            index = i;
            break;
        }
    }

    return filename;
}



CTimer::CTimer():m_bQuit(false),
m_cond(m_mutex),
m_bTimerRun(false),
m_nTimeout(1),
m_pParam(NULL)
{
    CAbstractThread::Start();
}


CTimer::~CTimer()
{
    Quit();
    CMutexLock lock(m_mutex);
    m_bTimerRun = true;
    m_cond.Wake();

    Join();
}


void CTimer::RegisterTimeoutCb(TimeoutCb timeoutcb, void* param)
{
    m_timeoutcb = timeoutcb;
    m_pParam = param;
}


void CTimer::Start(int sec)
{
    CMutexLock lock(m_mutex);
    m_nTimeout = sec;
    if (!m_bTimerRun)
    {
        m_bTimerRun = true;
        m_cond.Wake();
    }
}


void CTimer::Stop()
{
    CMutexLock lock(m_mutex);
    m_bTimerRun = false;
}


void CTimer::Get()
{
    CMutexLock lock(m_mutex);
    if (!m_bTimerRun)
        m_cond.Wait();
}

void CTimer::Run()
{
    m_bQuit = false;

    while (!m_bQuit)
    {

        Get();

        sleep(m_nTimeout);

        if (m_timeoutcb != NULL)
        {

            m_timeoutcb(m_pParam);

        }
        printf("run\n");


        Stop();
    }
}


} //Log
} //Kise
