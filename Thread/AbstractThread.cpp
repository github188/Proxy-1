#include "AbstractThread.h"

void CAbstractThread::Start()
{
    pthread_create(&m_Th, NULL, SwapRun, this);
}

void* CAbstractThread::SwapRun(void* arg)
{
    CAbstractThread* pThis = (CAbstractThread*) arg;

    pThis->Run();
    return NULL;
}
void CAbstractThread::Cancel()
{
	pthread_cancel(m_Th);
}

void CAbstractThread::Join()
{
	pthread_join(m_Th, NULL);
}

CAbstractThread::~CAbstractThread()
{
}
