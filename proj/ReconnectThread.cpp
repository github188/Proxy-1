
#include "ReconnectThread.h"
#include <unistd.h>

CReconnectThread::CReconnectThread(CNetwork& network)
	: m_bStop(false)
    , m_network(network)
{
}

void CReconnectThread::Run()
{
    m_bStop = false;
	while (!m_bStop) {
		printf("Start reconnect ... \n");
		pthread_mutex_lock(m_conns_lock);
		std::list<struct TCPConn>::iterator it = m_pConns->begin();
		while( it != m_pConns->end() ) {
			if( (*it).mod == CLIENT ) {
				m_network.Client((*it).ip.c_str(), (*it).port);
				it = m_pConns->erase(it);
			} else {
				it++;
			}
		}
		pthread_mutex_unlock(m_conns_lock);
		sleep(10);
	}
}
