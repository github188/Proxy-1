#include "Network.h"

#include <arpa/inet.h>
#include <sys/time.h>
#include "signal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include <fcntl.h>

#include "jsiomodel.h"
#include "Session.h"
#include "TaskDispatcher.h"

static void SIGPipeHandler_Sock(int/* sig*/)
{}


CNetwork::CNetwork(CTaskDispatcher* pDispatcher):
m_bQuit(false),
m_pIOmodel(NULL),
m_pTaskDispatcher(pDispatcher)
{
    static bool inited = false;

    if (!inited)
    {
        signal(SIGPIPE, SIGPipeHandler_Sock);
        inited = true;
    }

    m_pIOmodel = new CJSIOModel;
    m_pIOmodel->Start();
}

CNetwork::~CNetwork()
{
    delete m_pIOmodel;
}


int
make_socket_nonblocking(int fd)
{
    return 1;

#ifdef WIN32
    {
        u_long nonblocking = 1;
        if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            event_sock_warn(fd, "fcntl(%d, F_GETFL)", (int)fd);
            return -1;
        }
    }
#else
    {
        int flags;
        if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
            //event_warn("fcntl(%d, F_GETFL)", fd);
            return -1;
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            //event_warn("fcntl(%d, F_SETFL)", fd);
            return -1;
        }
    }
#endif
    return 0;
}

int CNetwork::Server(unsigned short nPort, int nMaxConnectNum/*=32*/)
{
    assert(nMaxConnectNum > 0);

    int servFd = TcpServer(nPort, nMaxConnectNum);
    if (servFd > 0)
    {
        Listener listener;
        listener.listenFd = servFd;
        listener.listenPort = nPort;
        AddListener(listener);

        make_socket_nonblocking(servFd);

        m_pIOmodel->AddNewFd(servFd, JSRD, ListenCallback, this);
    }
    else
    {
        printf("can't create tcp server, listen port = %d\n", nPort);
    }

    return servFd;
}


CSession* CNetwork::Client(const char* ip, unsigned short nPort)
{
    if (ip == NULL)
    {
        return NULL;
    }

    CSession* pSession = NULL;

    int clientFd = TcpClient(string(ip), nPort);
    if (clientFd > 0)
    {
        pSession = new CSession(this);
        pSession->SetFd(clientFd);
        pSession->SetIP(ip);
        pSession->SetPort(nPort);
        pSession->SetListenPort(0);
        pSession->SetState(NETWORK_STATE_CONNECTED);
        pSession->SetRole(NETWORK_ROLE_CLIENT);
        pSession->SetType(NETWORK_TYPE_UNKNOWN);
        AddSession(pSession);

        //make_socket_nonblocking(clientFd);

        m_pIOmodel->AddNewFd(clientFd, JSRDWREX|JSCONNECT, SessionCallback, this);
    }
    else
    {
        printf("tcp client, can't connect to ip = %s, port = %d\n", ip, nPort);
        pSession = new CSession(this);
        pSession->SetFd(-1);
        pSession->SetIP(ip);
        pSession->SetPort(nPort);
        pSession->SetListenPort(0);
        pSession->SetState(NETWORK_STATE_UNCONNECTED);
        pSession->SetRole(NETWORK_ROLE_CLIENT);
        pSession->SetType(NETWORK_TYPE_UNKNOWN);
        AddSession(pSession);

        m_pIOmodel->AddUnConnEvent(-1, SessionCallback, this);
    }

    return pSession;
}



int CNetwork::TcpServer(unsigned short nPort, int nMaxConnectNum/*=32*/)
{
    int servfd = -1;

    if (nPort <= 0 || nMaxConnectNum <= 0)
    {
        return -1;
    }

    servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd == -1)
    {
        perror("socket:");
        return -1;
    }
    int on = 1;
    setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in servAddr;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(nPort);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(servAddr.sin_zero), 8);

    if (bind(servfd, (struct sockaddr *) &servAddr, sizeof(struct sockaddr)) == -1)
    {
        perror("Bind:");
        close(servfd);
        return -1;
    }

    if (listen(servfd, nMaxConnectNum) == -1)
    {
        perror("Listen:");
        close(servfd);
        return -1;
    }

    return servfd;
}


int CNetwork::TcpClient(const string& strIP, unsigned short nPort)
{
    int clientfd = -1;

    if (strIP.empty() || nPort <= 0)
    {
        return -1;
    }

    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        return -1;
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(nPort);
    servAddr.sin_addr.s_addr = inet_addr(strIP.c_str());
    bzero(&(servAddr.sin_zero), 8);

    unsigned long ul = 1;
    ioctl(clientfd, FIONBIO, &ul); //设置为非阻塞模式
    bool ret = false;
    if (connect(clientfd, (struct sockaddr *) &servAddr, sizeof(struct sockaddr)) == -1)
    {
        timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        fd_set set;
        FD_ZERO(&set);
        FD_SET(clientfd, &set);
        if (select(clientfd + 1, NULL, &set, NULL, &tv) > 0)
        {
            int error = -1;
            int len = sizeof(int);
            getsockopt(clientfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
            if (error == 0)
            {
                ret = true;
                //cout<<"connect ok!!! error == 0"<<endl;
                //printf("error no = %d\n", errno);
            }
            else
            {
                ret = false;
                //cout<<"getsockopt error"<<endl;
                //printf("error no = %d\n", errno);
            }
        }
        else
        {
            ret = false;
            //cout<<"select<=0"<<endl;
            //printf("error no = %d\n", errno);
        }
    }
    else
    {
        ret = true;
        //cout<<"connect ok!!!"<<endl;
        //printf("error no = %d\n", errno);
    }

    ul = 0;
    ioctl(clientfd, FIONBIO, &ul); //设置为阻塞模式

    //printf("error no = %d\n", errno);

    if (!ret)
    {
        close(clientfd);
        //printf("BlockSocket connect fail\n");
        return -1;
    }
    //printf("BlockSocket connect ok\n");

    return clientfd;
}

int CNetwork::ListenCallback(int fd, int what, void* param)
{
    int ret = -1;
    CNetwork* pThis = (CNetwork*)param;
    if (what == JSREADEVENT)
    {
         ret = pThis->ListenCallback(fd);
    }

    return ret;
}


int CNetwork::ListenCallback(int servFd)
{
    if (servFd < 0)
    {
        return -1;
    }

    struct sockaddr_in ra;
    size_t len = sizeof(ra);
    int clientFd = accept(servFd, (struct sockaddr*) &ra, &len);

    if (clientFd > 0)
    {
        CSession* pSession = new CSession(this);
        pSession->SetFd(clientFd);
        pSession->SetIP(inet_ntoa(ra.sin_addr));
        pSession->SetPort(ntohs(ra.sin_port));
        pSession->SetListenPort(GetListenPort(servFd));
        pSession->SetState(NETWORK_STATE_CONNECTED);
        pSession->SetRole(NETWORK_ROLE_ACCEPT);
        pSession->SetType(NETWORK_TYPE_UNKNOWN);
        AddSession(pSession);
        printf("accept ok, listen port=%d, peer host=%s,port = %d. \n", 
				pSession->GetListenPort(), pSession->GetIP(), pSession->GetPort() );

        make_socket_nonblocking(clientFd);

        m_pIOmodel->AddNewFd(clientFd, JSRDWREX | JSCONNECT, SessionCallback, this);
    }
    return clientFd;
}

int CNetwork::SessionCallback(int fd, int what, void* param)
{
    CNetwork* pThis = (CNetwork*)param;
    switch (what)
    {
    case JSREADEVENT:
        pThis->ReadCallback(fd);
        break;
    case JSWRITEEVENT:
        pThis->WriteCallback(fd);
        break;
    case JSEXCEPTIONEVENT:
        pThis->Exception(fd);
        break;
    case JSCONNECTEDEVENT:
        pThis->ConnectCallback(fd, NETWORK_STATE_CONNECTED);
        break;
    case JSUNCONNECTEVENT:
        pThis->ConnectCallback(fd, NETWORK_STATE_UNCONNECTED);
        break;
    default:
        break;
    }

    return 0;
}


int CNetwork::ReadCallback(int fd)
{
    if (fd < 0)
    {
        return -1;
    }

    CSession* pSession = GetSession(fd);
    if (pSession == NULL)
    {
        printf("can't get session from fd = %d\n", fd);
        return -1;
    }

    int rlen = pSession->Recv(fd);
    if (rlen <= 0)
    {
        printf("CNetwork::ReadCallback rlen = %d, fd = %d\n", rlen, fd);
        //通知io线程，关闭检测
        pSession->Shutdown();
    }

	return 0;
}


int CNetwork::WriteCallback(int fd)
{
    if (fd < 0)
    {
        return -1;
    }

    CSession* pSession = GetSession(fd);
    if (pSession == NULL)
    {
        printf("can't get session from fd = %d\n", fd);
        return -1;
    }

    int wlen = pSession->Send(fd);
    if (wlen <= 0)
    {
        printf("CNetwork::WriteCallback wlen = %d, fd = %d\n", wlen, fd);
        pSession->Shutdown();
    }

	return 0;
}


int CNetwork::Exception(int fd)
{
    if (fd < 0)
    {
        return -1;
    }

    CSession* pSession = GetSession(fd);
    if (pSession == NULL)
    {
        printf("can't get session from fd = %d\n", fd);
        return -1;
    }

    printf("CNetwork::Exception fd = %d\n", fd);

    pSession->Shutdown();

	return 0;
}

int CNetwork::ConnectCallback(int fd, int what)
{
    if (fd < 0 && fd != -1)
    {
        return -1;
    }

    CSession* pSession = GetSession(fd);
    if (pSession == NULL)
    {
        printf("can't get session from fd = %d\n", fd);
        return -1;
    }

    //invoke
    if (m_pTaskDispatcher != NULL)
    {
        m_pTaskDispatcher->EventCallBack(pSession, what);
    }


    if (what == NETWORK_STATE_UNCONNECTED)
    {
        DelSession(pSession);
        delete pSession;
    }

    if (what == NETWORK_STATE_CONNECTED)
    {
        //do nothing
    }
	return 0;
}

void CNetwork::AddListener(const Listener& listener)
{
    m_lListenerList.push_back(listener);
}


unsigned short CNetwork::GetListenPort(int listenFd)
{
    unsigned short listenPort = 0;
    list<Listener>::iterator itbegin = m_lListenerList.begin();
    list<Listener>::iterator iteend = m_lListenerList.end();
    for (; itbegin != iteend; itbegin++)
    {
        if (itbegin->listenFd == listenFd)
        {
            listenPort = itbegin->listenPort;
            break;
        }
    }

    return listenPort;
}


void CNetwork::AddSession(CSession* pSession)
{
    CMutexLock lock(m_mutex);
    m_lSessionList.push_back(pSession);
}


void CNetwork::DelSession(CSession* pSession)
{
    CMutexLock lock(m_mutex);
    list<CSession*>::iterator itbegin = m_lSessionList.begin();
    list<CSession*>::iterator iteend = m_lSessionList.end();
    for (; itbegin != iteend; itbegin++)
    {
        if (*itbegin == pSession)
        {
            itbegin = m_lSessionList.erase(itbegin);
            break;
        }
    }
}


CSession* CNetwork::GetSession(int fd)
{
    CSession* pSession = NULL;

    CMutexLock lock(m_mutex);
    list<CSession*>::iterator itbegin = m_lSessionList.begin();
    list<CSession*>::iterator iteend = m_lSessionList.end();
    for (; itbegin != iteend; itbegin++)
    {
        if (*itbegin!= NULL && (*itbegin)->GetFd() == fd)
        {
            pSession = *itbegin;
            break;
        }
    }

    return pSession;
}


