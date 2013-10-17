
#ifndef PROXYTASKDISPATCHER_H
#define PROXYTASKDISPATCHER_H

#include "../Network/TaskDispatcher.h"
#include "../config/config.h"

#include <list>

struct SessionGroup {
	int id;
	enum CVT_RULE rule;
	std::list<CSession*> left;
	std::list<CSession*> right;
};

class CProxyTaskDispatcher: public CTaskDispatcher
{
	public:
		CProxyTaskDispatcher();
		virtual ~CProxyTaskDispatcher();
		virtual int Dispatch(CSession* pSession, const char* pData, int dataSize);
		virtual void EventCallBack(CSession* pSession, short event);

		void SetBrokenConns(std::list<struct TCPConn> *pconns,
				pthread_mutex_t *lock) {
			m_pbrokens = pconns;
			m_conns_lock = lock;
		}
	protected:
		virtual void Run(){}
	private:
		void InitSessionGroups(void);
	private:
		std::list<SessionGroup> m_groups;
		pthread_mutex_t m_session_lock;
		void AddSessionIntoGroup(
				const struct TCPConn &con,
				CSession *psession);
		void RemoveSessionInGroup(CSession *psession);
		
		std::list<struct TCPConn> *m_pbrokens;
		pthread_mutex_t *m_conns_lock;
		void AddIntoBrokens(struct TCPConn con);
};


#endif
