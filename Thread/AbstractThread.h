//***********************************************************
//Description: the abstract base class of linux posix thread 
//Author: jychen
//***********************************************************

#ifndef ABSTRACT_THREAD_H
#define ABSTRACT_THREAD_H

#include <pthread.h>
#include <stdio.h>

class CAbstractThread
{
public:
    void Start();
    virtual void Quit() = 0;
    virtual ~CAbstractThread();
    void Cancel();
    void Join();

protected:
    static void* SwapRun(void* arg);
    virtual void Run() = 0;

private:
    pthread_t m_Th;
};

#endif //ABSTRACT_THREAD_H

