
/* 车辆信息转换时通用的一些结构体和函数
 */

#ifndef CAR_INFO_CVT_COMMON_H
#define CAR_INFO_CVT_COMMON_H

#include "../include/pdebug.h"
#include "ProxyTaskDispatcher.h"

#include <arpa/inet.h> // for htons
#include <stdlib.h> // for abs
#include <sys/time.h>

static const int CAR_MESSAGE_TYPE = 1000;
static const int ACK_MESSAGE_LEN = 64;

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

inline void send_one_pkg(struct TCPConn from, const char *data, int size) 
{
	PDEBUG("Now should send an cached package (time out).\n");
	if(g_dispatcher)
		g_dispatcher->SendResultToOtherSide(from, data, size);
	else
		PDEBUG("Error, send data but g_dispatcher is NULL");
}

inline void response_packet_id(const char *data, CSession *psession)
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
	//			if( st1.speed == st2.speed ) {
				if( abs( (int)(st1.speed - st2.speed) ) < 2 ) {
					if( abs( (int)(st1.time - st2.time) ) <= 2 ) {
						return true; 
					}
				}
			}
		}
	}
	return false;
}

inline const char* uuid( const char* str)
{
	short space = 0;
	int time = 0;
	static char uuid[64];
	memset(uuid,0,64);
	for(unsigned i = 0; str[i] != 0; i++)
		space += str[i];

	struct timeval tv;
	gettimeofday(&tv,NULL);
	srandom((unsigned int)(tv.tv_sec + tv.tv_usec));
	time = random();
	const unsigned char* pTime = (const unsigned char*) (&time) ;
	const unsigned char* pSpace = (const unsigned char*) (&space);
	sprintf(uuid, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
			pTime[0], pSpace[0], pTime[1], pTime[2], pSpace[1], pTime[3]);
	return uuid;
}


#ifdef KISE_DEBUG
/* 在调试时记录数据的前世今生
 * flag : 0-新增数据  1-因合并而提取 2-因过期而删除
 */
inline void log_cache_history(int flag, const PKG_INFO_DATA *st, const PKG_INFO_DATA *pcm)
{
	// [CACHE_DEL, ITEM: overspeed=? speed=? timercv=? time=? ID=?]
	std::string str;
	if(flag == 0)
		str += "[CACHE_ADD,    ITEM, ";
	else if(flag == 1)
		str += "[CACHE_COMBINE ITEM, ";
	else if(flag ==2)
		str += "[CACHE_TIMEOUT ITEM, ";
	else
		str += "[CACHE_UNKNOWN ITEM, ";
	char temp[1024]; memset(temp, 0, 1024);
	sprintf(temp, "direction = %s, road = %d, overspeed = %d, speed = %d, timercv = %ld, time = %ld", 
	st->direction.c_str(), st->road, st->overspeed?1:0, st->speed, st->timercv, st->time);
	str += std::string(temp);
	if(flag == 1 && pcm != NULL) {
		sprintf(temp, ", combine: overspeed = %d, speed = %d, timercv = %ld, time = %ld",
				pcm->overspeed?1:0, pcm->speed, pcm->timercv, pcm->time);
		str += std::string(temp);
	}
	if( flag == 2 ) {
		time_t t0 = time(NULL);
		sprintf(temp, ", now = %ld, abs = %d", t0, abs( (int)(t0 - st->timercv)) );
		str += std::string(temp);
	}
	fprintf(stderr, "%s\n", str.c_str());
}
#endif  // ifdef KISE_DEBUG

#endif  // ifndef CAR_INFO_CVT_COMMON_H

