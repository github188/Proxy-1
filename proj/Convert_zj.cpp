
#include "Convert.h"
#include "../include/pdebug.h"

#include<arpa/inet.h>
#include<stdlib.h>

typedef struct {
	int len;  	// data的长度
	char* data; // 数据包的原始内容
	std::string direction; // 方向信息
	int speed;  
	time_t timercv; // 接收到该包的时刻
	time_t time;    // 该包中时间字段的值
	bool overspeed; // 是否是超速的数据
} DataSt4ZJ;


static bool CreateDataSt4ZJ(const char* pData, int dataSize, DataSt4ZJ& item);
static int DealWithSt4ZJ(DataSt4ZJ& st, const char *&ret, int &retsize, CSession *psession);
static int CreateResultPkg(
		DataSt4ZJ& st1, DataSt4ZJ& st2, const char *&ret, int &retsize);
static bool cmp2st(const DataSt4ZJ& st1, const DataSt4ZJ& st2);

/**
 * 中江的转换规则
 * TODO ！！！好多地方写死的代码！！！
 * 1. 判断数据包，如果不是车辆信息包则直接转发
 * 2. 如果是车辆信息包但是没有超速也直接转发 
 * 3. 如果是超速数据，则将超速相机与治安卡口数据关联生成包
 */
int Convert_zj(const char *srcdata, int size, 
		const char *&ret, int &retsize, CSession *psession)
{
	int type = 0;
	type = ntohl( *( (int*)srcdata ) ) & 0x0fffffff;
	PDEBUG("In ZJ convert, type is: %d\n", type);
	if( type != 1000 ) {
		return Convert_empty(srcdata, size, ret, retsize);
	} else {
		if( size< 112 ) { 
			printf("Error! dataSize is too small (%d) \n", size);
			return 0;
		}
		int speed = 0, limitSpeed = 0;
		speed = ntohl( *( (int*)(srcdata + 104) ) );
		limitSpeed = ntohl( *( (int*)(srcdata + 108) ) );
		PDEBUG("In Zj convert, speed = %d, limitSpeed = %d \n",
				speed, limitSpeed);
		if( speed <= limitSpeed ) {
			return Convert_empty(srcdata, size, ret, retsize);
		} else {
			DataSt4ZJ st;
			st.speed = speed;
			if( CreateDataSt4ZJ(srcdata, size, st) )
				return DealWithSt4ZJ(st, ret, retsize, psession);
		}
	}
	return 0;
}

/* DealWithSt4ZJ : 处理超速的车辆信息
 * 1. 与该函数中已经缓存的数据匹配，如果匹配上则合并新的包
 * 2. 如果匹配不上则将传入的包缓存起来
 * 3. 在函数结束前处理一次超时的数据, 
 *    将一条超时的数据返回给调用层用于发送
 */
static int DealWithSt4ZJ(DataSt4ZJ& st, const char *&ret, int &retsize, CSession *psession)
{
	static list<DataSt4ZJ> pkgs; 
	bool find = false; int r = 0;

	for(list<DataSt4ZJ>::iterator it = pkgs.begin(); 
			it != pkgs.end(); it++){
		if( cmp2st(st,(*it)) ) {
			r = CreateResultPkg(*it, st, ret, retsize);
			find = true;
			delete [] (*it).data; // 删除容器中匹配上的数据
			pkgs.erase(it);
			break;
		}
	}

	if(find)  
		delete [] st.data;
	else 
		pkgs.push_back(st);
	
	time_t now = time(NULL);
	for(list<DataSt4ZJ>::iterator it = pkgs.begin(); 
			it != pkgs.end(); it++)
	{
		PDEBUG("now(%ld), timercv(%ld) \n",now, (*it).timercv );
		if( abs( now - (*it).timercv ) > 5 ) 
		{
			printf("Drop one packge .....\n");
			delete [] (*it).data;
			pkgs.erase(it);
			it = pkgs.begin(); 
		}
	}

	PDEBUG( "Size of list is: %d \n",pkgs.size() );
	return r;
}

/* cmp2st 比较两个结构提是否匹配
 * 匹配条件:
 *   1. 一个是超速一个是治安卡口 
 *   2. 方向相同， 速度与时间差小
 */
static bool cmp2st(const DataSt4ZJ& st1, const DataSt4ZJ& st2)
{
	if( st1.overspeed != st2.overspeed )
		if( st1.direction == st2.direction )
			if( abs(st1.speed - st2.speed) < 5 )
				if( abs( st1.time - st2.time ) < 2 )
					return true; 
	return false;
}

/* CreateResultPkg: 封装匹配上的车辆信息成一个最终数据包
 * st1, st2 分别是匹配上的超速和治安卡口的数据
 * ret: 封装后的数据，内部动态分配的存储
 * retsize: ret中数据的长度
 * RETURN:  等于retsize
 */
static char *finalData = NULL;
static int CreateResultPkg(
		DataSt4ZJ& st1, DataSt4ZJ& st2, const char *&ret, int &retsize)
{
	PDEBUG("CreateResultPkg ... \n");
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
	if(finalData) delete [] finalData;
	finalData = new char[finalLen];
	
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

