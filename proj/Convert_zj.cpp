
/* 与中江协议高度耦合的代码
 */

#include "Convert.h"
#include "car_info_cvt_common.h"
#include "Convert_zj.h"

#include "ProxyTaskDispatcher.h"
#include "../include/pdebug.h"
#include "../include/img_endecode.h"
#include "../puttext/puttext.h"
#include "../config/ConfFile.h"
#include "backup.h"
#include "../Tool/md5.h"
#include "../CodeConvert/CodeConvert.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

static bool create_pkg_info(const char *pData, int dataSize, PKG_INFO_DATA *item);
static int paint_and_pkg(const char *psrc, int size, const char **ret, int *retsize);
static int dealwith_pkg(PKG_INFO_DATA *st, const char **ret, int *retsize);
static int create_result_pkg(const PKG_INFO_DATA *st1, const PKG_INFO_DATA *st2, 
		const char **ret, int *retsize);

/**
 * 中江的转换规则
 * 1. 判断数据包，如果不是车辆信息包则直接转发
 * 2. 如果是车辆信息包但是没有超速则打印图片信息并转发
 * 3. 如果是超速数据，则将超速相机与治安卡口数据关联生成包
 * 4. 考虑备份，应该先回应数据来源一个包ID
 * 5. 特别的，如果是一个车辆信息回应包，一要删除本程序中的备份数据
 *    二要将这个ACK包回应给数据来源
 */
int Convert_zj(const char *srcdata, int size, 
		const char **ret, int *retsize, CSession *psession, struct TCPConn con)
{
	int type = 0;
	type = ntohl( *( (int*)srcdata ) ) & 0x0fffffff;
	PDEBUG("In ZJ convert, type is: %d\n", type);

	if( type != CAR_MESSAGE_TYPE) {
		return Convert_empty(srcdata, size, ret, retsize);
	} else {
		if( size < (int)ZJ_PRO_SIZE ) { 
			PDEBUG("Here we get an ack, it's %s\n", srcdata+8);
			if( ! backup_remove(srcdata + 8) )
				return Convert_empty(srcdata, size, ret, retsize);
			else 
				return 0;
		} else {
			ZJPRO *ptr = (ZJPRO *) srcdata ;
			int speed = 0, limitSpeed = 0;
			speed = ntohl( ptr->speed );
			limitSpeed = ntohl( ptr->limitSpeed );

			int overspeed = ntohl( ptr->flag );
			PDEBUG("In Zj convert, speed = %d, limitSpeed = %d \n",
					speed, limitSpeed);
			// 是超速但是速度大于100，不处理
			// 此时卡口的会在缓存中等待，直到超时后被发送，发送时会修改其速度
			if(overspeed == 2 && (speed >= 100 || speed <= limitSpeed) ) {
				return 0;
			}

			// 是卡口且没超速则直接发送了
			if( overspeed == 0 && speed <= limitSpeed ) {
				return paint_and_pkg(srcdata, size, ret, retsize);
			} else {
				/* 从这里开始数据包被代理接管了，回复ACK给卡口程序 */
				ResponsePackID( ptr->packetID, psession);
				PKG_INFO_DATA st; st.from = con;
				st.speed = speed;
				if( create_pkg_info(srcdata, size, &st) ) {
					if( dealwith_pkg(&st, ret, retsize) > 0 && *ret != NULL) {
						const char *ptemp = *ret;
						paint_and_pkg(*ret, *retsize, ret, retsize);
						delete ptemp;
					}
					return *retsize;
				} 
			}
		}
	}
	return 0;
}

/* 获取配置文件中关于设备编号、地点名称、方向名称的配置 */
int get_text_config( int camera, 
		const char **machineID, const char **addr, const char **direction )
{
	const size_t local_size = 1024;
	char file[local_size];
	static char local_machineID[local_size];
	static char local_addr[local_size];
	static char local_direction[local_size];

	snprintf( file, local_size, "./camera%d.ini", camera + 1);
	PDEBUG("get_text_config, file = %s\n", file);
	CConfFile cfg(file);
	if( !cfg.getValue("Camera","machineID",local_machineID, local_size) )
		return -1;

	if( !cfg.getValue("Camera","addr",local_addr, local_size) ) 
		return -1;

	if( !cfg.getValue("Camera","direct",local_direction, local_size) ) 
		return -1;

	*machineID = local_machineID;
	*addr = local_addr;
	*direction = local_direction;
	return 0;

}

static const char* uuid( const char* str)
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

/* 对网络数据中的图片进行打字，并将打完字的图片封装成新的网络数据包 
 * 打完字的图片最后还加上了MD5码
 * 图片解码、压缩、网络数据包等占用的空间均在函数内部分配 
 */
static int paint_and_pkg(const char *psrc, int size, const char **ret, int *retsize)
{
	static char *rgb_buffer = (char*) malloc(1024 * 1024 * 3 * 3); //300万像素
	static char *jpg_buffer = (char*) malloc(1024 * 1024); // 1M
	static char *pkg_buffer = (char*) malloc(1024 * 1024 * 2); // 2M

	/* 数据包的内容： 包头部 + 图片数据 */
	ZJPRO *pprot = (ZJPRO *) psrc;
	memcpy(pkg_buffer, psrc, ZJ_PRO_SIZE); // 原来的头部

	int offset_src = ZJ_PRO_SIZE;
	int offset_pkg = ZJ_PRO_SIZE;

	int i;

	const char *machineID = NULL;
	const char *addr = NULL; 
	const char *direction = NULL;
	int speed = ntohl(pprot->speed);
	int limitSpeed = ntohl(pprot->limitSpeed);
	// 从对应的配置文件里面读取地点、方向名称
	if( get_text_config( ntohl(pprot->cameraNo), &machineID, &addr, &direction ) ) {
		PDEBUG("Get text config error.\n");
	}

	for(i = 0; i < ntohs(pprot->imageNum); i++ ) {
		
		memcpy(pkg_buffer + offset_pkg, psrc + offset_src, ZJ_IMGI_SIZE);
		ZJIMG *pnew = (ZJIMG *) (pkg_buffer + offset_pkg);
		offset_pkg += ZJ_IMGI_SIZE;

		ZJIMG *pimg = (ZJIMG *) (psrc + offset_src);
		offset_src += ( ZJ_IMGI_SIZE + ntohl(pimg->imageLen) );

		const char *pjpg = &(pimg->imgFirstByte); 

		int width, height;
		img_decode( (unsigned char*)rgb_buffer, &width, &height, 
				(unsigned char*)pjpg, ntohl(pimg->imageLen) );

		static char textout[2048] = {'\0'};
		static wchar_t wcs[1024] = {'\0'};
		snprintf( textout, 2048, "设备编号:%s", machineID?machineID:"未配置" );
		CCodeConvert cvt; 
		cvt.UTF8toUnicode(std::string(textout), wcs);
		puttext(1, wcs, rgb_buffer, width, height);

		snprintf( textout, 2048, "地点:%s 行驶方向:%s", addr?addr:"未配置",
				direction?direction:"未配置" );
		cvt.UTF8toUnicode(std::string(textout), wcs);
		puttext(2, wcs, rgb_buffer, width, height);
		
		snprintf( textout, 2048, "时间:%.2d/%.2d/%.2d  %.2d:%.2d:%.2d.%.3d", 
				ntohs(pimg->year), ntohs(pimg->mon), ntohs(pimg->day),
				ntohs(pimg->hour), ntohs(pimg->min), ntohs(pimg->sec),
				ntohs(pimg->ms) );
		cvt.UTF8toUnicode(std::string(textout), wcs);
		puttext(3, wcs, rgb_buffer, width, height);
		
		snprintf( textout, 2048, "车速:%dkm/h 限速:%dkm/h", speed, limitSpeed );
		cvt.UTF8toUnicode(std::string(textout), wcs);
		puttext(4, wcs, rgb_buffer, width, height);
		
		snprintf( textout, 2048, "防伪码:%s", machineID?uuid(machineID):uuid("未配置") );
		cvt.UTF8toUnicode(std::string(textout), wcs);
		puttext(5, wcs, rgb_buffer, width, height);

		unsigned long jpgsize = 0;
		img_encode( (unsigned char*)rgb_buffer, width, height, 
				(unsigned char*)jpg_buffer, 1024*1024, &jpgsize, 80);

		MD5 md5;
		md5.update( (unsigned char*)jpg_buffer, jpgsize);
		md5.finalize();
		md5.raw_digest( (unsigned char*)jpg_buffer + jpgsize);
		jpgsize += 16;

		memcpy(pkg_buffer + offset_pkg, jpg_buffer, jpgsize);
		offset_pkg += jpgsize;
		pnew->imageLen = htonl(jpgsize);
	}
	ZJPRO *pf = (ZJPRO *) pkg_buffer;
	pf->len = htonl(offset_pkg - 8);

	*ret = pkg_buffer;
	*retsize = offset_pkg;
	return offset_pkg;
}


/* dealwith_pkg: 处理超速的车辆信息
 * 1. 与该函数中已经缓存的数据匹配，如果匹配上则合并新的包
 * 2. 如果匹配不上则将传入的包缓存起来
 * 3. 在函数结束前处理一次超时的数据, 
 *    将一条超时的数据返回给调用层用于发送
 */
static list<PKG_INFO_DATA> cache_pkgs_list; 
static pthread_mutex_t cache_pkgs_lock = PTHREAD_MUTEX_INITIALIZER;
static int dealwith_pkg(PKG_INFO_DATA *st, const char **ret, int *retsize)
{
	pthread_mutex_lock(&cache_pkgs_lock);

	for(list<PKG_INFO_DATA>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		if( cmp2pkg( *st, (*it) ) ) {
			int r = create_result_pkg( &(*it), st, ret, retsize); 
			// CreateCCarAndBackup(ret, retsize);
#ifdef KISE_DEBUG
			LogCacheHistory(1, &(*it), st);
#endif
			delete [] (*it).data; 
			it = cache_pkgs_list.erase(it);
			delete [] st->data;
			pthread_mutex_unlock(&cache_pkgs_lock);
			return r;
		}
	}
	cache_pkgs_list.push_back(*st);
#ifdef KISE_DEBUG
	LogCacheHistory(0, st, NULL);
#endif

	time_t now = time(NULL);
	for(list<PKG_INFO_DATA>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		PDEBUG("now(%ld), timercv(%ld) \n",now, (*it).timercv );
		if( abs( (int)(now - (*it).timercv) ) >= ConfigGet()->control.cache_live )  {
			const char *pkg; int pkgsize;
			/* 这里，特殊的逻辑，对于没匹配上的数据 
			 * 丢弃超速的包， 对于卡口的将速度改到限速以下
			 * 大于100的速度改到限速以下
			 */
			if( ! (*it).overspeed ) {
				ZJPRO *p = (ZJPRO *) (*it).data;
				if( ntohl(p->speed) > ntohl(p->limitSpeed) || ntohl(p->speed) > 100 ) {
					PDEBUG("Here set speed to 39.\n");
					p->speed = htonl(39);
				}

				paint_and_pkg( (*it).data, (*it).len, &pkg, &pkgsize );
				SendOneStOut((*it).from, pkg, pkgsize);
			}

#ifdef KISE_DEBUG
			LogCacheHistory(2, &(*it), NULL);
#endif
			delete [] (*it).data;
			it = cache_pkgs_list.erase(it);
		}
	}

	PDEBUG( "Size of list is: %d \n",cache_pkgs_list.size() );
	pthread_mutex_unlock(&cache_pkgs_lock);
	return 0;
}



/* create_result_pkg: 封装匹配上的车辆信息成一个最终数据包
 * st1, st2 分别是匹配上的超速和治安卡口的数据
 * ret: 封装后的数据，内部动态分配的存储
 * retsize: ret中数据的长度
 * RETURN:  等于retsize
 * TODO 如果开启了备份，则把数据放入备份模块
 */
static int create_result_pkg(
		const PKG_INFO_DATA *st1, const PKG_INFO_DATA *st2, const char **ret, int *retsize)
{
	PDEBUG("CreateResultPkg ... \n");
	const PKG_INFO_DATA* pOverspeed = NULL;
	const PKG_INFO_DATA* pCommon = NULL;
	if( st1->overspeed ) {
		pOverspeed = st1;
		pCommon = st2;
	} else {
		pOverspeed = st2;
		pCommon = st1;
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
	*ret = finalData;
	*retsize = finalLen;
	return *retsize;

}

/**
 * 解析一个数据包，获取中江卡口的相关信息,存储在item中返回
 */
static bool create_pkg_info(const char* pData, int dataSize, PKG_INFO_DATA *item)
{
	ZJPRO *ptr = (ZJPRO *) pData;
	if( ntohs(ptr->imageNum) < 1 ) {
		printf("Error, imageNum < 1 when create_pkg_info\n");
		return false;
	}

	item->len = dataSize;  
	item->data = new char[dataSize];
	memcpy(item->data, pData, dataSize);

	item->direction = string(ptr->direction); 
	item->road = ntohl(ptr->roadNo);
	item->timercv = time(NULL);

	// 0 是治安卡口, 2 是超速卡口
	int type = ntohl(ptr->flag);
	if(type == 0) {
		item->overspeed = false;
	} else if(type == 2) {	
		item->overspeed = true;
	} else {
		printf("Error, create packet info, type unexpect (%d)\n",
				type);
		delete [] item->data;
		return false;
	}

	ZJIMG *pimg = (ZJIMG *) (pData + ZJ_PRO_SIZE);
	short year, month, day, hour, min, sec;
	year = ntohs( pimg->year );
	month = ntohs( pimg->mon );
	day = ntohs( pimg->day );
	hour = ntohs( pimg->hour );
	min = ntohs( pimg->min );
	sec = ntohs( pimg->sec );
	PDEBUG("Create Zj struct, time %d-%d-%d %d:%d:%d\n",
			year, month, day, hour, min, sec);

	struct tm pkgtm;
	pkgtm.tm_sec = sec;
	pkgtm.tm_min = min;
	pkgtm.tm_hour = hour;
	pkgtm.tm_mon = month - 1;  // 0 - 11
	pkgtm.tm_year = year - 1900;
	pkgtm.tm_mday = day;

	item->time = mktime(&pkgtm);

	return true;
}

