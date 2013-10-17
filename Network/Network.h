#ifndef NETWORK_H
#define NETWORK_H

#include <list>
#include <string>
using namespace std;

#include "../Thread/AbstractThread.h"
#include "../Thread/Mutex.h"

struct Listener
{
    int listenFd;
    unsigned short listenPort;
};

class CJSIOModel;
class CSession;
class CTaskDispatcher;

class CNetwork
{
public:
    CNetwork(CTaskDispatcher* pDispatcher);
    ~CNetwork();

    int Server(unsigned short nPort, int nMaxConnectNum=32);
    CSession* Client(const char* ip, unsigned short nPort);

    int TcpServer(unsigned short nPort, int nMaxConnectNum=32);
    int TcpClient(const string& strIP, unsigned short nPort);

    CTaskDispatcher* GetTaskDispatcher() { return m_pTaskDispatcher; }
    CJSIOModel* GetIOMode() { return m_pIOmodel; }

    //io线程已经启动，此线程无用，只是为了保持与原接口一致
    void Start() {}

private:
    static int ListenCallback(int fd, int what, void* param);
    int ListenCallback(int fd);

    static int SessionCallback(int fd, int what, void* param);
    int SessionCallback(int fd, int what);

    int ReadCallback(int fd);
    int WriteCallback(int fd);
    int Exception(int fd);
    int ConnectCallback(int fd, int what);


    void AddListener(const Listener& listener);
    unsigned short GetListenPort(int listenFd);

    void AddSession(CSession* pSession);
    void DelSession(CSession* pSession);
    CSession* GetSession(int fd);


private:
    bool m_bQuit;
    CJSIOModel* m_pIOmodel;
    CTaskDispatcher* m_pTaskDispatcher;

    CMutex m_mutex;
    list<CSession*> m_lSessionList;
    list<Listener> m_lListenerList;
};

#endif //NETWORK_H

