
#ifndef PROXYTASKDISPATCHER_H
#define PROXYTASKDISPATCHER_H

#include <Network/TaskDispatcher.h>
#include <config/config.h>

#include <list>

class CProxyTaskDispatcher: public CTaskDispatcher
{
	public:
		CProxyTaskDispatcher();
		virtual ~CProxyTaskDispatcher();
		virtual int Dispatch(CSession* pSession, const char* pData, int dataSize);
		virtual void EventCallBack(CSession* pSession, short event);

		void SetBrokenConns(std::list<struct tcp_conn> *pConns){}
	protected:
		virtual void Run(){}
	private:
};


#endif
