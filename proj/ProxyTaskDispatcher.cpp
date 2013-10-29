
#include "../include/pdebug.h"
#include "ProxyTaskDispatcher.h"
#include "Convert.h"
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
int CProxyTaskDispatcher::Dispatch( CSession* psession, const char* pData, int dataSize)
{ 
	struct TCPConn con = CreateConnBySession(psession);
	int groupid = -1;
	enum GRP_SIDE side;
	enum CVT_RULE rule;
	if( ConfigLookup(con, &groupid, &side, &rule) ) {
		const char *pret = NULL; int retsize = 0;
		if(Convert(rule, pData, dataSize, 
			pret, retsize, psession, con)) {
			SendResultToOtherSide(groupid, side, pret, retsize);
			if(pret != pData) delete [] pret; 
		}
	} else {
		printf("Error: data from unconfiged session.\n");
	}

	return 1;
}

/* EventCallBack, 网络事件回调函数:
 * 如果连接断开，本端是客户端则进行重连
 * 还要找到这个会话属于哪一个组，将这个组的会话数更新
 * 如果是新的连接，同样要找到这个会话属于哪个组
 * 将这个新的会话加入它所属的组
 */
void CProxyTaskDispatcher::EventCallBack(
		CSession* psession, short event)
{
	struct TCPConn con = CreateConnBySession(psession);
	switch (event) {
		case NETWORK_STATE_UNCONNECTED:
		{
			printf("Disconnect: mod=%s, ip=%s, port=%u.\n",
					con.mod == CLIENT?"CLIENT":"SERVER", 
					con.ip.c_str(), con.port);
			RemoveSessionInGroup(psession);
			if(con.mod == CLIENT) 
				AddIntoBrokens(con);
			break;
		}
		case NETWORK_STATE_CONNECTED:
		{
			printf("Connected: mod=%s, ip=%s, port=%u.\n",
					con.mod == CLIENT?"CLIENT":"SERVER", 
					con.ip.c_str(), con.port);
			if(con.mod == CLIENT)
				RemoveFromBrokens(con);
			AddSessionIntoGroup(con, psession);

			break;
		}
		default:
		{
			printf("Error: undefined event!! \n");
			break;
		}
	}
}

/**
 * 根据配置中的组的数目来创建本类中的组
 * 在配置中主要配置每一组转换两边的端点ip port和模式
 * 在本类中对于每个连接要加入CSession成员
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

/**
 * 将一个连接加入需要重连的列表中
 * 列表是外部设置的，外部维护重连线程来进行相关的重连工作
 *
 */
void CProxyTaskDispatcher::AddIntoBrokens(struct TCPConn con)
{
	pthread_mutex_lock(m_conns_lock);
	m_pbrokens->push_back(con);
	pthread_mutex_unlock(m_conns_lock);
}

void CProxyTaskDispatcher::RemoveFromBrokens(const struct TCPConn &con)
{
	pthread_mutex_lock(m_conns_lock);
	std::list<struct TCPConn>::iterator it;	
	for(it = m_pbrokens->begin(); it != m_pbrokens->end(); it++) {
		if( TCPConnEq(con, (*it) ) ) {
			m_pbrokens->erase(it);
		}
	}
	pthread_mutex_unlock(m_conns_lock);
}

/* AddSessionIntoGroup: 将一个会话添加到某个组
 * 首先根据con查询配置，获取应该将这个会话添加到哪个组
 * 然后将会话加入指定的组（根据ID以及side）
 */
void CProxyTaskDispatcher::AddSessionIntoGroup(
		const struct TCPConn &con, CSession *psession)
{
	pthread_mutex_lock(&m_session_lock);
	int groupid = -1;
	enum GRP_SIDE side;
	if( ConfigLookup(con, &groupid, &side, NULL) ) {
		std::list<SessionGroup>::iterator it;
		for(it = m_groups.begin(); it != m_groups.end(); it++) {
			if((*it).id == groupid) {
				if(side == LEFTSIDE) {
					(*it).left.push_back(psession);
					break;
				} else if(side == RIGHTSIDE) {
					(*it).right.push_back(psession);
					break;
				} else {

				}
			}
		}
		printf("New session, add in group %d, %s.\n",
				groupid, side == LEFTSIDE?"LEFT":"RIGHT");
	} else {
		printf("Session not configed, ip = %s, port=%u, mode=%s\n",
				con.ip.c_str(), con.port, con.mod == CLIENT?"CLIENT":"SERVER");
	}
	pthread_mutex_unlock(&m_session_lock);
}

/* RemoveSessionInGroup: 从组里面删除一个会话
 * 依次遍历每一个组的左右两边的会话list
 * 一旦找到符合的session，则将其移除
 */
void CProxyTaskDispatcher::RemoveSessionInGroup(CSession *psession)
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
				pthread_mutex_unlock(&m_session_lock);
				return;
			}
		}
		std::list<CSession*> &lr = (*it).right;
		for(temp = lr.begin(); temp != lr.end(); temp++) {
			if( (*temp) == psession ) {
				lr.erase(temp);
				printf("Erase session in : groupid=%d, side=%s\n",
						(*it).id, "RIGHT");
				pthread_mutex_unlock(&m_session_lock);
				return;
			}
		}
	}
	printf("RemoveSessionInGroup, session havn't store yet.\n");
	pthread_mutex_unlock(&m_session_lock);
}

/** 
 * 根据一个CSession类的对象，构建一个TCPConn的对象
 */
struct TCPConn CProxyTaskDispatcher::CreateConnBySession(CSession *psession) {
	TCPConn con;
	con.mod = (psession->GetRole() == NETWORK_ROLE_CLIENT ? CLIENT : SERVER);
	if(con.mod == CLIENT) {
		con.ip = psession->GetIP();
		con.port = psession->GetPort();
	} else {
		con.ip = "0";
		con.port = psession->GetListenPort();
	}

	return con;
}

/**
 * SendResultToOtherSide: 将数据发送给另一边
 * gpid ：发送者所属的会话组
 * side : 发送者在会话组的边（LEFTSIDE/RIGHTSIDE）
 * data, size: 要发送的数据及长度
 */
void CProxyTaskDispatcher::SendResultToOtherSide(
		int gpid, enum GRP_SIDE side, 
		const char *data, int size)
{
	pthread_mutex_lock(&m_session_lock);

	std::list<SessionGroup>::iterator it;
	for(it = m_groups.begin(); it != m_groups.end(); it++) {
		if((*it).id == gpid) {
			std::list<CSession*>::iterator temp;
			std::list<CSession*> &ll =
				( side == LEFTSIDE ? (*it).right : (*it).left );
			for(temp = ll.begin(); temp != ll.end(); temp++) {
				if( (*temp) )
					(*temp)->Send(data, size);
			}
		}
	}

	pthread_mutex_unlock(&m_session_lock);
}

void CProxyTaskDispatcher::SendResultToOtherSide(
		const struct TCPConn &con, const char *data, int size)
{
	int groupid; enum GRP_SIDE side;
	if( ConfigLookup(con, &groupid, &side, NULL) )
		SendResultToOtherSide(groupid, side, data, size);
	else 
		PDEBUG("SendResultToOtherSide, lookup failed.\n");
}

