
/* 车辆信息转换时通用的一些结构体和函数
 */

#ifndef CAR_INFO_CVT_COMMON_H
#define CAR_INFO_CVT_COMMON_H

#include "../include/pdebug.h"
#include "ProxyTaskDispatcher.h"

#include <arpa/inet.h> // for htons
#include <stdlib.h> // for abs

static const int CAR_MESSAGE_TYPE = 1000;

typedef struct pkg_info_and_data {
	time_t timercv; // 接收到该包的时刻
	time_t time;    // 作为判断两个包关系的依据之一

	bool overspeed; // 是否是超速数据

	int speed;      // 车辆的速度

	int road;       // 车道信息
	std::string direction; // 方向信息

	struct TCPConn from; // 数据包所来自的连接

	int len;        // 数据包的长度
	char *data;     // 数据包的数据
} PKG_INFO_DATA;

class CProxyTaskDispatcher;
extern CProxyTaskDispatcher *g_dispatcher;

inline void SendOneStOut(struct TCPConn from, const char *data, int size) 
{
	PDEBUG("Now should send an cached package (time out).\n");
	if(g_dispatcher)
		g_dispatcher->SendResultToOtherSide(from, data, size);
	else
		PDEBUG("Error, send data but g_dispatcher is NULL");
}

inline void ResponsePackID(const char *data, CSession *psession)
{
	char pkg[73];
	memset(pkg,0, 73);
	*( (short*)(pkg + 2) ) = htons((short)CAR_MESSAGE_TYPE);
	*( (int *) (pkg + 4) ) = htonl(64);
	memcpy(pkg+8, data, 64);
	PDEBUG("We send an ack to source session, it's %s\n", pkg+8);
	psession->Send( pkg, 72);
}

/* cmp2st 比较两个结构提是否匹配
 * 匹配条件:
 *   1. 一个是超速一个是治安卡口 
 *   2. 方向相同， 速度与时间差小
 */
inline bool cmp2pkg( const PKG_INFO_DATA& st1, const PKG_INFO_DATA& st2)
{
	if( st1.overspeed != st2.overspeed ) {
		if( st1.direction == st2.direction ) {
			if(st1.road == st2.road ) {
				if( abs( (int)(st1.speed - st2.speed) ) < 5 ) {
					if( abs( (int)(st1.time - st2.time) ) <= 2 ) {
						return true; 
					}
				}
			}
		}
	}
	return false;
}

#endif 

