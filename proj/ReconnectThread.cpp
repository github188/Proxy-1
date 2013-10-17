
#include "ReconnectThread.h"

CReconnectThread::CReconnectThread(CNetwork& network)
	: m_bStop(false)
    , m_network(network)
{
}

void CReconnectThread::Run()
{
    m_bStop = false;
    
	while (!m_bStop) {
		printf("Reconnect here....\n");
		sleep(2);
	}
}
