
#include "ProxyTaskDispatcher.h"

CProxyTaskDispatcher::CProxyTaskDispatcher()
{
	pthread_mutex_init(&m_session_lock, NULL);
	InitSessionGroups();
}

CProxyTaskDispatcher::~CProxyTaskDispatcher()
{
	pthread_mutex_destroy(&m_session_lock);
}

/* 当有数据到来时，先判断它是属于哪一组哪一边的数据
 * 然后对它进行设置好的转换
 * 如果转换成功，则将它发送到与它对应的另一边
 */
int CProxyTaskDispatcher::Dispatch(
		CSession* pSession, const char* pData, int dataSize)
{
	bool bValid = true;
	switch (pSession->GetRole())
	{
		case NETWORK_ROLE_CLIENT:	
			// 作为客户角色建立的会话
			break ;
		case NETWORK_ROLE_ACCEPT:	
			// 作为服务器角色建立的会话
			break;
		default:
			bValid = false;
			break ;
	}

	return bValid ? 1 : 0;
}

/* EventCallBack, 网络事件回调函数:
 * 如果连接断开，本端是客户端则进行重连
 * 还要找到这个会话属于哪一个组，将这个组的会话数更新
 * 如果是新的连接，同样要找到这个会话属于哪个组
 * 将这个新的会话加入它所属的组
 */
void CProxyTaskDispatcher::EventCallBack(
		CSession* pSession, short event)
{
	TCPConn con;
	con.mod = (pSession->GetRole() == NETWORK_ROLE_CLIENT ? 
			CLIENT : SERVER);
	if(con.mod == CLIENT) {
		con.ip = pSession->GetIP();
		con.port = pSession->GetPort();
	} else {
		con.ip = "0";
		con.port = pSession->GetListenPort();
	}

	switch (event) {
		case NETWORK_STATE_UNCONNECTED:
			printf("Disconnect: mod=%s, ip=%s, port=%u.\n",
					con.mod == CLIENT?"CLIENT":"SERVER", 
					con.ip.c_str(), con.port);
			RemoveSessionInGroup(pSession);
			if(con.mod == CLIENT) AddIntoBrokens(con);
			break;
		case NETWORK_STATE_CONNECTED:
			printf("Connected: mod=%s, ip=%s, port=%u.\n",
					con.mod == CLIENT?"CLIENT":"SERVER", 
					con.ip.c_str(), con.port);
			AddSessionIntoGroup(pSession);
			break;
		default:
			printf("Error: undefined event!! \n");
			break;
	}
}

/**
 * 根据配置中的组的数目来创建本类中的组
 */
void CProxyTaskDispatcher::InitSessionGroups()
{
	PXGROUP::const_iterator it;
	const PXGROUP &gp = ConfigGet()->groups;
	for(it = gp.begin(); it != gp.end(); it++ ) {
		struct SessionGroup gp;
		gp.id = (*it).id;
		gp.rule = (*it).rule;
		m_groups.push_back(gp);
		printf("Init a new session group, id = %d\n", gp.id);
	}
}

void CProxyTaskDispatcher::AddIntoBrokens(struct TCPConn con)
{
	pthread_mutex_lock(m_conns_lock);
	m_pbrokens->push_back(con);
	pthread_mutex_unlock(m_conns_lock);
}

/* AddSessionIntoGroup: 将一个会话添加到某个组
 * 首先根据con查询配置，获取应该将这个会话添加到哪个组
 * 然后将会话加入指定的组（根据ID以及side）
 */
void AddSessionIntoGroup(const struct TCPConn &con,
		CSession *psession)
{
	pthread_mutex_lock(&m_session_lock);

	pthread_mutex_unlock(&m_session_lock);
}

/* RemoveSessionInGroup: 从组里面删除一个会话
 * 依次遍历每一个组的左右两边的会话list
 * 一旦找到符合的session，则将其移除
 */
void RemoveSessionInGroup(CSession *psession)
{
	pthread_mutex_lock(&m_session_lock);

	std::list<SessionGroup>::iterator it;
	for(it = m_groups.begin(); it != m_groups.end(); it++) {
		std::list<CSession*>::iterator temp;
		std::list<CSession*> &ll = (*it).left;
		for(temp = ll.begin(); temp != ll.end(); temp++) {
			if( (*temp) == psession ) {
				ll.erase(temp);
				printf("Erase session in : groupid=%d, side=%s\n",
						(*it).id, "LEFT");
				return;
			}
		}
		std::list<CSession*> &lr = (*it).right;
		for(temp = lr.begin(); temp != lr.end(); temp++) {
			if( (*temp) == psession ) {
				lr.erase(temp);
				printf("Erase session in : groupid=%d, side=%s\n",
						(*it).id, "RIGHT");
				return;
			}
		}
	}

	pthread_mutex_unlock(&m_session_lock);
}

