/*
 * jsio.h
 *
 *  Created on: 2009-6-29
 *      Author: root
 */

#ifndef KISE_JSIOMODEL_H_
#define KISE_JSIOMODEL_H_

#include <arpa/inet.h>
#include <sys/time.h>
#include "signal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>

#include <pthread.h>
#include <list>
#include <iostream>
using namespace std;

#define JSMAXPOOLSIZE   1024
#define JSMAXIOEVENTCOUNT 100

//FD EVENT
#define JSREADEVENT			0x00000001
#define JSWRITEEVENT		0x00000002
#define JSEXCEPTIONEVENT	0x00000003
#define JSCONNECTEDEVENT    0x00000004
#define JSUNCONNECTEVENT    0x00000005

//#define JSUNSETCONEVENT     0x00000006

#define  JSRD				0x00000001
#define  JSWR               0x00000002
#define  JSEX               0x00000004
#define  JSRDWREX           0x00000007
#define  JSCONNECT          0x00000008

//#define JSREADMASK			0x00000001
//#define JSWRITEMASK			0x00000002
//#define JSEXCEPTIONMASK		0x00000004
//#define JSCONNECTEVENTMASK  0x00000008

#define JSISSETMASK(mask,event)	(((mask)&(event))==(mask))

typedef int(*EventCallBack)(int fd, int event, void *param);

struct JSFdEvent
{
    int fd;
    int event;
    EventCallBack eventCallBack;
    void* userParam;
};

struct ConnEvent
{
    JSFdEvent fdEvent;
    int curEvent;
    bool caseConnEvent; //连接or断开
};

class CJSIOModel
{
public:
    CJSIOModel();
    ~CJSIOModel();

    void Start();

    int AddNewFd(int fd, int event, EventCallBack eventCallBack, void* userParam = NULL);
    int DelFd(int fd);

    void FdEnable(int fd, int event);
    void FdDisable(int fd, int event);

    static void* Work(void* arg);

    void GetFdSet(fd_set& rdSet, fd_set& wrSet, fd_set& exSet, ConnEvent& connEvent);


    //此函数是不应该出现的，为了不修改原代码才出现的
    void AddUnConnEvent(int fd, EventCallBack eventCallBack, void* userParam = NULL);

private:

    JSFdEvent pool[JSMAXPOOLSIZE];
    int m_maxPoolIndex;

    fd_set m_rdSet;
    fd_set m_wrSet;
    fd_set m_exSet;
    int m_maxFd;

    list<ConnEvent> m_connEventInvokeList;

    pthread_mutex_t m_mutex;
    pthread_t m_workThread;
};

#endif /* JSIOMODEL_H_ */
