
#include "Convert.h"
#include "Convert_sf.h"
#include "ProxyTaskDispatcher.h"
#include "../include/pdebug.h"
#include "../include/timewrap.h"
#include "backup.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

extern CProxyTaskDispatcher *g_dispatcher;

typedef struct {
	int len;  	// data的长度
	char* data; // 数据包的原始内容
	std::string direction; // 方向信息
	int road;       // 车辆信息包中的车道号
	int speed;      // 速度
	time_t timercv; // 接收到该包的时刻
	time_t time;    // 该包中时间字段的值
	bool overspeed; // 是否是超速的数据
	
	struct TCPConn from;
} struct4sf;

/* ResponsePackID: 确认收到数据,回应包ID给发送者
 * data: 数据包的原始数据 psession: 数据来源的会话 */
static inline void ResponsePackID(const char *data, CSession *psession)
{
	struct sf_protocol_data *p = (struct sf_protocol_data*) data;
	char pkg[73];
	memset(pkg,0, 73);
	*( (short*)(pkg + 2) ) = htons(1000);
	*( (int *) (pkg + 4) ) = htonl(64);
	memcpy(pkg+8, p->packetID, 64);
	PDEBUG("We send an ack to source session, it's %s\n", pkg+8);
	psession->Send( pkg, 72);
}

/* SendOneStOut: 将一个数据包发送出去
 * st: 记录了数据和相关信息的结构体 */
static inline void SendOneStOut(const struct4sf &st) 
{
	PDEBUG("Now should send an cached package (time out).\n");
	if(g_dispatcher)
		g_dispatcher->SendResultToOtherSide(st.from, st.data, st.len);
	else
		PDEBUG("Error, send data but g_dispatcher is NULL");
}

static bool Createstruct4sf(const char* pData, int dataSize, struct4sf& item);
static int DealWithSt4SF(struct4sf& st, const char **ret, int *retsize);
static int CreateResultPkg(const struct4sf& st1, const struct4sf& st2, 
		const char **ret, int *retsize);
static bool cmp2st(const struct4sf& st1, const struct4sf& st2);
static void CreateCCarAndBackup(const char *data, int datasize);

/**
 * 什邡的转换规则
 * 1. 判断数据包，如果不是车辆信息包则直接转发
 * 2. 如果是车辆信息包但是没有超速也直接转发 
 * 3. 如果是超速数据，则将超速相机与治安卡口数据关联生成包
 * 4. 考虑备份，应该先回应数据来源一个包ID
 * 5. 特别的，如果是一个车辆信息回应包，一要注意删除本程序中的备份数据
 *    二要可能将这个ACK包回应给数据来源
 *
 * 如果返回大于0且 srcdata != ret，则在内部分配了存放新数据包的空间
 * 在调用者处理完了之后应当释放ret
 */
int Convert_sf(const char *srcdata, int size, 
		const char **ret, int *retsize, CSession *psession, struct TCPConn con)
{
	int type = 0;
	type = ntohl( *( (int*)srcdata ) ) & 0x0fffffff;
	PDEBUG("In SF convert, type is: %d\n", type);
	if( type != 1000 ) {
		return Convert_empty(srcdata, size, ret, retsize);
	} else {
		if( size < (int)sizeof(struct sf_protocol_data) ) { 
			PDEBUG("Here we get an ack, it's %s\n", srcdata+8);
			if( ! backup_remove(srcdata + 8) )
				return Convert_empty(srcdata, size, ret, retsize);
			else 
				return 0;
		} else {
			struct sf_protocol_data *p;
			p = (struct sf_protocol_data *) srcdata;
			int speed = 0, limitSpeed = 0;
			speed = ntohl( p->speed );
			limitSpeed = ntohl( p->limitSpeed );
			PDEBUG("In SF convert, speed = %d, limitSpeed = %d\n",
					speed, limitSpeed);
			if( speed <= limitSpeed - 2 ) { 
				/* 超速相机的speed值必然大于limitSpeed, 
				 * 所以这个-2是针对速度在两边有误差的时候
				 * 将卡口数据缓存的范围扩大一些 */
				return Convert_empty(srcdata, size, ret, retsize);
			} else {
				/* 从这里开始数据包被代理接管了，回复ACK给卡口程序 */
				ResponsePackID(srcdata, psession);
				struct4sf st; st.from = con;
				st.speed = speed;
				if( Createstruct4sf(srcdata, size, st) ) {
					return DealWithSt4SF(st, ret, retsize);
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
inline void LogCacheHistory(int flag, const struct4sf &st, const struct4sf *pcm)
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
	struct sf_protocol_data *pt = (struct sf_protocol_data*) st.data;
	sprintf(temp, "overspeed = %d, speed = %d, timercv = %ld, time = %ld, packetID=%s",
		   st.overspeed?1:0, st.speed, st.timercv, st.time, pt->packetID);
	str += std::string(temp);
	if(flag == 1 && pcm != NULL) {
		struct sf_protocol_data *pp = (struct sf_protocol_data*)(pcm->data);
		sprintf(temp, ", combine: overspeed = %d, speed = %d, timercv = %ld, time = %ld, packetID=%s",
				pcm->overspeed?1:0, pcm->speed, pcm->timercv, pcm->time, pp->packetID);
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

/* DealWithSt4SF : 处理超速的车辆信息
 * 1. 与该函数中已经缓存的数据匹配，如果匹配上则合并新的包
 * 2. 如果匹配不上则将传入的包缓存起来
 * 3. 在函数结束前处理一次超时的数据, 
 *    将一条超时的数据返回给调用层用于发送
 */
static list<struct4sf> cache_pkgs_list; 
static pthread_mutex_t cache_pkgs_lock = PTHREAD_MUTEX_INITIALIZER;
static int DealWithSt4SF(struct4sf& st, const char **ret, int *retsize)
{
	pthread_mutex_lock(&cache_pkgs_lock);

	for(list<struct4sf>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		if( cmp2st( st, (*it) ) ) {
			int r = CreateResultPkg(*it, st, ret, retsize); 
			CreateCCarAndBackup(*ret, *retsize);
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
	for(list<struct4sf>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		PDEBUG("now(%ld), timercv(%ld) \n",now, (*it).timercv );
		if( abs( (int)(now - (*it).timercv) ) >= ConfigGet()->control.cache_live )  {
			CreateCCarAndBackup( (*it).data, (*it).len );
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
static bool cmp2st(const struct4sf& st1, const struct4sf& st2)
{
	if( st1.overspeed != st2.overspeed ) {
		if( st1.direction == st2.direction ) {
			if(st1.road == st2.road) {
				if( abs( (int)(st1.speed - st2.speed) ) < 5 ) {
					if( abs( (int)(st1.time - st2.time) ) < 2 ) {
						return true; 
					}
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
 */
static int CreateResultPkg(
		const struct4sf& st1, const struct4sf& st2, const char **ret, int *retsize)
{
	const struct4sf* pOverspeed = NULL;
	const struct4sf* pCommon = NULL;
	if( st1.overspeed ) {
		pOverspeed = &st1;
		pCommon = &st2;
	} else {
		pOverspeed = &st2;
		pCommon = &st1;
	}
	
	struct sf_protocol_data *po = (struct sf_protocol_data*)(pOverspeed->data);
	struct sf_protocol_data *pc = (struct sf_protocol_data*)(pCommon->data);
	struct sf_img_data *pcominfo = (struct sf_img_data*) ( 
			pCommon->data + sizeof(struct sf_protocol_data) );
	PDEBUG("--------->>>Common: %d-%d-%d : %d-%d-%d -- %d\n",
			ntohs(pcominfo->year), ntohs(pcominfo->month), ntohs(pcominfo->day), 
			ntohs(pcominfo->hour), ntohs(pcominfo->minute),ntohs(pcominfo->second), 
			ntohs(pcominfo->msecond) );
	
	/* 最终数据 = 超速数据基本信息 + 超速图片信息 + 超速图片数据 + 卡口图片信息 + 卡口图片 */
	int commonImgLen = ntohl( pcominfo->length );
	int finalLen = 0;
	finalLen += sizeof(struct sf_protocol_data); // 超速基本信息长度
	int offset = sizeof(struct sf_protocol_data); 
	for( int i = 0; i < ntohs(po->imageNum); i++ ) {
		struct sf_img_data *pi = (struct sf_img_data*)(pOverspeed->data + offset);
		PDEBUG("------>Image[%d] length = %d \n", i, ntohl(pi->length) );
		PDEBUG("---------->>>Overspeed: %d-%d-%d : %d-%d-%d -- %d\n",
				ntohs(pi->year), ntohs(pi->month), ntohs(pi->day), 
				ntohs(pi->hour), ntohs(pi->minute), ntohs(pi->second), ntohs(pi->msecond) );
		finalLen += sizeof(struct sf_img_data); // 超速图像信息长度
		finalLen += ntohl(pi->length); // 超速图像数据长度
		offset = finalLen;
	}
	finalLen += ( sizeof(struct sf_img_data) + commonImgLen ); //卡口图像信息及数据长度
	finalLen += 1; // 视频数目

	PDEBUG("Final sizeof result = %d\n", finalLen);

	char *finalData = new char[finalLen];
	if(finalData == NULL) {
		printf("Memory not enough for result packet.\n");
		return 0;
	}

	PDEBUG("sizeof(sf_pro) = %u, sizeof(sf_img) = %u, offset = %u\n",
			sizeof(struct sf_protocol_data), sizeof(sf_img_data), offset );
	memcpy(finalData, pOverspeed->data, offset);

	/* 覆盖车牌号码 */
	struct sf_protocol_data *pf = (struct sf_protocol_data*)(finalData);
	memcpy( pf->plate, pc->plate, sizeof(pf->plate) );
	PDEBUG("plate is: %s \n", pf->plate);
	
	memcpy(finalData + offset, (pCommon->data + sizeof(struct sf_protocol_data)), 
			sizeof(struct sf_img_data) + commonImgLen);

	/* 视频数目 */
	*( (char*)(finalData + finalLen - 1) ) = 0;

#ifdef KISE_DEBUG
	struct sf_img_data *veri = (struct sf_img_data*)(finalData + offset) ;
	PDEBUG("---------->>>Final: %d-%d-%d : %d-%d-%d -- %d\n",
			ntohs(veri->year), ntohs(veri->month), ntohs(veri->day), 
			ntohs(veri->hour), ntohs(veri->minute), ntohs(veri->second), ntohs(veri->msecond) );
#endif

	/* 最后将图片数目加1 */
	pf->imageNum = htons( ntohs(pf->imageNum) + 1 );

	*( (int*)(finalData + 4) ) = htonl(finalLen -8); 
	
	*ret = finalData;
	*retsize = finalLen;
	return *retsize;

}

/* 解析一个数据包，获取卡口的相关信息,存储在item中返回
 * 将原始数据拷贝了出来,在堆上分配了空间
 * 同时记录当前的时间作为收到数据包的时间
 */
static bool Createstruct4sf(const char* pData, int dataSize, struct4sf& item)
{
	if( dataSize < (int)(sizeof(struct sf_protocol_data) + sizeof(struct sf_img_data) ) ) {
		printf("Data can't convert ... len (%d) \n", dataSize);
		return false;
	}

	item.len = dataSize;  
	item.data = new char[dataSize];
	memcpy(item.data, pData, dataSize);

	struct sf_protocol_data *p = (struct sf_protocol_data *)pData;

	item.direction = string( p->direction ); 
	item.road = ntohl( p->road );
	item.timercv = time(NULL);

	int imgnum = ntohs( p->imageNum );
	if( imgnum == 1 ) {
		item.overspeed = false;
	} else if( imgnum == 2) {	
		item.overspeed = true;
	} else {
		printf("Error, Createstruct4sf, image number unexpect (%d)\n", imgnum);
		delete [] item.data;
		return false;
	}

	/* 这里获取第一张图片的信息 */
	struct sf_img_data *imginfo = 
		(struct sf_img_data *)( pData + sizeof(struct sf_protocol_data) );

	struct tm pkgtm;
	pkgtm.tm_sec = ntohs(imginfo->second);
	pkgtm.tm_min = ntohs(imginfo->minute);
	pkgtm.tm_hour = ntohs(imginfo->hour);
	pkgtm.tm_mon = ntohs(imginfo->month) - 1;  // 0 - 11
	pkgtm.tm_year = ntohs(imginfo->year) - 1900;
	pkgtm.tm_mday = ntohs(imginfo->day);

	PDEBUG("Create SF struct, time %d-%d-%d %d:%d:%d\n",
			ntohs(imginfo->year), ntohs(imginfo->month), ntohs(imginfo->day), 
			ntohs(imginfo->hour), ntohs(imginfo->minute), ntohs(imginfo->second) );

	item.time = mktime(&pkgtm);

	return true;
}

/* 将网络数据包封装成CCar并进行备份 
 * 由于备份模块new并复制了CCar所以本类结束时将释放自己new的Car对象
 */
static void CreateCCarAndBackup(const char* data, int datasize) {
	if( !ConfigGet()->control.enable_backup )
		return;
#ifdef CONFIG_BACKUP
	CCar *pcar = new CCar;
	struct sf_protocol_data *p = (struct sf_protocol_data *)data;

	pcar->packetID = std::string(p->packetID);
	pcar->machineID = "";
	pcar->monitorID = std::string(p->monitorID);
	pcar->direction = std::string(p->direction);
	pcar->cameraNo = ntohl(p->camera);
	pcar->virtualRoadNo = 0;
	pcar->roadNo = ntohl(p->road);
	memcpy(pcar->plate, p->plate, sizeof(pcar->plate) );
	pcar->plateX = ntohl(p->plateX);
	pcar->plateY = ntohl(p->plateY);
	pcar->plateWidth = ntohl(p->plateWidth);
	pcar->plateHeight = ntohl(p->plateHeight);
	pcar->confidence = ntohl(p->confidence);
	pcar->plateColor = ntohs(p->plateColor);
	pcar->style = ntohs(p->style);
	pcar->color = ntohs(p->color);
	pcar->detect = p->detect;
	pcar->isViolation = p->isViolation;

	pcar->tmRedBegin = get_timeval_by_date(
			ntohs(p->year), ntohs(p->month), ntohs(p->day),
			ntohs(p->hour), ntohs(p->minute), ntohs(p->second),
			ntohs(p->msecond)
			);

	memcpy(pcar->vioType, p->vioType, sizeof(pcar->vioType) );
	pcar->speed = ntohl(p->speed);
	pcar->limitSpeed = ntohl(p->limitSpeed);
	pcar->imageNum = ntohs(p->imageNum);

	int offset = sizeof(struct sf_protocol_data);
	for(int i = 0; i < pcar->imageNum; i++) {
		struct sf_img_data *pi = (struct sf_img_data *)(data + offset);
		struct tm cartm;
		cartm.tm_sec = ntohs(pi->second);
		cartm.tm_min = ntohs(pi->minute);
		cartm.tm_hour = ntohs(pi->hour);
		cartm.tm_mon = ntohs(pi->month) - 1;  // 0 - 11
		cartm.tm_year = ntohs(pi->year) - 1900;
		cartm.tm_mday = ntohs(pi->day);
		pcar->time[i].tv_sec = mktime( &cartm );
		pcar->time[i].tv_usec = ntohs( pi->msecond ) * 1000;
		pcar->imageLen[i] = ntohl(pi->length);

		offset += sizeof(struct sf_img_data);

		pcar->image[i] = new char[ pcar->imageLen[i] ];
		memcpy( pcar->image[i], data + offset, pcar->imageLen[i] );
		
		offset += ntohl(pi->length);
	}

	pcar->videoNum = 0;

	backup_add(pcar);

	delete pcar;
#endif
}

/* 备份的回调函数，在这里将数据发送出去  
 * 发送到配置文件中指定的地方
 */
void backup_callback(CCar *pcar) 
{
	PDEBUG(" ~~~~~> In backup call back.\n");
	int length = sizeof(struct sf_protocol_data);
	for(int i = 0; i < pcar->imageNum; i++) {
		length += sizeof(struct sf_img_data);
		length += pcar->imageLen[i];
	}
	length += 1;
	char *ret = new char[length];
	struct sf_protocol_data *pret = (struct sf_protocol_data*)ret;

	pret->type = htonl(1000);
	pret->length = htonl(length - 8);
	strncpy( pret->monitorID, pcar->monitorID.c_str(), sizeof(pret->monitorID) );
	strncpy( pret->direction, pcar->direction.c_str(), sizeof(pret->direction) );
	pret->camera = htonl(pcar->cameraNo);
	pret->road = htonl(pcar->roadNo);
	memcpy(pret->plate, pcar->plate, sizeof(pret->plate));
	pret->plateX = htonl(pcar->plateX);
	pret->plateY = htonl(pcar->plateY);
	pret->plateWidth = htonl(pcar->plateWidth);
	pret->plateHeight = htonl(pcar->plateHeight);
	pret->confidence = htonl(pcar->confidence);;
	pret->plateColor = htons(pcar->plateColor);
	pret->style = htons(pcar->style);
	pret->color = htons(pcar->color);
	pret->detect = pcar->detect;
	pret->isViolation = pcar->isViolation;

	get_date_by_timeval_net( pcar->tmRedBegin, &(pret->year), &(pret->month),
			&(pret->day), &(pret->hour), &(pret->minute), &(pret->second),
			&(pret->msecond) );

	memcpy(pret->vioType, pcar->vioType, sizeof(pret->vioType) );
	pret->speed = htonl(pcar->speed);
	pret->limitSpeed = htonl(pcar->limitSpeed);
	strncpy( pret->packetID, pcar->packetID.c_str(), sizeof(pret->packetID) );
	pret->imageNum = htons(pcar->imageNum);

	int offset = sizeof(struct sf_protocol_data);
	for(int j = 0; j < pcar->imageNum; j++) {
		struct sf_img_data *pi = (struct sf_img_data*) (ret + offset) ;
		struct tm *imgtm = gmtime( &(pcar->time[j].tv_sec) );
		pi->year = (short) htons(imgtm->tm_year + 1900);
		pi->month = (short) htons(imgtm->tm_mon + 1);
		pi->day = (short) htons(imgtm->tm_mday);
		pi->hour = (short) htons(imgtm->tm_hour);
		pi->minute = (short) htons(imgtm->tm_min);
		pi->second = (short) htons(imgtm->tm_sec);
		pi->msecond = (short) ( htons( (short)( (pcar->time[j].tv_usec) / 1000) ) );
		pi->length = htonl( pcar->imageLen[j] );

		offset += sizeof(struct sf_img_data);

		memcpy( ret + offset, pcar->image[j], pcar->imageLen[j] );
		
		offset += pcar->imageLen[j];
	}

	*( (char *)(ret - 1) ) = 0;

	g_dispatcher->SendResultToOtherSide(
			ConfigGet()->backup->backup_group_id, 
			ConfigGet()->backup->backup_side, 
			ret, length);

	delete [] ret;
}

