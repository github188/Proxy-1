
#ifndef PROXYTASKDISPATCHER_H
#define PROXYTASKDISPATCHER_H

#include "../Network/TaskDispatcher.h"
#include "../config/config.h"
#include "Convert.h"
#include <list>

/* struct SessionGroup: 会话组
 * 每一个会话组有左右两个会话连表
 * 两个连表通过规则rule交换数据
 */
struct SessionGroup {
	int id;
	enum CVT_RULE rule;
	std::list<CSession*> left;
	std::list<CSession*> right;
};

/* CProxyTaskDispatcher: 代理的网络分发类
 * 封装的是网络模块的回调函数,处理网络事件和数据
 */
class CProxyTaskDispatcher: public CTaskDispatcher
{
	public:
		CProxyTaskDispatcher();
		virtual ~CProxyTaskDispatcher();
		virtual int Dispatch(CSession* pSession, const char* pData, int dataSize);
		virtual void EventCallBack(CSession* pSession, short event);

		void SendResultToOtherSide(const struct TCPConn &con, const char *data, int size);

		void SetBrokenConns(std::list<struct TCPConn> *pconns,
				pthread_mutex_t *lock) {
			m_pbrokens = pconns;
			m_conns_lock = lock;
		}
	protected:
		virtual void Run(){}
	private:
		void InitSessionGroups(void);

		struct TCPConn CreateConnBySession(CSession *psession);
		void SendResultToOtherSide(int gpid, enum GRP_SIDE side,
				const char *data, int size);
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
		void RemoveFromBrokens(const struct TCPConn &con);
};


#endif
