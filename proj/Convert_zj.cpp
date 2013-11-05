
#include "Convert.h"
#include "ProxyTaskDispatcher.h"
#include "../include/pdebug.h"
#include "backup.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

extern CProxyTaskDispatcher *g_dispatcher;

typedef struct {
	int len;  	// data的长度
	char* data; // 数据包的原始内容
	std::string direction; // 方向信息
	int speed;  
	time_t timercv; // 接收到该包的时刻
	time_t time;    // 该包中时间字段的值
	bool overspeed; // 是否是超速的数据
	
	struct TCPConn from;
} DataSt4ZJ;

inline void ResponsePackID(const char *data, CSession *psession)
{
	char pkg[73];
	memset(pkg,0, 73);
	*( (short*)(pkg + 2) ) = htons(1000);
	*( (int *) (pkg + 4) ) = htonl(64);
	memcpy(pkg+8, data+112, 64);
	PDEBUG("We send an ack to source session, it's %s\n", pkg+8);
	psession->Send( pkg, 72);
}

inline void SendOneStOut(const DataSt4ZJ &st) 
{
	PDEBUG("Now should send an cached package (time out).\n");
	if(g_dispatcher)
		g_dispatcher->SendResultToOtherSide(st.from, st.data, st.len);
	else
		PDEBUG("Error, send data but g_dispatcher is NULL");
}

static bool CreateDataSt4ZJ(const char* pData, int dataSize, DataSt4ZJ& item);
static int DealWithSt4ZJ(DataSt4ZJ& st, const char *&ret, int &retsize);
static int CreateResultPkg(const DataSt4ZJ& st1, const DataSt4ZJ& st2, 
		const char *&ret, int &retsize);
static bool cmp2st(const DataSt4ZJ& st1, const DataSt4ZJ& st2);

/**
 * 中江的转换规则
 * TODO ！！！硬编码
 * 1. 判断数据包，如果不是车辆信息包则直接转发
 * 2. 如果是车辆信息包但是没有超速也直接转发 
 * 3. 如果是超速数据，则将超速相机与治安卡口数据关联生成包
 * 4. 考虑备份，应该先回应数据来源一个包ID
 * 5. 特别的，如果是一个车辆信息回应包，一要删除本程序中的备份数据
 *    二要将这个ACK包回应给数据来源
 */
int Convert_zj(const char *srcdata, int size, 
		const char *&ret, int &retsize, CSession *psession, struct TCPConn con)
{
	int type = 0;
	type = ntohl( *( (int*)srcdata ) ) & 0x0fffffff;
	PDEBUG("In ZJ convert, type is: %d\n", type);
	if( type != 1000 ) {
		return Convert_empty(srcdata, size, ret, retsize);
	} else {
		if( size < 176 ) { 
			PDEBUG("Here we get an ack, it's %s\n", srcdata+8);
			if( ! backup_remove(srcdata + 8) )
				return Convert_empty(srcdata, size, ret, retsize);
			else 
				return 0;
		} else {
			int speed = 0, limitSpeed = 0;
			speed = ntohl( *( (int*)(srcdata + 104) ) );
			limitSpeed = ntohl( *( (int*)(srcdata + 108) ) );
			PDEBUG("In Zj convert, speed = %d, limitSpeed = %d \n",
					speed, limitSpeed);
			if( speed <= limitSpeed ) {
				return Convert_empty(srcdata, size, ret, retsize);
			} else {
				/* 从这里开始数据包被代理接管了，回复ACK给卡口程序 */
				ResponsePackID(srcdata, psession);
				DataSt4ZJ st; st.from = con;
				st.speed = speed;
				if( CreateDataSt4ZJ(srcdata, size, st) ) {
					return DealWithSt4ZJ(st, ret, retsize);
				} 
			}
		}
	}
	return 0;
}

#ifdef KISE_DEBUG
/* LogCacheHistory 在调试时记录数据的前世今生
 * flag : 0-新增数据  1-因合并而提取 2-因过期而删除
 */
inline void LogCacheHistory(int flag, const DataSt4ZJ &st, const DataSt4ZJ *pcm)
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
	sprintf(temp, "overspeed = %d, speed = %d, timercv = %ld, time = %ld, packID=%s",
		   st.overspeed?1:0, st.speed, st.timercv, st.time, st.data+112);
	str += std::string(temp);
	if(flag == 1 && pcm != NULL) {
		sprintf(temp, ", combine: overspeed = %d, speed = %d, timercv = %ld, time = %ld, packID=%s",
				pcm->overspeed?1:0, pcm->speed, pcm->timercv, pcm->time, pcm->data+112);
		str += std::string(temp);
	}
	if( flag == 2 ) {
		time_t t0 = time(NULL);
		sprintf(temp, ", now = %ld, abs = %d", t0, abs( (int)(t0 - st.timercv)) );
		str += std::string(temp);
	}
	fprintf(stderr, "%s\n", str.c_str());
}
#endif

/* DealWithSt4ZJ : 处理超速的车辆信息
 * 1. 与该函数中已经缓存的数据匹配，如果匹配上则合并新的包
 * 2. 如果匹配不上则将传入的包缓存起来
 * 3. 在函数结束前处理一次超时的数据, 
 *    将一条超时的数据返回给调用层用于发送
 */
static list<DataSt4ZJ> cache_pkgs_list; 
static pthread_mutex_t cache_pkgs_lock = PTHREAD_MUTEX_INITIALIZER;
static int DealWithSt4ZJ(DataSt4ZJ& st, const char *&ret, int &retsize)
{
	pthread_mutex_lock(&cache_pkgs_lock);

	for(list<DataSt4ZJ>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		if( cmp2st( st, (*it) ) ) {
			int r = CreateResultPkg(*it, st, ret, retsize); 
			// CreateCCarAndBackup(ret, retsize);
#ifdef KISE_DEBUG
			LogCacheHistory(1, (*it), &st);
#endif
			delete [] (*it).data; 
			it = cache_pkgs_list.erase(it);
			delete [] st.data;
			pthread_mutex_unlock(&cache_pkgs_lock);
			return r;
		}
	}
	cache_pkgs_list.push_back(st);
#ifdef KISE_DEBUG
	LogCacheHistory(0, st, NULL);
#endif

	time_t now = time(NULL);
	for(list<DataSt4ZJ>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		PDEBUG("now(%ld), timercv(%ld) \n",now, (*it).timercv );
		if( abs( (int)(now - (*it).timercv) ) >= ConfigGet()->control.cache_live )  {
			// CreateCCarAndBackup( (*it).data, (*it).len );
			SendOneStOut(*it);
#ifdef KISE_DEBUG
			LogCacheHistory(2, (*it), NULL);
#endif
			delete [] (*it).data;
			it = cache_pkgs_list.erase(it);
		}
	}

	PDEBUG( "Size of list is: %d \n",cache_pkgs_list.size() );
	pthread_mutex_unlock(&cache_pkgs_lock);
	return 0;
}


/* cmp2st 比较两个结构提是否匹配
 * 匹配条件:
 *   1. 一个是超速一个是治安卡口 
 *   2. 方向相同， 速度与时间差小
 */
static bool cmp2st(const DataSt4ZJ& st1, const DataSt4ZJ& st2)
{
	if( st1.overspeed != st2.overspeed ) {
		if( st1.direction == st2.direction ) {
			if( abs( (int)(st1.speed - st2.speed) ) < 5 ) {
				if( abs( (int)(st1.time - st2.time) ) < 2 ) {
					return true; 
				}
			}
		}
	}
	return false;
}

/* CreateResultPkg: 封装匹配上的车辆信息成一个最终数据包
 * st1, st2 分别是匹配上的超速和治安卡口的数据
 * ret: 封装后的数据，内部动态分配的存储
 * retsize: ret中数据的长度
 * RETURN:  等于retsize
 * TODO 如果开启了备份，则把数据放入备份模块
 */
static int CreateResultPkg(
		const DataSt4ZJ& st1, const DataSt4ZJ& st2, const char *&ret, int &retsize)
{
	PDEBUG("CreateResultPkg ... \n");
	const DataSt4ZJ* pOverspeed = NULL;
	const DataSt4ZJ* pCommon = NULL;
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
	char *finalData = new char[finalLen];
	
   	// 前半部分跟超速数据一样
	memcpy(finalData, pOverspeed->data, pOverspeed->len);// 超速数据包长度
	// 覆盖车牌号码
	memcpy(finalData + 60, pCommon->data + 60, 16);
	PDEBUG("plate is: %s \n", pCommon->data + 60);
	
	memcpy(finalData + pOverspeed->len, 
			pCommon->data + 178, 
			18 + commonImgLen     // 图片相关数据（时间）及图片长度
		  ); // 拷贝治安卡口 imgNum 之后的数据

	*( (short*)(finalData + 176) ) = htons(  ntohs( *(short*)(finalData + 176) ) + 1); 
	*( (int*)(finalData + 4) ) = htonl( finalLen - 8 );  // 修改协议包中的L字段
	*( (int*)(finalData + 8) ) = htonl(1); // 卡口加超速数据
	ret = finalData;
	retsize = finalLen;
	return retsize;

}

/**
 * 解析一个数据包，获取中江卡口的相关信息,存储在item中返回
 */
static bool CreateDataSt4ZJ(const char* pData, int dataSize, DataSt4ZJ& item)
{
	if(dataSize < 190 ) {
		printf("Data can't convert ... len (%d) \n", dataSize);
		return false;
	}

	item.len = dataSize;  
	item.data = new char[dataSize];
	memcpy(item.data, pData, dataSize);

	char direct[8] = {0};
	memcpy(direct, pData + 44, 8);
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
	PDEBUG("Create Zj struct, time %d-%d-%d %d:%d:%d\n",
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

