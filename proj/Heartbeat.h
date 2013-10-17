#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <list>
#include <string>
using namespace std;

#include "Thread/AbstractThread.h"
#include "Thread/Mutex.h"

class CSessionVector;
class CServiceVector;
class CSession;

struct ReconnectInfo
{
	string ip;
	unsigned short port;
};

class CHeartbeatThread : public CAbstractThread
{
public:
	CHeartbeatThread(CSessionVector& allSession, CServiceVector& reconnectServices);
	~CHeartbeatThread();

protected:
	void Quit() { m_bQuit = true; }
	void Run();

private:
	void SendHeartbeat(CSession* pSession);

private:
	bool m_bQuit;
	CSessionVector& m_allSession;	// 所有会话
	CServiceVector& m_reconnectServices;	// 需要重新连接的会话
	list<ReconnectInfo> m_lReconInfoList;
};

#endif //HEARTBEAT_H


