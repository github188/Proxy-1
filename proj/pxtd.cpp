/*************************************************************************
 * FILE            :  ProxyTaskDispatcher.cpp
 * TASKS           :  代理处理所有内部网络事务，提供外部与内部之间的网络数据
 * 					     交换，分发各种网络数据
 *
 * PROJECT         :
 * MODULE NAME     :  网络请求任务分发器
 *
 * DATE            :  24,01,2011
 * COPYRIGHT (C)   :  Kise
 * AUTHOR          :  cchao
 *
 * ENVIRONMENT     :
 * MODULES CALLED  :
 * NOTES           :
 * CURRENT VERSION :  1.0
 * HISTORY :
 *   <author>        <time>       <version>        <desc>
 *  Chao Chen       24,01,2011      1.0
 **************************************************************************/
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "MessageDef.h"
#include "ProxyTaskDispatcher.h"

/*******************************************************************************
 * @Name: CSessionVector(class CSessionVector)
 *
 * @Purpose: 创建会话管理容器
 * @Returns: 无
 * @Parameters: int | size[IN] 容器大小
 * @Remarks: 无
 ********************************************************************************/
CSessionVector::CSessionVector(int size)
	: m_sessions(size)
	, m_count(0)
{
	for (int i = 0; i < m_sessions.size(); i++)
	{
		m_sessions[i] = NULL;
	}
}

CSessionVector::~CSessionVector()
{
}

/*******************************************************************************
 * @Name: Size(class CSessionVector)
 *
 * @Purpose: 获得能够管理的最大会话数目
 * @Returns: int | 最大会话数目
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
int CSessionVector::Size()
{
	int size;

	size = m_sessions.size();

	return size;
}

/*******************************************************************************
 * @Name: Count(class CSessionVector)
 *
 * @Purpose: 获得当前有效的会话数目
 * @Returns: 当前有效的会话数目
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
int CSessionVector::Count()
{
	int count;

	count = m_count;

	return count;
}

/*******************************************************************************
 * @Name: At(class CSessionVector)
 *
 * @Purpose: 获得某一索引位置的会话
 * @Returns: 对应索引位置的会话
 * @Parameters: int | index[IN] 索引
 * @Remarks: 无
 ********************************************************************************/
SessionInfo* CSessionVector::At(int index)
{
	SessionInfo* pSessionInfo;

	pSessionInfo = m_sessions[index];

	return pSessionInfo;
}

/*******************************************************************************
 * @Name: Add(class CSessionVector)
 *
 * @Purpose: 在新的会话建立时，将一个会话加入信息加入管理容器
 * @Returns: void
 * @Parameters: SessionInfo* | pSessionInfo[IN] 待加入的会话信息
 * @Remarks: 无
 ********************************************************************************/
void CSessionVector::Add(SessionInfo* pSessionInfo)
{
	bool bExisted = false;

//	for (int i = 0; i < m_sessions.size(); i++)
//	{
//		if (m_sessions[i] != NULL)
//		{
//			printf("port1: %d, port2: %d\n", m_sessions[i]->pSession->GetPort(), pSessionInfo->pSession->GetPort());
//			if (m_sessions[i]->pSession->GetPort() == pSessionInfo->pSession->GetPort() &&
//						!strcmp(m_sessions[i]->pSession->GetIP(), pSessionInfo->pSession->GetIP()))
//			{
//				bExisted = true;
//				break ;
//			}
//		}
//	}

	if (!bExisted)
	{
		for (int i = 0; i < m_sessions.size(); i++)
		{
			if (m_sessions[i] == NULL)
			{
				m_count++;
				m_sessions[i] = pSessionInfo;

				//printf("Add1 %s: %d in index=%d\n", pSessionInfo->pSession->GetIP(), pSessionInfo->pSession->GetPort(), i);
				break ;
			}
		}
	}
}

/*******************************************************************************
 * @Name: Remove(class CSessionVector)
 *
 * @Purpose: 在撤销会话时，将一个会话从容器移除
 * @Returns: void
 * @Parameters: CSession* | pSession[IN] 需要移除的会话
 * @Remarks:
 ********************************************************************************/
void CSessionVector::Remove(CSession* pSession)
{
	int len = m_sessions.size();
	for (int i = 0; i < m_sessions.size(); i++)
	{
		if (m_sessions[i] != NULL && m_sessions[i]->pSession == pSession)
		{
			m_count--;
			m_sessions[i] = NULL;

			break ;
		}
	}
}

void CSessionVector::Lock()
{
	m_lock.Lock();
}

void CSessionVector::Unlock()
{
	m_lock.Unlock();
}

CServiceVector::CServiceVector(int size)
	:  m_services(size)
{
	for (int i = 0; i < size; i++)
	{
		m_services[i] = NULL;
	}
}

int CServiceVector::Size()
{
	int size;

	size = m_services.size();

	return size;
}

CService* CServiceVector::At(int index)
{
	CService* pService = NULL;

	if (index >= 0 || index < m_services.size())
	{
		pService = m_services[index];
	}

	return pService;
}

void CServiceVector::Add(string ip, unsigned short port)
{
	bool bExisted = false;

	for (int i = 0; i < m_services.size(); i++)
	{
		if (m_services[i] != NULL)
		{
			if (m_services[i]->Port() == port && m_services[i]->IP() == ip)
			{
				bExisted = true;
			}
		}
	}

	if (!bExisted)
	{
		for (int i = 0; i < m_services.size(); i++)
		{
			if (m_services[i] == NULL)
			{
				m_services[i] = new CService(ip, port);
				break ;
			}
		}
	}
}

void CServiceVector::Remove(string ip, unsigned short port)
{
	int len = m_services.size();
	for (int i = 0; i < m_services.size(); i++)
	{
		if (m_services[i] != NULL && m_services[i]->IP() == ip && m_services[i]->Port() == port)
		{
			delete m_services[i];

			m_services[i] = NULL;

			break ;
		}
	}
}

void CServiceVector::Lock()
{
	m_lock.Lock();
}

void CServiceVector::Unlock()
{
	m_lock.Unlock();
}

/*******************************************************************************
 * @Name: CProxyTaskDispatcher(class CProxyTaskDispatcher)
 *
 * @Purpose: 网络代理的任务分发器
 * @Returns: 无
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
CProxyTaskDispatcher::CProxyTaskDispatcher(SServerConf& conf)
	: m_conf(conf)
	, m_reconnectServices(12)
{
	int inet_sock;
	struct ifreq ifr;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, "eth0");
	if (ioctl(inet_sock, SIOCGIFADDR, &ifr) < 0)
	{
		perror("ioctl on eth0");
	}

	char* strIp = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
	printf("eth0:%s\n", strIp);
	m_hostIp = strIp;
}

CProxyTaskDispatcher::~CProxyTaskDispatcher()
{
}

/*******************************************************************************
 * @Name: Dispatch(class CProxyTaskDispatcher)
 *
 * @Purpose: 分发网络数据
 * @Returns: int | 请求得到正确处理后返回1，否则返回0
 * @Parameters: CSession* | pSession[IN] 来源数据所在的会话
 * 				const char* | pData[IN] 来源数据
 * 				int | dataSize[IN] 来源数据的大小
 * @Remarks: 基类定义的虚方法
 ********************************************************************************/
int CProxyTaskDispatcher::Dispatch(CSession* pSession, const char* pData, int dataSize)
{
	bool bValid = true;

	switch (pSession->GetRole())
	{
	case NETWORK_ROLE_CLIENT:	// 作为客户角色建立的会话
	{
		OnClientReceive(pSession, const_cast<char*>(pData), dataSize);
		break ;
	}
	case NETWORK_ROLE_ACCEPT:	// 作为服务器角色建立的会话
	{
		OnServerReceive(pSession, const_cast<char*>(pData), dataSize);
		break ;
	}
	default:
	{
		bValid = false;
		break ;
	}
	}

	return bValid ? 1 : 0;
}

/*******************************************************************************
 * @Name: EventCallBack(class CProxyTaskDispatcher)
 *
 * @Purpose: 处理网络事件
 * @Returns: ...
 * @Parameters: ...
 * @Remarks: 基类定义的虚方法
 ********************************************************************************/
void CProxyTaskDispatcher::EventCallBack(CSession* pSession, short event)
{
	switch (event)
	{
	case NETWORK_STATE_UNCONNECTED:
	{
		printf("Unconnected: %s:%d!\n", pSession->GetIP(), pSession->GetPort());
		m_allSession.Remove(pSession);

		// 将在断开后需要重新连接的会话添加到重连容器

		SESSION_KIND kind = GetSessionKind(pSession->GetIP(), pSession->GetPort());
		if (kind != SESSION_KIND_UNKOWN)
		{
			m_reconnectServices.Lock();
			m_reconnectServices.Add(pSession->GetIP(), pSession->GetPort());
			m_reconnectServices.Unlock();
		}
		break ;
	}
	case NETWORK_STATE_CONNECTED:
	{
		printf("Connected: %s:%d!\n", pSession->GetIP(), pSession->GetPort());

		SessionInfo* sessionInfo = new SessionInfo;

		sessionInfo->pSession = pSession;
		sessionInfo->extra = -1;
		//gettimeofday(&sessionInfo->tmLastBeat, 0);
		struct timeval tv;
		gettimeofday(&tv, 0);
		sessionInfo->tmLastBeat = tv;
		sessionInfo->tmLastSendTime = tv;
		sessionInfo->tmLastRecvTime = tv;

		int serverPort = pSession->GetListenPort();
		if (serverPort > 0)	// 该会话为代理在某一端口接收到客户连接请求所建立的连接
		{
			sessionInfo->kind = GetSessionKind(serverPort);
		}
		else	// 该会话在代理中作为客户端被相应服务器响应建立的连接
		{
			sessionInfo->kind = GetSessionKind(pSession->GetIP(), pSession->GetPort());
		}
		m_allSession.Lock();
		m_allSession.Add(sessionInfo);
		m_allSession.Unlock();


		// 更新需要重连会话队列

		m_reconnectServices.Lock();
		for (int i = 0; i < m_reconnectServices.Size(); i++)
		{
			m_reconnectServices.Remove(pSession->GetIP(), pSession->GetPort());
		}
		m_reconnectServices.Unlock();

		break ;
	}
	case NETWORK_STATE_UNKNOWN:
	{
		break ;
	}
	}
}

/*******************************************************************************
 * @Name: ReconnectedService(class CProxyTaskDispatcher)
 *
 * @Purpose: 获得需要重连会话
 * @Returns: CSessionVector& | 所有重连会话
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
CServiceVector& CProxyTaskDispatcher::ReconnectedServices()
{
	return m_reconnectServices;
}

CSessionVector& CProxyTaskDispatcher::AllSession()
{
	return m_allSession;
}

/*******************************************************************************
 * @Name: HostIP(class CProxyTaskDispatcher)
 *
 * @Purpose: 主机IP
 * @Returns: string | 本机在eht0网卡的IP地址
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
string CProxyTaskDispatcher::HostIP()
{
	return m_hostIp;
}

string CProxyTaskDispatcher::RemoteServerIP()
{
	return m_conf.m_remoteSysIP;
}

void CProxyTaskDispatcher::SendToRemote(char* pData, int dataSize)
{
	m_allSession.Lock();
	for (int i = 0; i < m_allSession.Size(); i++)
	{
		SessionInfo* pEach = m_allSession.At(i);

		if (pEach != NULL && pEach->kind == SESSION_KIND_REMOTE_SERVER_SYS)
		{
			gettimeofday(&pEach->tmLastSendTime, NULL);

			pEach->pSession->Send(pData, dataSize);
			CTlvNetPackage pkg(pData);
			printf("type in sys msgID=%d, len=%d\n", pkg.Type() & 0xffff, pkg.Length());
			if ( (pkg.Type() & 0xffff) == 1003)
			{
				printf("Camera state >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			}
			//printf("Sent out to sys-client %s:%d\n", pEach->pSession->GetIP(), pEach->pSession->GetPort());
		}
	}
	m_allSession.Unlock();

}

/**
 * 解析一个数据包，获取中江卡口的相关信息
 */
static bool CreateDataSt4ZJ(const char* pData, int dataSize, DataSt4ZJ& item)
{
	if(dataSize < 190 ) {
		printf("Data can't convert ... len (%d) \n", dataSize);
		return false;
	}

	item.len = dataSize;  // 数据的长度和内容
	item.data = new char[dataSize];
	memcpy(item.data, pData, dataSize);

	char direct[8] = {0};
	memcpy(direct, pData + 44, 8);
	printf("Direction is : %s \n", direct);
	item.direction = string(direct); 
	item.timercv = time(NULL);

	// 0 是治安卡口, 2 是超速卡口
	int type = ntohl( *( (int*)(pData + 8) ) );
	if(type == 0) {
		item.overspeed = false;
	} else if(type == 2) {	
		item.overspeed = true;
	} else {
		printf("Error, CreateDataSt4ZJ, type unexpect (%d)\n",
				type);
		delete [] item.data;
		return false;
	}

	short year, month, day, hour, min, sec;
	year = ntohs( *( (short*)(pData + 178) ) );
	month = ntohs( *( (short*)(pData + 180) ) );
	day = ntohs( *( (short*)(pData + 182) ) );
	hour = ntohs( *( (short*)(pData + 184) ) );
	min = ntohs( *( (short*)(pData + 186) ) );
	sec = ntohs( *( (short*)(pData + 188) ) );
	printf("Create Zj struct, time %d-%d-%d %d:%d:%d\n",
			year, month, day, hour, min, sec);
	struct tm pkgtm;
	pkgtm.tm_sec = sec;
	pkgtm.tm_min = min;
	pkgtm.tm_hour = hour;
	pkgtm.tm_mon = month - 1;  // 0 - 11
	pkgtm.tm_year = year - 1900;
	pkgtm.tm_mday = day;

	item.time = mktime(&pkgtm);

	return true;
}

static bool cmp2st(const DataSt4ZJ& st1, const DataSt4ZJ& st2)
{
	if( st1.overspeed != st2.overspeed )     // 不能为同一类数据包
		if( st1.direction == st2.direction ) // 方向相同
			if( abs(st1.speed - st2.speed) < 5 )     // 速度一致
				if( abs( st1.time - st2.time ) < 2 )    // 时间接近
					return true; 
	return false;
}

// 将超速相机的数据包和 关联的治安卡口数据包合并为一个新的包
// 并将该包发送给中间件
void CProxyTaskDispatcher::CreatePkgAndSend(
		DataSt4ZJ& st1, DataSt4ZJ& st2)
{
	printf("CreatePkgAndSend ... \n");
	DataSt4ZJ* pOverspeed = NULL;
	DataSt4ZJ* pCommon = NULL;
	if( st1.overspeed ) {
		pOverspeed = &st1;
		pCommon = &st2;
	} else {
		pOverspeed = &st2;
		pCommon = &st1;
	}
	
	int commonImgLen = ntohl( *(int*)(pCommon->data + 192) );

	// 最终数据的长度等于： 超速数据包的长度 + 卡口数据图片相关固定长度18 + 卡口图片长度
	int finalLen = pOverspeed->len + 18 + commonImgLen;
	char* finalData = new char[finalLen];
	
   	// 前半部分跟超速数据一样
	memcpy(finalData, pOverspeed->data, pOverspeed->len);// 超速数据包长度
	// 覆盖车牌号码
	memcpy(finalData + 60, pCommon->data + 60, 16);
	printf("plate is: %s \n", pCommon->data + 60);
	
	memcpy(finalData + pOverspeed->len, 
			pCommon->data + 178, 
			18 + commonImgLen     // 图片相关数据（时间）及图片长度
		  ); // 拷贝治安卡口 imgNum 之后的数据

	*( (short*)(finalData + 176) ) = htons(  ntohs( *(short*)(finalData + 176) ) + 1); 
	*( (int*)(finalData + 4) ) = htonl( finalLen - 8 );  // 修改协议包中的L字段

	SendToRemote(finalData,finalLen);
	delete [] finalData;
}

// 处理中江的超速图片，在短时间内的治安卡口图片中去提取信息
void CProxyTaskDispatcher::DealWithSt4ZJ(DataSt4ZJ& st)
{
	// 静态的容器，用来保存短时间内的数据包
	static list<DataSt4ZJ> pkgs; 
	
	bool find = false;

	// 遍历容器, 通过比较函数查找同一个车的数据包
	for(list<DataSt4ZJ>::iterator it = pkgs.begin(); 
			it != pkgs.end(); it++){
		if( cmp2st(st,(*it)) ) {
			CreatePkgAndSend(*it, st);
			find = true;
			delete [] (*it).data; // 删除容器中匹配上的数据
			pkgs.erase(it);
			break;
		}
	}

	if(find)  
		// 如果已经处理完了，则释放这个包的资源
		delete [] st.data;
	else 
		pkgs.push_back(st);
	

	// 开始清理过期数据
	time_t now = time(NULL);
	for(list<DataSt4ZJ>::iterator it = pkgs.begin(); 
			it != pkgs.end(); it++)
	{
		printf("now(%u), timercv(%u) \n",now, (*it).timercv );
		if( abs( now - (*it).timercv ) > 5 ) 
		{
			printf("Drop one packge .....\n");
			delete [] (*it).data;
			pkgs.erase(it);
			it = pkgs.begin(); // 内容已经改变，重新获取迭代器
		}
	}

	printf( "Size of list is: %d \n",pkgs.size() );

}

void CProxyTaskDispatcher::DoZjConvert(
		char* pData, int dataSize)
{
	int type = 0;
	type = ntohl( *( (int*)pData ) ) & 0x0fffffff;
	printf("In ZJ convert, type is: %d\n",type);
	if( type != 1000 ) {
		// 不是车辆数据包,直接发送
		SendToRemote(pData, dataSize);
		return;
	} else {
		if( dataSize < 112 ) {  // 无法取到偏移为108的超速值
			printf("dataSize is too small (%d) \n",dataSize);
			return;
		}
		// 再判断是否超速
		int speed = 0, limitSpeed = 0;
		speed = ntohl( *( (int*)(pData + 104) ) );
		limitSpeed = ntohl( *( (int*)(pData + 108) ) );
		printf("In Zj convert, speed = %d, limitSpeed = %d \n",
				speed, limitSpeed);
		if( speed <= limitSpeed ) {
			// 如果没有超速也直接发送
			SendToRemote(pData, dataSize);
			return;
		} else {
			DataSt4ZJ st;
			st.speed = speed;
			// 将数据包解析出来
			if( CreateDataSt4ZJ(pData, dataSize, st) )
				// 处理解析好的数据包
				DealWithSt4ZJ(st);
		}
	}
	return;
}

/*******************************************************************************
 * @Name: OnClientReceive(class CProxyTaskDispatcher)
 *
 * @Purpose: 处理角色为客户的请求
 * @Returns: void
 * @Parameters: CSession* | pSession[IN] 当前客户所在的会话
 * 				char* | pData[IN] 从客户端接收到的数据（请求）
 * 				int | dataSize[IN] 接收到的数据的长度
 * @Remarks: 无
 ********************************************************************************/
void CProxyTaskDispatcher::OnClientReceive(CSession* pSession, char* pData, int dataSize)
{
	// SessionInfo* pSessionInfo = NULL;

	int kind = SESSION_KIND_UNKOWN;

	m_allSession.Lock();
	for (int i = 0; i < m_allSession.Size(); i++)
	{
		SessionInfo* pEach = m_allSession.At(i);

		if (pEach != NULL && pEach->pSession == pSession)
		{
			kind = pEach->kind;

			gettimeofday(&pEach->tmLastRecvTime, NULL);
		}
	}
	m_allSession.Unlock();

	CTlvNetPackage pkg(pData);

	printf("CProxyTaskDispatcher::OnClientReceive kind = %d\n", kind);

	switch (kind)
	{
	case SESSION_KIND_CLIENT_CAMERA:
	{
		printf("Received from camera top %s:%d\n", pSession->GetIP(), pSession->GetPort());
		CTlvLEPackage* cdPkg;
		CTlvNetPackage stdPkg(pData);

		int cameraNo;

		cdPkg = StdPkgToCd(stdPkg, &cameraNo);
		cameraNo--;
		if (cdPkg == NULL)
		{
			break ;
		}

		int virtualType = pkg.Type() << 1 >> 1;

		if (virtualType == MSG_TYPE_CAM_VIDEO || virtualType == MSG_TYPE_CAM_PARAM_SETTING_REPLY ||
				virtualType == MSG_TYPE_CAM_PARAM_REPLY)
		{
			m_allSession.Lock();
			for (int i = 0; i < m_allSession.Size(); i++)
			{
				SessionInfo* pEach = m_allSession.At(i);

				if (pEach != NULL && pEach->kind == SESSION_KIND_ACCEPT_CAMREA)
				{
					if (pEach->extra == cameraNo && pEach->pSession != NULL)
					{
						int ret = pEach->pSession->Send(cdPkg->TlvData(), cdPkg->Size());
						printf("Send out to camera-client: size=%d, ret=%d, ip=%s, port=%d\n", cdPkg->Size(), ret, pEach->pSession->GetIP(), pEach->pSession->GetListenPort());
					}
				}
			}
			m_allSession.Unlock();
		}

		delete cdPkg;
		break ;
	}
	case SESSION_KIND_CLIENT_SYS:
	{
		// 收到卡口程序发来的数据包，转发给中间件
		// 现在不直接转发，而是做一些转换
		DoZjConvert(pData,dataSize);
		break ;
	}
	/* 这部分现在放到作为服务端的代码里面去,因为中江的中间件是作为客户端连接代理的
	case SESSION_KIND_REMOTE_SERVER_SYS:
	{
		printf("get remote(sys) from %s\n", pSession->GetIP());

		m_allSession.Lock();
		for (int i = 0; i < m_allSession.Size(); i++)
		{
			SessionInfo* pEach = m_allSession.At(i);

			if (pEach != NULL && pEach->pSession != NULL)
			{
				if (pEach->kind == SESSION_KIND_CLIENT_SYS)
				{
					gettimeofday(&pEach->tmLastSendTime, NULL);

					pEach->pSession->Send(pData, dataSize);
				}
			}
		}
		m_allSession.Unlock();
		break ;
	}
	*/
	case SESSION_KIND_CLIENT_DEBUG:
	{
		m_allSession.Lock();
		for (int i = 0; i < m_allSession.Size(); i++)
		{
			SessionInfo* pEach = m_allSession.At(i);

			if (pEach != NULL && pEach->kind == SESSION_KIND_ACCEPT_DEBUG)
			{
				//printf("Received debug info from %s: %d\n", pEach->pSession->GetIP(), pEach->pSession->GetPort());
				pEach->pSession->Send(pData, dataSize);
			}
		}
		m_allSession.Unlock();
		break ;
	}
	case SESSION_KIND_CLIENT_CAR_DETECTOR:
	{
		m_allSession.Lock();
		for (int i = 0; i < m_allSession.Size(); i++)
		{
			SessionInfo* pEach = m_allSession.At(i);

			if (pEach != NULL && pEach->kind == SESSION_KIND_ACCEPT_SYS)
			{
				//printf("Received car detector info from %s: %d\n", pEach->pSession->GetIP(), pEach->pSession->GetPort());
				pEach->pSession->Send(pData, dataSize);
			}
		}
		m_allSession.Unlock();
		break ;
	}
	default:
	{
		break ;
	}
	}
}

struct ThreadArgs
{
	CSession* m_pSession;
	int m_type;	// 回应类型
	char* m_data;
	int m_dataSize;
};


/*******************************************************************************
 * @Name: ReplyThread
 *
 * @Purpose: 相机参数应答参数
 * @Returns: void* | 线程返回值
 * @Parameters: void* | arg[IN] 向请求相机的客户端发送参数信息
 * @Remarks: 无
 ********************************************************************************/
void* ReplyThread(void* arg)
{
	pthread_detach(pthread_self());
	ThreadArgs* pArgs = (ThreadArgs*)arg;

	switch (pArgs->m_type)
	{
	case MSG_TYPE_CAM_PARAM_REPLY:
	{
		CTlvLEPackage replyPkg(MSG_CAMERA_CD::TYPE_HEADER, 4 + pArgs->m_dataSize);
		replyPkg.PutInt(MSG_CAMERA_CD::TYPE_PARAM_REPLY);
		replyPkg.PutBytes(pArgs->m_data, pArgs->m_dataSize);
		pArgs->m_pSession->Send(replyPkg.TlvData(), replyPkg.Size());

		break ;
	}
	case MSG_TYPE_CAM_BEAT_REPLY:
	{
		CTlvLEPackage tlvBeat(MSG_CAMERA_CD::TYPE_HEADER, 4 * 4);

		for (int i = 0; i < 4; i++)
		{
			tlvBeat.PutInt(MSG_CAMERA_CD::TYPE_BEAT);
		}
		pArgs->m_pSession->Send(tlvBeat.TlvData(), tlvBeat.Size());

		break ;
	}
	case MSG_TYPE_CAM_PARAM_SETTING_REPLY:
	{
		CTlvLEPackage tlvBeat(pArgs->m_data);

		pArgs->m_pSession->Send(tlvBeat.TlvData(), tlvBeat.Size());

		break ;
	}
	default:
	{
		break ;
	}
	}
	sleep(2);

	delete[] pArgs->m_data;
	delete pArgs;
	return NULL;
}

/*******************************************************************************
 * @Name: OnServerReceive(class CProxyTaskDispatcher)
 *
 * @Purpose: 服务器处理客户端请求
 * @Returns: void
 * @Parameters: CSession* | pSession[IN] 对应一路客户连接
 * 				char* | pData[IN] 服务器端口收到的数据
 * 				int | dataSize[IN] 服务器端口收到的数据的大小
 * @Remarks: 无
 ********************************************************************************/
void CProxyTaskDispatcher::OnServerReceive(CSession* pSession, char* pData, int dataSize)
{
	SessionInfo* pSessionInfo = NULL;

	int kind = SESSION_KIND_UNKOWN;

	m_allSession.Lock();
	for (int i = 0; i < m_allSession.Size(); i++)
	{
		SessionInfo* pEach = m_allSession.At(i);

		if (pEach != NULL && pEach->pSession == pSession)
		{
			pSessionInfo = pEach;
			kind = pSessionInfo->kind;
			break ;
		}
	}
	m_allSession.Unlock();

	if (pSessionInfo != NULL)
	{
		gettimeofday(&pSessionInfo->tmLastBeat, 0);
	}

	switch (kind)
	{
		case SESSION_KIND_ACCEPT_CAMREA:
			{
				CTlvNetPackage* stdPkg;
				CTlvLEPackage cdPkg(pData);
				stdPkg = CdPkgToStd(cdPkg, pSessionInfo->extra + 1);

				if (stdPkg == NULL)
				{
					break ;
				}

				switch (stdPkg->Type() << 1 >> 1)
				{
					case MSG_TYPE_CAM_SWITCH:
						{
							cdPkg.SeekToBegin();
							pSessionInfo->extra = cdPkg.GetInt();
							printf("Request(Camera switch camera to: %d) from %s\n", \
									pSessionInfo->extra, pSession->GetIP());
							break ;
						}
					case MSG_TYPE_CAM_PARAM_QUERY:
						{
							if (pSessionInfo->extra < 0)
							{
								printf("Request(Camera parameters query) from %s.\n" \
										"No camera is specified for control, need switch to a camera\n", \
										pSession->GetIP());
								break ;
							}

							//printf("Request(Camera parameters query) from %s\n", pSession->GetIP());

							SendPkgToCameraServer(*stdPkg);

							break ;
						}
					case MSG_TYPE_CAM_BEAT:
						{
							//			if (pSessionInfo->extra < 0)
							//			{
							//				printf("Camera heart beating request from %s.\n"	\
							//						"No camera is specified for control, may no switch to a camera\n", \
							//						pSession->GetIP());
							//				break ;
							//			}

							//printf("Request(Camera heart beating) from %s\n", pSession->GetIP());

							char* buff = new char[16];

							memset(buff, 0xaa, sizeof(buff));

							ThreadArgs* pArgs = new ThreadArgs;

							pArgs->m_pSession = pSession;
							pArgs->m_type = MSG_TYPE_CAM_BEAT_REPLY;
							pArgs->m_data = buff;
							pArgs->m_dataSize = 16;

							gettimeofday(&pSessionInfo->tmLastBeat, 0);

							pthread_t sendThread;
							pthread_create(&sendThread, NULL, &ReplyThread, pArgs);
							break ;
						}
					case MSG_TYPE_CAM_PARAM_SETTING:
						{
							if (pSessionInfo->extra < 0)
							{
								printf("Camera parameter setting request from %s.\n"
										"No camera is specified for control, may no switch to a camera\n",
										pSession->GetIP());
								break ;
							}

							printf("Request(Camera parameter setting) from %s\n", pSession->GetIP());

							CTlvNetPackage tmpStdPkg(stdPkg->TlvData());

							SendPkgToCameraServer(*stdPkg);

							break ;
						}
					case MSG_TYPE_CAM_RESET:
						{
							if (pSessionInfo->extra < 0)
							{
								printf("Camera reset request from %s.\n"
										"No camera is specified for control, may no switch to a camera\n",
										pSession->GetIP());
								break ;
							}

							//printf("Request(Camera reset) from %s\n", pSession->GetIP());

							SendPkgToCameraServer(*stdPkg);
							break ;
						}
					case MSG_TYPE_CAM_TRIGGER:
						{
							if (pSessionInfo->extra < 0)
							{
								printf("Camera trigger request coming from %s.\n"
										"No camera is specified for control, may no switch to a camera\n",
										pSession->GetIP());
								break ;
							}

							//printf("Request(Camera trigger) from %s\n", pSession->GetIP());

							SendPkgToCameraServer(*stdPkg);
							break ;
						}
					default:
						{
							printf("Request(Camera unknown) from %s\n", pSession->GetIP());
							break ;
						}
				}

				delete stdPkg;

				break ;
			}
		case SESSION_KIND_ACCEPT_SYS:
			{
				//printf("Request(sys) from %s\n", pSession->GetIP());

				m_allSession.Lock();
				for (int i = 0; i < m_allSession.Size(); i++)
				{
					SessionInfo* pEach = m_allSession.At(i);

					if (pEach != NULL && pEach->pSession != NULL)
					{
						if (pEach->kind == SESSION_KIND_CLIENT_SYS)
						{
							pEach->pSession->Send(pData, dataSize);
						}
					}
				}
				m_allSession.Unlock();
				break ;
			}
		case SESSION_KIND_ACCEPT_DEBUG:
			{
				//printf("Request(Debug) from %s\n", pSession->GetIP());
				break ;
			}
		case SESSION_KIND_ACCEPT_CAR_DETECTOR:
			{
				//printf("Request(Car Detector) from %s\n", pSession->GetIP());
				break ;
			}
		case SESSION_KIND_REMOTE_SERVER_SYS:
			{
				// 收到中间件的数据直接转发给卡口程序
				printf("get remote(sys) from %s\n", pSession->GetIP());

				m_allSession.Lock();
				for (int i = 0; i < m_allSession.Size(); i++)
				{
					SessionInfo* pEach = m_allSession.At(i);

					if (pEach != NULL && pEach->pSession != NULL)
					{
						if (pEach->kind == SESSION_KIND_CLIENT_SYS)
						{
							gettimeofday(&pEach->tmLastSendTime, NULL);

							pEach->pSession->Send(pData, dataSize);
						}
					}
				}
				m_allSession.Unlock();
				break ;
			}
		default:
			{
				break ;
			}
	}
}

////////////////////////////for test////////////////////////////
char* GetImageFromFile(const char* fileName, int* size)
{
	FILE* fp = fopen(fileName, "r");
	if (fp == NULL)
	{
		*size = 0;
		printf("Can't open file: 1.jpeg\n");
		return NULL;
	}

	char* frameData;

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	frameData = new char[*size];
	memset(frameData, 0, *size);
	fseek(fp, 0L, SEEK_SET);
	fread(frameData, *size, 1, fp);
	fclose(fp);

	return frameData;
}


/*******************************************************************************
 * @Name: Run(class CProxyTaskDispatcher)
 *
 * @Purpose: 作为测试每隔一固定时间向客户端发送相机心跳信息以及相机捕获到的图像帧
 * @Returns: void
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
void CProxyTaskDispatcher::Run()
{
	while (1)
	{
		struct timeval tmEnd;

		gettimeofday(&tmEnd, 0);

		SessionInfo* pUnconnected = NULL;

		m_allSession.Lock();
		for (int i = 0; i < m_allSession.Size(); i++)
		{
			SessionInfo* pEach = m_allSession.At(i);

			if (pEach != NULL && pEach->pSession != NULL)
			{
				if (pEach->kind == SESSION_KIND_ACCEPT_CAMREA)
				{
					if (tmEnd.tv_sec - pEach->tmLastBeat.tv_sec >= 15)
					{
						printf("Server has shut down the connection with client: %s\n",
														pEach->pSession->GetIP());
						pEach->pSession->Shutdown();
						pEach->pSession = NULL;
						pEach->extra = -1;
						pEach->kind = SESSION_KIND_UNKOWN;
						break ;
					}
				}
			}
		}

		if (pUnconnected != NULL)
		{
			m_allSession.Remove(pUnconnected->pSession);
		}
		m_allSession.Unlock();

		sleep(5);
	}
}
////////////////////////////for test////////////////////////////

/*******************************************************************************
 * @Name: GetSessionKind(class CProxyTaskDispatcher)
 *
 * @Purpose: 根据服务器端口号获得客户端请求服务的会话类型
 * @Returns: 客户端请求的会话类型
 * @Parameters: int | nPort[IN] 端口号
 * @Remarks: 无
 ********************************************************************************/
SESSION_KIND CProxyTaskDispatcher::GetSessionKind(int nPort)
{
	SESSION_KIND kind;

	// 现在代理作为中间件的服务端，在这里返回类型
	if(m_conf.m_remoteSysPort == nPort)
	{
		kind = SESSION_KIND_REMOTE_SERVER_SYS;
	}
	else if (m_conf.m_portSrvCamera == nPort)
	{
		kind = SESSION_KIND_ACCEPT_CAMREA;
	}
	else if (m_conf.m_portSrvSys == nPort)
	{
		kind = SESSION_KIND_ACCEPT_SYS;
	}
	else if (m_conf.m_portSrvDebug == nPort)
	{
		kind = SESSION_KIND_ACCEPT_DEBUG;
	}
	else if (m_conf.m_portSrvCarDetector == nPort)
	{
		kind = SESSION_KIND_ACCEPT_CAR_DETECTOR;
	}
	else
	{
		kind = SESSION_KIND_UNKOWN;
	}

	return kind;
}

/*******************************************************************************
 * @Name: GetSessionKind(class CProxyTaskDispatcher)
 *
 * @Purpose: 根据客户端请求的端口号确定会话类型
 * @Returns: 客户端角色发起请求的会话类型
 * @Parameters: const char* | strIp[IN] 服务器IP
 * 				int | nPort[IN] 服务器端口号
 * @Remarks: 无
 ********************************************************************************/
SESSION_KIND CProxyTaskDispatcher::GetSessionKind(const char* strIp, int nPort)
{
	SESSION_KIND kind = SESSION_KIND_UNKOWN;

	/* 这个版本的代理作为中间件的服务端，所以不在这里返回
	if (nPort == m_conf.m_remoteSysPort && !strcmp(strIp, m_conf.m_remoteSysIP.c_str()))
	{
		return SESSION_KIND_REMOTE_SERVER_SYS;
	}
	*/

	printf("remote IP is: %s, port is: %d \n", strIp, nPort);
	if (!strcmp(strIp, m_hostIp.c_str()) ||
			!strcmp(strIp, "127.0.0.1"))
	{
		for (int i = 0; i < m_conf.m_cameraNum; i++)
		{
		printf("port = %d, m_conf.m_sysPorts[%d] = %d, m_conf.m_debugPorts[%d] = %d\n", nPort, i, m_conf.m_sysPorts[i], i, m_conf.m_debugPorts[i]);
			if (nPort == m_conf.m_cameraPorts[i])
			{
				kind = SESSION_KIND_CLIENT_CAMERA;
				break ;
			}
			else if (nPort == m_conf.m_sysPorts[i])
			{
				kind = SESSION_KIND_CLIENT_SYS;
				break ;
			}
			else if (nPort == m_conf.m_debugPorts[i])
			{
				kind = SESSION_KIND_CLIENT_DEBUG;
				break ;
			}
			else if (nPort == m_conf.m_carDetector)
			{
				kind = SESSION_KIND_CLIENT_CAR_DETECTOR;
				break ;
			}
		}
	}

	return kind;
}

/*******************************************************************************
 * @Name: StdPkgToCd(class CProxyTaskDispatcher)
 *
 * @Purpose: 将标准的内部通信请求包转换为川大智胜的网络请求包
 * @Returns: CTlvLEPackage* | 为内部通信请求包创建的转换为后的川大智胜的网络请求包
 * @Parameters: CTlvNetPackage& | cdPkg[IN] 从内部接收到的原始请求包
 * 				int* | cameraNo[OUT] 内部回应川大智胜请求控制的相机的编号
 * @Remarks: 返回的请求包需要在适当的时机释放资源
 ********************************************************************************/
CTlvLEPackage* CProxyTaskDispatcher::StdPkgToCd(CTlvNetPackage& stdPkg, int* cameraNo)
{
	int stdType = stdPkg.Type() << 1 >> 1;

	if (stdType <= MSG_TYPE_CAM_START || stdType >= MSG_TYPE_CAM_END)
	{
		*cameraNo = -1;
		return NULL;
	}

	CTlvLEPackage* cdPkg = NULL;
	printf("From inner server: %d\n", stdType);
	switch (stdType)
	{
	case MSG_TYPE_CAM_VIDEO:
	{
		cdPkg = new CTlvLEPackage(MSG_CAMERA_CD::TYPE_HEADER, stdPkg.Length());
		cdPkg->PutBytes(stdPkg.GetBytes(stdPkg.Length() - 4), stdPkg.Length() - 4);
		*cameraNo = stdPkg.GetInt();
		cdPkg->PutInt(*cameraNo-1);//注：这里跟川大智胜的相机id开始序号不一致的问题。
		break ;
	}
	case MSG_TYPE_CAM_PARAM_SETTING_REPLY:
	{
		cdPkg = new CTlvLEPackage(MSG_CAMERA_CD::TYPE_HEADER, stdPkg.Length());
		*cameraNo = stdPkg.GetInt();
		cdPkg->PutInt(MSG_CAMERA_CD::TYPE_PARAM_SETTING_REPLY);
		cdPkg->PutInt(stdPkg.GetInt());
		cdPkg->PutInt(stdPkg.GetInt());
		break ;
	}
	case MSG_TYPE_CAM_PARAM_REPLY:
	{
		*cameraNo = stdPkg.GetInt();
		printf("NetworkProxy - - > cameraNo = %d\n", *cameraNo);

		cdPkg = new CTlvLEPackage(MSG_CAMERA_CD::TYPE_HEADER, stdPkg.Length());
		cdPkg->PutInt(MSG_CAMERA_CD::TYPE_PARAM_REPLY);
//		cdPkg->PutBytes(stdPkg.GetBytes(stdPkg.Length() - 4), stdPkg.Length() - 4);

		int *param = new int[0x160];
		for (int i = 0; i < 0x160; i++)
		{
			param[i] = stdPkg.GetInt();
			if (i == 0)
			{
				param[0] = param[0]-1;	//注：这里跟川大智胜的相机id开始序号不一致的问题。
			}
			cdPkg->PutInt(param[i]);
		}

		printf("NetworkProxy - - > id = %d\n", param[0x00]);
		printf("NetworkProxy - - > gain = %d\n", param[0x90]);
		printf("NetworkProxy - - > fps = %d\n", param[0x92]);
		printf("NetworkProxy - - > Shutter = %d\n", param[0x93]);
		printf("NetworkProxy - - > BalanceMode = %d\n", param[0xd2]);
		printf("NetworkProxy - - > Contrast = %d\n", param[0xe0]);
		printf("NetworkProxy - - > JpegQuality = %d\n", param[0x128]);
		printf("NetworkProxy - - > ExpEnable = %d\n", param[0x130]);

		delete[] param;
		break ;
	}
	case MSG_TYPE_CAM_BEAT:
	{
//		cdPkg = new CTlvLEPackage(MSG_CAMERA_CD::TYPE_HEADER, 16);
//		for (int i = 0; i < 4; i++)
//		{
//			cdPkg->PutInt(MSG_CAMERA_CD::TYPE_BEAT);
//		}
		break ;
	}
	default:
	{
		*cameraNo = -1;
		break ;
	}
	}

	return cdPkg;
}

/*******************************************************************************
 * @Name: CdPkgToStd(class CProxyTaskDispatcher)
 *
 * @Purpose: 将川大智胜的网络请求包转换为标准的内部通信请求包
 * @Returns: CTlvNetPackage* | 为川大智胜的网络请求包创建的转换为后的内部通信请求包
 * @Parameters: CTlvLEPackage& | cdPkg[IN] 从川大智胜接收到的原始请求包
 * 				int | cameraNo[IN] 川大智胜请求控制的相机的编号
 * @Remarks: 返回的请求包需要在适当的时机释放资源
 ********************************************************************************/
CTlvNetPackage* CProxyTaskDispatcher::CdPkgToStd(CTlvLEPackage& cdPkg, int cameraNo)
{
	CTlvNetPackage* stdPkg = NULL;

	switch (cdPkg.Type())
	{
	case MSG_CAMERA_CD::TYPE_HEADER:
	{
		stdPkg = CdCameraPkgToStd(cdPkg, cameraNo);
		break ;
	}
	default:
	{
		break ;
	}
	}

	return stdPkg;
}

/*******************************************************************************
 * @Name: CdCameraPkgToStd(class CProxyTaskDispatcher)
 *
 * @Purpose: 处理川大智胜发送的与相机相关的请求包，并将其转换为标准的内部请求包
 * @Returns: CTlvNetPackage* | 为川大智胜的网络请求包创建的转换为后的内部通信请求包
 * @Parameters: CTlvLEPackage& | package[IN] 从川大智胜接收到的原始相机相关的请求包
 * 				int | cameraNo[IN] 川大智胜请求控制的相机的编号
 * @Remarks:
 ********************************************************************************/
CTlvNetPackage* CProxyTaskDispatcher::CdCameraPkgToStd(CTlvLEPackage& package, int cameraNo)
{
	int kind = MSG_TYPE_UNKNOWN;
	int type = package.GetInt();

	switch (type)
	{
	case MSG_CAMERA_CD::TYPE_SWITCH_0:
	case MSG_CAMERA_CD::TYPE_SWITCH_1:
	case MSG_CAMERA_CD::TYPE_SWITCH_2:
	case MSG_CAMERA_CD::TYPE_SWITCH_3:
	case MSG_CAMERA_CD::TYPE_SWITCH_4:
	{
		kind = MSG_TYPE_CAM_SWITCH;
		break ;
	}
	case MSG_CAMERA_CD::TYPE_PARAM:
	{
		if (package.Length() == 4)
		{
			kind = MSG_TYPE_CAM_PARAM_QUERY;
		}
		else if(package.Length() == 3 * 4)
		{
			kind = MSG_TYPE_CAM_PARAM_SETTING;
		}
		break ;
	}
	case MSG_CAMERA_CD::TYPE_RESET:
	{
		kind = MSG_TYPE_CAM_RESET;
		break ;
	}
	case MSG_CAMERA_CD::TYPE_TRIGGER:
	{
		kind = MSG_TYPE_CAM_TRIGGER;
		break ;
	}
	case MSG_CAMERA_CD::TYPE_BEAT:
	{
		kind = MSG_TYPE_CAM_BEAT;
		break ;
	}
	default:
	{
		break ;
	}
	}


	CTlvNetPackage* stdPkg = NULL;

	switch (kind)
	{
	case MSG_TYPE_CAM_SWITCH:
	{
		stdPkg = new CTlvNetPackage(kind, sizeof(int));
		stdPkg->PutInt(type/*cameraNo*/);

//		printf("Switch camera to: %d***************************************\n", type);
		break ;
	}
	case MSG_TYPE_CAM_PARAM_QUERY:
	{
		stdPkg = new CTlvNetPackage(kind, sizeof(int));
		stdPkg->PutInt(cameraNo);	// 相机号

//		printf("Query camera params****************************************\n");
		break ;
	}
	case MSG_TYPE_CAM_PARAM_SETTING:
	{
		stdPkg = new CTlvNetPackage(kind, 3 * sizeof(int));

		int paramNo, paramVal;

		paramNo = package.GetInt();
		paramVal = package.GetInt();

		stdPkg->PutInt(cameraNo);
		stdPkg->PutInt(paramNo);			// 相机参数号
		stdPkg->PutInt(paramVal);			// 相机参数值

//		printf("Camera param setting:**************************************\n");
//		printf("\tcameraNo: %d\n", cameraNo);
//		printf("\t paramNo: 0x%x\n", paramNo);
//		printf("\tparamVal: 0x%x\n", paramVal);

		break ;
	}
	case MSG_TYPE_CAM_RESET:
	{
		stdPkg = new CTlvNetPackage(kind, sizeof(int));
		stdPkg->PutInt(cameraNo);

//		printf("Reset current camera**************************************\n");
		break ;
	}
	case MSG_TYPE_CAM_TRIGGER:
	{
		stdPkg = new CTlvNetPackage(kind, sizeof(int));
		stdPkg->PutInt(cameraNo);

//		printf("Trigger camera********************************************\n");
		break ;
	}
	case MSG_TYPE_CAM_BEAT:
	{
		stdPkg = new CTlvNetPackage(kind, 4 * sizeof(int));
		for (int i = 0; i < 4; i++)
		{
			stdPkg->PutInt(MSG_CAMERA_CD::TYPE_BEAT);
		}

//		printf("Heart beating********************************************\n");
		break ;
	}
	default:
	{
//		printf("Unknown message from Camera*******************************\n");
		break ;
	}
	}
//	printf("\n");

	return stdPkg;
}

/*******************************************************************************
 * @Name: SendPkgToCameraServer(class CProxyTaskDispatcher)
 *
 * @Purpose: 将川大智胜发送的相机相关请求包转换后的内部数据包发送给最终的参数配置
 * 			服务器
 * @Returns: CTlvNetPackage* | 为川大智胜的网络请求包创建的转换为后的内部通信请求包
 * @Parameters: CTlvNetPackage& | package[IN]
 * @Remarks:
 ********************************************************************************/
void CProxyTaskDispatcher::SendPkgToCameraServer(CTlvNetPackage& package)
{
	m_allSession.Lock();
	for (int i = 0; i < m_allSession.Size(); i++)
	{
		SessionInfo* pEach = m_allSession.At(i);

		if (pEach != NULL && pEach->pSession != NULL)
		{
			if (SESSION_KIND_CLIENT_CAMERA == GetSessionKind(pEach->pSession->GetIP(), pEach->pSession->GetPort()))
			{
				pEach->pSession->Send(package.TlvData(), package.Size());
			}
		}
	}
	m_allSession.Unlock();
}



