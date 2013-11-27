
#include "Convert.h"
#include "car_info_cvt_common.h"
#include "Convert_sf.h"
#include "backup.h"

#include "ProxyTaskDispatcher.h"
#include "../include/pdebug.h"

#ifdef CONFIG_PUTTEXT

#include "../include/img_endecode.h"
#include "../puttext/puttext.h"
#include "../config/ConfFile.h"
#include "../Tool/md5.h"
#include "../CodeConvert/CodeConvert.h"

#endif

#include "../include/timewrap.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

static bool create_pkg_info_sf(const char* pdata, int size, PKG_INFO_DATA *item);
static int dealwith_pkg_sf(PKG_INFO_DATA *st, const char **ret, int *retsize);
static int create_result_pkg_sf(const PKG_INFO_DATA *st1, const PKG_INFO_DATA *st2, 
		const char **ret, int *retsize);
static int paint_and_pkg(const char *psrc, int size, const char **ret, int *retsize);
static void CreateCCarAndBackup(const char *data, int datasize);

/**
 * 什邡的转换规则
 * 1. 判断数据包，如果不是车辆信息包则直接转发
 * 2. 如果是车辆信息包但是没有超速也直接转发 
 * 3. 如果是超速数据，则将超速相机与治安卡口数据关联生成包
 * 4. 考虑备份，应该先回应数据来源一个包ID
 * 5. 特别的，如果是一个车辆信息回应包，一要注意删除本程序中的备份数据
 *    二要可能将这个ACK包回应给数据来源
 */
int Convert_sf(const char *srcdata, int size, 
		const char **ret, int *retsize, CSession *psession, struct TCPConn con)
{
	int type = 0;
	type = ntohl( *( (int*)srcdata ) ) & 0x0fffffff;
	PDEBUG("In SF convert, type is: %d\n", type);

	if( type != CAR_MESSAGE_TYPE ) {
		return Convert_empty(srcdata, size, ret, retsize);
	} else {
		/* 比协议结构体小，类型又是车辆信息的是ACK数据  */
		if( size < (int) SF_PRO_SIZE ) { 
			PDEBUG("Here we get an ack, it's %s\n", srcdata+8);
			if( ! backup_remove(srcdata + 8) )
				return Convert_empty(srcdata, size, ret, retsize);
			else 
				return 0;
		} else {
			SFPRO *p = (SFPRO *) srcdata;
			int speed = 0, limitSpeed = 0;
			speed = ntohl( p->speed );
			limitSpeed = ntohl( p->limitSpeed );

			int overspeed = ntohs( p->imageNum );
			PDEBUG("In SF convert, speed = %d, limitSpeed = %d\n",
					speed, limitSpeed);
			if(overspeed == 2 && (speed >= 150 || speed <= limitSpeed) ) {
				response_packet_id( p->packetID, psession );
				return 0;
			}

			if( overspeed == 1 && speed <= limitSpeed ) { 
				/* 超速相机的speed值必然大于limitSpeed, 
				 * 所以这个-2是针对速度在两边有误差的时候
				 * 将卡口数据缓存的范围扩大一些 */
				//PAINT return paint_and_pkg(srcdata, size, ret, retsize);
				return Convert_empty(srcdata, size, ret, retsize);
			} else {
				/* 从这里开始数据包被代理接管了，回复ACK给卡口程序 */
				response_packet_id( p->packetID, psession);
				PKG_INFO_DATA st; st.from = con;
				st.speed = speed;
				if( create_pkg_info_sf(srcdata, size, &st) ) {
					/* PAINT
					if( dealwith_pkg_sf(&st, ret, retsize) > 0 && *ret != NULL ) {
						const char *ptemp = *ret;
						paint_and_pkg(*ret, *retsize, ret, retsize);
						delete ptemp;
					}
					return *retsize;
					*/
					return dealwith_pkg_sf(&st, ret, retsize);
				} 
			}

		}
	}
	return 0;
}
/*****打字需要的函数********/
static int get_text_config(int camera, const char **machineID, const char **addr, const char **direction )
{
#ifdef CONFIG_PUTTEXT
	const size_t local_size = 1024;
	char file[local_size];
	static char local_machineID[local_size];
	static char local_addr[local_size];
	static char local_direction[local_size];

	snprintf( file, local_size, "./common%d.ini", camera + 1);
	PDEBUG("get_text_config, file = %s\n", file);
	CConfFile cfg(file);
	if( !cfg.getValue("Info","machineID",local_machineID, local_size) )
		return -1;

	if( !cfg.getValue("Info","monitorName",local_addr, local_size) ) 
		return -1;

	if( !cfg.getValue("Info","directionName",local_direction, local_size) ) 
		return -1;

	*machineID = local_machineID;
	*addr = local_addr;
	*direction = local_direction;
	return 0;
#else
	return -1;
#endif
}

static int paint_and_pkg(const char *psrc, int size, const char **ret, int *retsize)
{
#ifdef CONFIG_PUTTEXT
	static char *rgb_buffer = (char*) malloc(1024 * 1024 * 3 * 3); //300万像素
	static char *jpg_buffer = (char*) malloc(1024 * 1024); // 1M
	static char *pkg_buffer = (char*) malloc(1024 * 1024 * 2); // 2M

	/* 数据包的内容： 包头部 + 图片数据 */
	SFPRO *pprot = (SFPRO *) psrc;
	memcpy(pkg_buffer, psrc, SF_PRO_SIZE); // 原来的头部

	int offset_src = SF_PRO_SIZE;
	int offset_pkg = SF_PRO_SIZE;

	int i;

	const char *machineID = NULL;
	const char *addr = NULL; 
	const char *direction = NULL;
	int speed = ntohl(pprot->speed);
	int limitSpeed = ntohl(pprot->limitSpeed);
	// 从对应的配置文件里面读取地点、方向名称
	if( get_text_config( ntohl(pprot->camera), &machineID, &addr, &direction ) ) {
		PDEBUG("Get text config error.\n");
	}

	for(i = 0; i < ntohs(pprot->imageNum); i++ ) {

		memcpy(pkg_buffer + offset_pkg, psrc + offset_src, SF_IMGI_SIZE);
		SFIMG *pnew = (SFIMG *) (pkg_buffer + offset_pkg);
		offset_pkg += SF_IMGI_SIZE;

		SFIMG *pimg = (SFIMG *) (psrc + offset_src);
		offset_src += ( SF_IMGI_SIZE + ntohl(pimg->imageLen) );

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

	offset_pkg += 1; // 视频个数
	*(pkg_buffer + offset_pkg) = 0;

	SFPRO *pf = (SFPRO *) pkg_buffer;
	pf->length = htonl(offset_pkg - 8);

	*ret = pkg_buffer;
	*retsize = offset_pkg;
	return offset_pkg;
#else 
	return 0;
#endif
}

/* dealwith_pkg_sf: 处理超速的车辆信息
 * 1. 与该函数中已经缓存的数据匹配，如果匹配上则合并新的包
 * 2. 如果匹配不上则将传入的包缓存起来
 * 3. 在函数结束前处理一次超时的数据, 
 *    将一条超时的数据返回给调用层用于发送
 */
static list<PKG_INFO_DATA> cache_pkgs_list; 
static pthread_mutex_t cache_pkgs_lock = PTHREAD_MUTEX_INITIALIZER;
static int dealwith_pkg_sf(PKG_INFO_DATA *st, const char **ret, int *retsize)
{
	pthread_mutex_lock(&cache_pkgs_lock);

	for(list<PKG_INFO_DATA>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		if( cmp2pkg( *st, (*it) ) ) {
			int r = create_result_pkg_sf( &(*it), st, ret, retsize); 
			CreateCCarAndBackup(*ret, *retsize);
#ifdef KISE_DEBUG
			log_cache_history(1, &(*it), st);
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
	log_cache_history(0, st, NULL);
#endif

	time_t now = time(NULL);
	for(list<PKG_INFO_DATA>::iterator it = cache_pkgs_list.begin(); 
			it != cache_pkgs_list.end(); it++) {
		PDEBUG("now(%ld), timercv(%ld) \n",now, (*it).timercv );
		if( abs( (int)(now - (*it).timercv) ) >= ConfigGet()->control.cache_live )  {
			CreateCCarAndBackup( (*it).data, (*it).len );
			//PAINT const char *pkg; int pkgsize;

			if(! (*it).overspeed ) {
				/*
				SFPRO *p = (SFPRO *) (*it).data;
				if( ntohl(p->speed) > ntohl(p->limitSpeed) ) {
					PDEBUG("Here set speed to 39.\n");
					p->speed = htonl(39);
				}
				*/
				//PAINT paint_and_pkg( (*it).data, (*it).len, &pkg, &pkgsize );
				send_one_pkg( (*it).from, (*it).data, (*it).len );

			} else {
				// Log drop overspeed data
			}
#ifdef KISE_DEBUG
			log_cache_history(2, &(*it), NULL);
#endif
			delete [] (*it).data;
			it = cache_pkgs_list.erase(it);
		}
	}

	PDEBUG( "Size of list is: %d \n",cache_pkgs_list.size() );
	pthread_mutex_unlock(&cache_pkgs_lock);
	return 0;
}

/* create_result_pkg_sf: 封装匹配上的车辆信息成一个最终数据包
 * st1, st2 分别是匹配上的超速和治安卡口的数据
 * ret: 封装后的数据，内部动态分配的存储
 * retsize: ret中数据的长度
 * RETURN:  等于retsize
 */
static int create_result_pkg_sf(
		const PKG_INFO_DATA *st1, const PKG_INFO_DATA *st2, const char **ret, int *retsize)
{
	static const int finalDataSize = 1024*1024*2;
	static char *finalData = (char *) malloc(finalDataSize);
	int finalLen = 0;

	const PKG_INFO_DATA* pOverspeed = NULL;
	const PKG_INFO_DATA* pCommon = NULL;
	if( st1->overspeed ) {
		pOverspeed = st1;
		pCommon = st2;
	} else {
		pOverspeed = st2;
		pCommon = st1;
	}
	
	SFPRO *po = (SFPRO *)(pOverspeed->data);
	SFPRO *pc = (SFPRO *)(pCommon->data);
	SFIMG *pcominfo = (SFIMG *) (pCommon->data + SF_PRO_SIZE);
	PDEBUG("--------->>>Common: %d-%d-%d : %d-%d-%d -- %d\n",
			ntohs(pcominfo->year), ntohs(pcominfo->mon), ntohs(pcominfo->day), 
			ntohs(pcominfo->hour), ntohs(pcominfo->min),ntohs(pcominfo->sec), 
			ntohs(pcominfo->ms) );
	
	/* 最终数据 = 超速数据基本信息 + 超速图片信息 + 超速图片数据 + 卡口图片信息 + 卡口图片 */
	int commonImgLen = ntohl( pcominfo->imageLen );
	finalLen += SF_PRO_SIZE; // 超速基本信息长度
	int offset = SF_PRO_SIZE; 
	for( int i = 0; i < ntohs(po->imageNum); i++ ) {
		SFIMG *pi = (SFIMG *)(pOverspeed->data + offset);
		PDEBUG("------>Image[%d] length = %d \n", i, ntohl(pi->imageLen) );
		PDEBUG("---------->>>Overspeed: %d-%d-%d : %d-%d-%d -- %d\n",
				ntohs(pi->year), ntohs(pi->mon), ntohs(pi->day), 
				ntohs(pi->hour), ntohs(pi->min), ntohs(pi->sec), ntohs(pi->ms) );
		finalLen += SF_IMGI_SIZE; // 超速图像信息长度
		finalLen += ntohl(pi->imageLen); // 超速图像数据长度
		offset = finalLen;
	}
	finalLen += ( SF_IMGI_SIZE + commonImgLen ); //卡口图像信息及数据长度
	finalLen += 1; // 视频数目

	PDEBUG("Final sizeof result = %d\n", finalLen);

	// char *finalData = new char[finalLen];
	if(finalData == NULL) {
		printf("Memory not enough for result packet.\n");
		return 0;
	}
	assert(offset <= finalDataSize);
	memcpy(finalData, pOverspeed->data, offset);

	/* 覆盖车牌号码 */
	SFPRO *pf = (SFPRO *)(finalData);
	memcpy( pf->plate, pc->plate, sizeof(pf->plate) );
	PDEBUG("plate is: %s \n", pf->plate);

	assert( (int)(offset + SF_IMGI_SIZE + commonImgLen) < finalDataSize);
	memcpy(finalData + offset, (pCommon->data + SF_PRO_SIZE), SF_IMGI_SIZE + commonImgLen);

	/* 视频数目 */
	*( (char*)(finalData + finalLen - 1) ) = 0;

#ifdef KISE_DEBUG
	struct sf_img_data *veri = (struct sf_img_data*)(finalData + offset) ;
	PDEBUG("---------->>>Final: %d-%d-%d : %d-%d-%d -- %d\n",
			ntohs(veri->year), ntohs(veri->mon), ntohs(veri->day), 
			ntohs(veri->hour), ntohs(veri->min), ntohs(veri->sec), ntohs(veri->ms) );
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
static bool create_pkg_info_sf(const char* pdata, int size, PKG_INFO_DATA *item)
{
	if( size < (int) (SF_PRO_SIZE + SF_IMGI_SIZE) ) {
		printf("Data can't convert ... len (%d) \n", size);
		return false;
	}

	SFPRO *p = (SFPRO *)pdata;

	item->len = size;  
	item->data = new char[size];
	memcpy(item->data, pdata, size);
	item->direction = string( p->direction ); 
	item->road = ntohl( p->road );
	item->timercv = time(NULL);

	int imgnum = ntohs( p->imageNum );
	if( imgnum == 1 ) {
		item->overspeed = false;
	} else if( imgnum == 2) {	
		item->overspeed = true;
	} else {
		printf("Error, create_pkg_info_sf, image number unexpect (%d)\n", imgnum);
		delete [] item->data;
		return false;
	}

	/* 这里获取第一张图片的信息 */
	SFIMG *imginfo = (SFIMG *)( pdata + SF_PRO_SIZE);

	struct tm pkgtm;
	pkgtm.tm_sec = ntohs(imginfo->sec);
	pkgtm.tm_min = ntohs(imginfo->min);
	pkgtm.tm_hour = ntohs(imginfo->hour);
	pkgtm.tm_mon = ntohs(imginfo->mon) - 1;  // 0 - 11
	pkgtm.tm_year = ntohs(imginfo->year) - 1900;
	pkgtm.tm_mday = ntohs(imginfo->day);

	PDEBUG("Create SF struct, time %d-%d-%d %d:%d:%d\n",
			ntohs(imginfo->year), ntohs(imginfo->mon), ntohs(imginfo->day), 
			ntohs(imginfo->hour), ntohs(imginfo->min), ntohs(imginfo->sec) );

	item->time = mktime(&pkgtm);

	return true;
}


/**********以下是与备份相关的函数*****************/

/* 将网络数据包封装成CCar并进行备份 
 * 由于备份模块new并复制了CCar所以本类结束时将释放自己new的Car对象
 */
static void CreateCCarAndBackup(const char* data, int datasize) {

	if( !ConfigGet()->control.enable_backup )
		return;
#ifdef CONFIG_BACKUP
	//PAINT
	const char *pkg = data; // int pkgsize = datasize;
	//PAINT paint_and_pkg(data, datasize, &pkg, &pkgsize);

	CCar *pcar = new CCar;
	struct sf_protocol_data *p = (struct sf_protocol_data *) pkg;

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

	int offset = SF_PRO_SIZE;
	for(int i = 0; i < pcar->imageNum; i++) {
		struct sf_img_data *pi = (struct sf_img_data *)(pkg + offset);
		struct tm cartm;
		cartm.tm_sec = ntohs(pi->sec);
		cartm.tm_min = ntohs(pi->min);
		cartm.tm_hour = ntohs(pi->hour);
		cartm.tm_mon = ntohs(pi->mon) - 1;  // 0 - 11
		cartm.tm_year = ntohs(pi->year) - 1900;
		cartm.tm_mday = ntohs(pi->day);
		pcar->time[i].tv_sec = mktime( &cartm );
		pcar->time[i].tv_usec = ntohs( pi->ms ) * 1000;
		pcar->imageLen[i] = ntohl(pi->imageLen);

		offset += SF_IMGI_SIZE;

		pcar->image[i] = new char[ pcar->imageLen[i] ];
		memcpy( pcar->image[i], pkg + offset, pcar->imageLen[i] );
		
		offset += ntohl(pi->imageLen);
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
	int length = SF_PRO_SIZE;
	for(int i = 0; i < pcar->imageNum; i++) {
		length += SF_IMGI_SIZE;
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

	PDEBUG("~~~~~~~~~~~~msecond = %d\n", ntohs(pret->msecond) );
	assert( ntohs(pret->msecond) >=0 && ntohs(pret->msecond) < 1000 );

	memcpy(pret->vioType, pcar->vioType, sizeof(pret->vioType) );
	pret->speed = htonl(pcar->speed);
	pret->limitSpeed = htonl(pcar->limitSpeed);
	strncpy( pret->packetID, pcar->packetID.c_str(), sizeof(pret->packetID) );
	pret->imageNum = htons(pcar->imageNum);

	int offset = SF_PRO_SIZE;
	for(int j = 0; j < pcar->imageNum; j++) {
		struct sf_img_data *pi = (struct sf_img_data*) (ret + offset) ;
		struct tm *imgtm = localtime( &(pcar->time[j].tv_sec) );
		pi->year = (short) htons(imgtm->tm_year + 1900);
		pi->mon = (short) htons(imgtm->tm_mon + 1);
		pi->day = (short) htons(imgtm->tm_mday);
		pi->hour = (short) htons(imgtm->tm_hour);
		pi->min = (short) htons(imgtm->tm_min);
		pi->sec = (short) htons(imgtm->tm_sec);
		pi->ms = (short) ( htons( (short)( (pcar->time[j].tv_usec) / 1000) ) );
		
		PDEBUG("~~~~~~~~~image msecond = %d\n", ntohs(pi->ms) );
		assert( ntohs(pi->ms) >=0 && ntohs(pi->ms) < 1000 );
		
		pi->imageLen = htonl( pcar->imageLen[j] );

		offset += SF_IMGI_SIZE;

		memcpy( ret + offset, pcar->image[j], pcar->imageLen[j] );
		
		offset += pcar->imageLen[j];
	}

	*( (char *)(ret + offset) ) = 0;

	g_dispatcher->SendResultToOtherSide(
			ConfigGet()->backup->backup_group_id, 
			ConfigGet()->backup->backup_side, 
			ret, length);

	delete [] ret;
}

