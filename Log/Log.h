#ifndef KISE_LOG_LOG_H
#define KISE_LOG_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <iostream>
using namespace std;

#include "../Thread/AbstractThread.h"
#include "../Thread/Mutex.h"

//接口说明：
//对外接口采用以下宏方式，WARNLOG跟ERRLOG都会写到日志文件里，暂时不支持对外配置
//默认每天一个日志文件，默认最大记录天数为7天。
//注意：每次记录的信息请不要超过1024字节，因为内部未判断
#define PRINTLOG(...) printf(__VA_ARGS__)
#define WARNLOG(...) Kise::Log::CLog::Instance().Log(Kise::Log::CLog::WarnLevel, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define ERRLOG(...) Kise::Log::CLog::Instance().Log(Kise::Log::CLog::ErrLevel, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define TRACELOG(...) //Kise::Log::CLog::Instance().Log(Kise::Log::CLog::TraceLevel, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

namespace Kise{
namespace Log{

typedef void (*TimeoutCb)(void* param);

class CTimer;

class CLog
{
public:
    static CLog& Instance();

    enum Level{PrintLevel = 0, WarnLevel = 1, ErrLevel = 2, TraceLevel = 3};


    void SetLogPath(const string& path)
    {
        m_strLogPath = path;
    }

    void SetFileSize(int size) { m_nMaxFileSize = size; }
    void SetMaxLogDayNum(int dayNum)
    {
        m_nLogDayNum = dayNum;
    }

    void Log(Level lev, const char* file, const int line, const char* func, \
            const char* format, ...);

private:
    CLog();
    ~CLog();


private:
    //根据限制的文件长度及限定的记录日志天数进行记录
    void Log(const char* msg);
    void WriteLog(char* msg[], int num);

    static void Timeout(void* param);
    void Timeout();

    void FormatString(char* buf, Level level, \
            const char* file, const int line, const char* func, \
            const char* format, va_list ap);

    string GetLogFilename();
    long FileSize(const char* filename);

private:
    static CLog* m_pLog;
    const static int RECORD_NUM = 256;    //当记录数超过此数目时，自动写入日志文件
    const static int RECORD_LEN = 1024; //每条记录最大长度，单位：字节
    const static int TIMEOUT = 10; //单位：秒

    string m_strLogPath;
    int m_nMaxFileSize;
    int m_nLogDayNum;
    char* m_pRecord[RECORD_NUM];
    int m_nRecordIndex;
    CMutex m_mutex;

    CTimer* m_pTimer;
};


//不太准的定时器
class CTimer : public CAbstractThread
{
public:
    CTimer();
    ~CTimer();

    void RegisterTimeoutCb(TimeoutCb timeoutcb, void* param);

    //启动定时器，如果定时器已经启动，将不会重新启动定时器
    void Start(int sec);

    //关闭定时器
    void Stop();

    void Quit() { m_bQuit = true; }

protected:
    void Run();

    void Get();

private:
    bool m_bQuit;

    CMutex m_mutex;
    CCondition m_cond;
    bool m_bTimerRun;

    int m_nTimeout;

    TimeoutCb m_timeoutcb;
    void* m_pParam;
};



} //Log
} //Kise


#endif //KISE_LOG_LOG_H
