/*
 * jsiomodel.cpp
 *
 *  Created on: 2009-6-29
 *      Author: root
 */

#include"jsiomodel.h"

CJSIOModel::CJSIOModel()
{
    int i;
    for (i = 0; i < JSMAXPOOLSIZE; i++)
    {
        pool[i].fd = -1;
        pool[i].event = -1;
        pool[i].eventCallBack = NULL;
    }
    m_maxPoolIndex = -1;
    m_maxFd = -1;

    FD_ZERO(&m_rdSet);
    FD_ZERO(&m_wrSet);
    FD_ZERO(&m_exSet);

    pthread_mutex_init(&m_mutex, NULL);
}

CJSIOModel::~CJSIOModel()
{
    pthread_mutex_destroy(&m_mutex);
}

void CJSIOModel::Start()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&m_workThread, NULL, Work, this);
}

int CJSIOModel::AddNewFd(int fd, int event, EventCallBack eventCallBack, void* userParam/*=NULL*/)
{
    int i;
    int rt = -1;

    pthread_mutex_lock(&m_mutex);

    for (i = 0; i < JSMAXPOOLSIZE; i++)
    {
        if (pool[i].fd == -1)
        {
            pool[i].fd = fd;
            pool[i].event = event;
            pool[i].eventCallBack = eventCallBack;
            pool[i].userParam = userParam;

            rt = i;
            if (m_maxPoolIndex < i)
            {
                m_maxPoolIndex = i;
            }
            if (JSISSETMASK(JSRD,event))
            {
                FD_SET(fd, &m_rdSet);
            }
            if (JSISSETMASK(JSWR,event))
            {
                FD_SET(fd, &m_wrSet);
            }
            if (JSISSETMASK(JSEX,event))
            {
                FD_SET(fd, &m_exSet);
            }
            if (JSISSETMASK(JSCONNECT,event))
            {
                ConnEvent connEvent;
                connEvent.fdEvent = pool[i];
                connEvent.curEvent = JSCONNECTEDEVENT;
                connEvent.caseConnEvent = true;
                m_connEventInvokeList.push_back(connEvent);
            }
            if (m_maxFd < fd)
            {
                m_maxFd = fd;
            }
            break;
        }
    }

    pthread_mutex_unlock(&m_mutex);

    if (rt == -1)
    {
        perror("add Fd");
    }

    return rt;
}

int CJSIOModel::DelFd(int fd)
{
    int i;
    int rt = -1;

    pthread_mutex_lock(&m_mutex);

    for (i = 0; i < m_maxPoolIndex + 1; i++)
    {
        if (fd == pool[i].fd)
        {
            if (JSISSETMASK(JSCONNECT,pool[i].event))
            {
                ConnEvent connEvent;
                connEvent.fdEvent = pool[i];
                connEvent.curEvent = JSUNCONNECTEVENT;
                connEvent.caseConnEvent = true;
                m_connEventInvokeList.push_back(connEvent);
            }

            pool[i].fd = -1;
            pool[i].event = -1;
            pool[i].eventCallBack = NULL;
            pool[i].userParam = NULL;
            rt = i;
            FD_CLR(fd, &m_rdSet);
            FD_CLR(fd, &m_wrSet);
            FD_CLR(fd, &m_exSet);

            if (i == m_maxPoolIndex)
            {
                for (--i; i >= 0 && pool[i].fd < 0; i--)
                {
                    ;
                }
                m_maxPoolIndex = i;
            }
            if (m_maxFd == fd)
            {
                m_maxFd = 0;
                for (int j = 0; j < m_maxPoolIndex + 1; j++)
                {
                    if (pool[j].fd > m_maxFd)
                        m_maxFd = pool[j].fd;
                }
            }
        }
    }

    pthread_mutex_unlock(&m_mutex);

    return rt;
}

void CJSIOModel::FdEnable(int fd, int event)
{
    pthread_mutex_lock(&m_mutex);

    for (int i = 0; i < m_maxPoolIndex + 1; i++)
    {
        if (fd == pool[i].fd)
        {
            //to do 置位相应pool[i].event


            if (JSISSETMASK(JSRD,event))
            {
                FD_SET(fd, &m_rdSet);
            }
            if (JSISSETMASK(JSWR,event))
            {
                FD_SET(fd, &m_wrSet);
            }
            if (JSISSETMASK(JSEX,event))
            {
                FD_SET(fd, &m_exSet);
            }
        }
    }

    pthread_mutex_unlock(&m_mutex);
}

void CJSIOModel::FdDisable(int fd, int event)
{
    pthread_mutex_lock(&m_mutex);

    for (int i = 0; i < m_maxPoolIndex + 1; i++)
    {
        if (fd == pool[i].fd)
        {
            //to do 置位相应pool[i].event


            if (JSISSETMASK(JSRD,event))
            {
                FD_CLR(fd, &m_rdSet);
            }
            if (JSISSETMASK(JSWR,event))
            {
                FD_CLR(fd, &m_wrSet);
            }
            if (JSISSETMASK(JSEX,event))
            {
                FD_CLR(fd, &m_exSet);
            }
        }
    }

    pthread_mutex_unlock(&m_mutex);
}

void CJSIOModel::GetFdSet(fd_set& rdSet, fd_set& wrSet, fd_set& exSet, ConnEvent& connEvent)
{
    pthread_mutex_lock(&m_mutex);
    rdSet = m_rdSet;
    wrSet = m_wrSet;
    exSet = m_exSet;

    connEvent.caseConnEvent = false;

    if (!m_connEventInvokeList.empty())
    {
        connEvent = m_connEventInvokeList.front();
        connEvent.caseConnEvent = true;
        m_connEventInvokeList.pop_front();
    }

    pthread_mutex_unlock(&m_mutex);
}


void CJSIOModel::AddUnConnEvent(int fd, EventCallBack eventCallBack, void* userParam/* = NULL*/)
{
    pthread_mutex_lock(&m_mutex);

    ConnEvent connEvent;
    connEvent.fdEvent.fd = fd;
    connEvent.fdEvent.event = JSCONNECT;
    connEvent.fdEvent.eventCallBack = eventCallBack;
    connEvent.fdEvent.userParam = userParam;

    connEvent.curEvent = JSUNCONNECTEVENT;
    connEvent.caseConnEvent = true;
    m_connEventInvokeList.push_back(connEvent);

    pthread_mutex_unlock(&m_mutex);
}


void* CJSIOModel::Work(void*arg)
{
    CJSIOModel *pThis = (CJSIOModel*) arg;
    fd_set wrSet;
    fd_set rdSet;
    fd_set exSet;

    ConnEvent connEvent;

    timeval tv;

    int n = 0;

    bool bShouldSleep = true;

    while (1)
    {

        tv.tv_sec = 0;
        tv.tv_usec = 100;

        pThis->GetFdSet(rdSet, wrSet, exSet, connEvent);

        if (connEvent.caseConnEvent)
        {
            int fd = connEvent.fdEvent.fd;
            EventCallBack handler = connEvent.fdEvent.eventCallBack;
            void* userParam = connEvent.fdEvent.userParam;

            (*handler)(fd, connEvent.curEvent, userParam);
            //printf("iomode fd = %d, curevent = %d\n", fd, connEvent.curEvent);
            continue;
        }

        bShouldSleep = true;

        n = select(pThis->m_maxFd + 1, &rdSet, &wrSet, NULL, &tv);
        if (n <= 0)
        {
            continue;
        }

        for (int i = 0; i < pThis->m_maxPoolIndex + 1; i++)
        {
            if ((pThis->pool[i]).fd == -1)
            {
                continue;
            }

            int fd = pThis->pool[i].fd;
            EventCallBack handler = pThis->pool[i].eventCallBack;
            void* userParam = pThis->pool[i].userParam;

            if (FD_ISSET(fd, &rdSet))
            {
                bShouldSleep = false;
                (*handler)(fd, JSREADEVENT, userParam);
            }

            if (FD_ISSET(fd, &wrSet))
            {
                bShouldSleep = false;
                (*handler)(fd, JSWRITEEVENT, userParam);
            }
        }

        if (bShouldSleep)
        {
            timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 1000000;
            nanosleep(&ts, 0);
        }
    }

    return 0;
}

