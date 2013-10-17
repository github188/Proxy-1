
#ifndef RECONNECT_THREAD_H
#define RECONNECT_THREAD_H

#include "../config/config.h"
#include "../Network/Network.h"
#include "../Thread/AbstractThread.h"

#include <pthread.h>
#include <list>

class CReconnectThread: public CAbstractThread
{
	public:
		CReconnectThread(CNetwork& network);
		~CReconnectThread() {}

	public:
		void SetBrokenConns(std::list<struct TCPConn> *pConns,
				pthread_mutex_t *lock) {
			m_pConns = pConns;
			m_conns_lock = lock;
		}

	protected:
		virtual void Run();
		virtual void Quit() { m_bStop = true; }

	private:
		bool m_bStop;
		CNetwork& m_network;

		std::list<struct TCPConn> *m_pConns;
		pthread_mutex_t *m_conns_lock;
};

#endif
