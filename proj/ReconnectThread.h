
#ifndef RECONNECT_THREAD_H
#define RECONNECT_THREAD_H

class CReconnectThread: public CAbstractThread
{
	public:
		CReconnectThread(CNetwork& network);
		SetBrokenConns(std::list<struct tcp_conn> *pConns)
		{ m_pConns = pConns; }
	protected:
		virtual void Run();
		virtual void Quit() { m_bStop = true; }
	private:
		bool m_bStop;
		CNetwork& m_network;

		std::list<struct tcp_conn> *m_pConns;
};

#endif
