#include "Heartbeat.h"

#include <stdlib.h>
#include <unistd.h>
#include "Network/Session.h"
#include "ProxyTaskDispatcher.h"

CHeartbeatThread::CHeartbeatThread(CSessionVector& allSession, CServiceVector& reconnectServices):
m_bQuit(false),
m_allSession(allSession),
m_reconnectServices(reconnectServices)
{

}

CHeartbeatThread::~CHeartbeatThread()
{
	m_bQuit = true;
	Join();
}


void CHeartbeatThread::SendHeartbeat(CSession* pSession)
{

}


void CHeartbeatThread::Run()
{
	int timeout = 15;

	while (!m_bQuit)
	{
		//超时检测及发送心跳检测
		m_allSession.Lock();
		for (int i = 0; i < m_allSession.Size(); i++)
		{
			SessionInfo* pEach = m_allSession.At(i);

			if (pEach != NULL && pEach->pSession != NULL \
					&& pEach->kind == SESSION_KIND_REMOTE_SERVER_SYS)
			{
				if (abs(time(0) - pEach->tmLastRecvTime.tv_sec) > timeout)
				{
					ReconnectInfo info;
					info.ip = pEach->pSession->GetIP();
					info.port = pEach->pSession->GetPort();
					m_lReconInfoList.push_back(info);
                    
                    pEach->pSession->Shutdown();

					m_allSession.Remove(pEach->pSession);
				}
				else
				{
					//发送心跳检测
					if (abs(time(0) - pEach->tmLastSendTime.tv_sec) >= 5)
					{
						gettimeofday(&pEach->tmLastSendTime, NULL);
						SendHeartbeat(pEach->pSession);
					}
				}
			}
		}
		m_allSession.Unlock();

		//超时链接重连
		if (m_lReconInfoList.size() > 0)
		{
			m_reconnectServices.Lock();
			while (m_lReconInfoList.size() > 0)
			{
				ReconnectInfo& info = m_lReconInfoList.front();
				m_reconnectServices.Add(info.ip.c_str(), info.port);
				m_lReconInfoList.pop_front();
			}
			m_reconnectServices.Unlock();
		}

		sleep(5);
	}
}



