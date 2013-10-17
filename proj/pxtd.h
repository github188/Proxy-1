/*
 * ProxyTaskDispatcher.h
 *
 *  Created on: 2011-1-24
 *      Author: cchao
 */

#ifndef PROXYTASKDISPATCHER_H_
#define PROXYTASKDISPATCHER_H_

#include "package.h"
#include "Thread/Mutex.h"
#include "Network/TaskDispatcher.h"

#include <sys/time.h>
#include <vector>
#include <string>

enum SESSION_KIND {		// 标志会话来源
	SESSION_KIND_UNKOWN = -1,
	SESSION_KIND_CLIENT_CAMERA,		// 相机控制
	SESSION_KIND_CLIENT_SYS,		// 系统信息（包括：硬件部件状态，车牌识别结果）
	SESSION_KIND_CLIENT_DEBUG,		// 调试信息
	SESSION_KIND_CLIENT_CAR_DETECTOR, 		// 车检器信息
	SESSION_KIND_ACCEPT_CAMREA,		// 代理相机控制服务对象
	SESSION_KIND_ACCEPT_SYS,		// 代理系统信息服务对象
	SESSION_KIND_ACCEPT_DEBUG,		// 代理debug信息服务对象
	SESSION_KIND_ACCEPT_CAR_DETECTOR, //
	SESSION_KIND_REMOTE_SERVER_SYS	//远程服务器
};

typedef struct {
	int len;  // data的长度
	char* data; // 数据包的原始内容
	std::string direction; // 方向信息
	int speed;  
	time_t timercv; // 接收到该包的时刻
	time_t time;    // 该包中时间字段的值
	bool overspeed; // 是否是超速的数据
} DataSt4ZJ;


struct SessionInfo
{
	SESSION_KIND kind;	// 会话类别
	CSession* pSession;
	int extra;			// 在相机控制会话中，作为相机编号（<0时无效）
	struct timeval tmLastBeat;		// 上一次心跳发生的时间（目前仅用于相机端）
	struct timeval tmLastSendTime;
	struct timeval tmLastRecvTime;
};

class CSessionVector
{
public:
	CSessionVector(int size = 20);
	virtual ~CSessionVector();
	int Size();
	int Count();
	SessionInfo* At(int index);
	void Add(SessionInfo* pSessionInfo);
	void Remove(CSession* pSession);
	void Lock();
	void Unlock();
private:
	CMutex m_lock;
	vector<SessionInfo*> m_sessions;
	int m_count;
};

class CService
{
public:
	CService(const string ip, unsigned short port)
		: m_ip(ip)
		, m_port(port)
	{
	}
	string IP()
	{
		return m_ip;
	}
	unsigned short Port()
	{
		return m_port;
	}
private:
	string m_ip;
	unsigned short m_port;
};

class CServiceVector
{
public:
	CServiceVector(int size);
	int Size();
	CService* At(int index);
	void Add(string ip, unsigned short port);
	void Remove(string ip, unsigned short port);
	void Lock();
	void Unlock();
private:
	vector<CService*> m_services;
	CMutex m_lock;
};

struct SServerConf
{
	int m_cameraNum;
	string m_remoteSysIP;
	unsigned short m_remoteSysPort;
	unsigned short m_sysPorts[16];
	unsigned short m_debugPorts[16];
	unsigned short m_cameraPorts[16];
	unsigned short m_carDetector;
	
	unsigned short m_portSrvCamera;
	unsigned short m_portSrvSys;
	unsigned short m_portSrvDebug;
	unsigned short m_portSrvCarDetector;
};


#endif /* PROXYTASKDISPATCHER_H_ */
