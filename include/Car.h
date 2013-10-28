//********************************************************
//作者:陈金义
//描述:Car表示的是卡口信息，一个这样的结构表示一辆车的卡口信息，对于多辆车则用数组形式
//********************************************************

#ifndef CAR_H
#define CAR_H

#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>
#include <assert.h>
#include <string>

#include "../Tool/timeclass.h"

using namespace std;

// #include "../Tool/timeclass.h"

const int CAR_NUM = 4; //超速的时候会有两张图片
const int MAX_VIDEO_NUM = 2;

class CCar
{
public:
    void CreatePacketID(const CCar* pCar, string& packetID) const
    {
        assert(pCar != NULL);

        char tmp[256];

        CTime t(pCar->time[0]);
        snprintf(tmp, sizeof(tmp), "%d_%s_%s_%d_%.4d%.2d%.2d%.2d%.2d%.2d%.3d",  \
                pCar->cameraNo, pCar->monitorID.c_str(), pCar->direction.c_str(), pCar->roadNo, \
                t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second(), t.MilliSecond());
        packetID = tmp;
    }
public:
    string packetID;
    string machineID;
    string monitorID;      /*监控点编号*/
    string direction;      /*方向编号*/
    int cameraNo;       /*相机编号,车道内侧第一个检测相机编号为一*/
    int virtualRoadNo;  /*虚拟车道编号，从0开始，加上车道开始偏移量就是真实车道号*/
    int roadNo;         /*车道号,车道内侧第一根道为车道一*/

    char plate[16];             /*车牌号码*/
    int plateX;                 /*车牌坐标X*/
    int plateY;                 /*车牌坐标Y*/
    int plateWidth;             /*车牌width*/
    int plateHeight;            /*车牌height*/
    int confidence;             /*自信度*/
    unsigned short plateColor;  /*车牌颜色,具体查看宏定义*/

    unsigned short style;       /*车型,具体查看宏定义*/
    unsigned short color;       /*车身颜色,具体查看宏定义，现在不用*/

	unsigned char detect; /*检测方式0x00-视频，0x01-线圈*/
	bool isViolation; /*是否违章*/

	// 红灯开始时间
	struct timeval tmRedBegin;

	char vioType[32]; // 违法类型

	int speed; /*车辆实际速度*/
	int limitSpeed; /*最大限速*/

    unsigned short imageNum;                    /*图片数量*/
    struct timeval time[CAR_NUM];               /*图片抓拍时间，超速时会有两张*/

	unsigned long imageLen[CAR_NUM]; /*车辆图片长度*/

	char* image[CAR_NUM]; //jpeg data           /*车辆图片*/

	int videoNum; // 视频数目,实际为char类型

	// 各个视频起止时间

	struct timeval tmVideoBegin[MAX_VIDEO_NUM];
	struct timeval tmVideoEnd[MAX_VIDEO_NUM];

	char videoFormat[MAX_VIDEO_NUM][8]; // 视频格式"ASF", "AVI", "MP4"
	unsigned long videoLen[MAX_VIDEO_NUM];
	char* videoData[MAX_VIDEO_NUM];

    CCar()
    {
        machineID = "1000";
        monitorID = "1000";
        cameraNo = 0;
        virtualRoadNo = 0;
        roadNo = 1;
        direction = "0";

        memset(plate, 0, sizeof(plate));
        plateX = 0;
        plateY = 0;
        plateWidth = 0;
        plateHeight = 0;
        confidence = 0;

        plateColor = 0;         /*车牌颜色*/
        style = 0;              /*车型*/
        color = 0;

		detect = 0;
		isViolation = false;

		gettimeofday(&tmRedBegin, NULL);
		memset(vioType, 0, sizeof(vioType));

		speed = 0;
		limitSpeed = 0;

		imageNum = 0;

		for (int i = 0; i < CAR_NUM; i++)
		{
			gettimeofday(&time[i], NULL);
			imageLen[i] = 0;
			image[i] = NULL;
		}

		videoNum = 0;
		for (int i = 0; i < MAX_VIDEO_NUM; i++)
		{
			gettimeofday(&tmVideoBegin[i], NULL);
			gettimeofday(&tmVideoEnd[i], NULL);
			memset(videoFormat[i], 0, sizeof(videoFormat[i]));
			videoLen[i] = 0;
			videoData[i] = NULL;
		}
	}

	~CCar()
	{
		for (int i = 0; i < CAR_NUM; i++)
		{
			if (image[i] != NULL)
			{
				delete[] image[i];
				image[i] = NULL;
			}
		}

		for (int i = 0; i < MAX_VIDEO_NUM; i++)
		{
		    if (videoData[i] != NULL)
		    {
		        delete[] videoData[i];
		        videoData[i] = NULL;
		    }
		}
	}

	CCar(const CCar& rhs)
	{
		Copy(rhs);

		for (int i = 0; i < CAR_NUM; i++)
		{
			if (rhs.image[i] != NULL && rhs.imageLen[i] > 0)
			{
				image[i] = new char[rhs.imageLen[i]];
				memcpy(image[i], rhs.image[i], rhs.imageLen[i]);
			}
			else
			{
			    image[i] = NULL;
			}
		}

        for (int i = 0; i < MAX_VIDEO_NUM; i++)
        {
            if (rhs.videoData[i] != NULL && rhs.videoLen[i] > 0)
            {
                videoData[i] = new char [rhs.videoLen[i]];
                memcpy(videoData[i], rhs.videoData[i], rhs.videoLen[i]);
            }
            else
            {
                videoData[i] = NULL;
            }
        }
    }

    CCar& operator=(const CCar& rhs)
    {

        if (this == &rhs)
        {
            return *this;
        }

        Copy(rhs);

        for (int i = 0; i < CAR_NUM; i++)
        {
            if (image[i] != NULL)
            {
                delete[] image[i];
                image[i] = NULL;
            }

			if (rhs.image[i] != NULL && rhs.imageLen[i] > 0)
			{
				image[i] = new char[rhs.imageLen[i]];
				memcpy(image[i], rhs.image[i], rhs.imageLen[i]);
			}
		}

		for (int i = 0; i < MAX_VIDEO_NUM; i++)
		{
			if (videoData[i] != NULL)
			{
				delete[] videoData[i];
				videoData[i] = NULL;
			}

			if (rhs.videoData[i] != NULL && rhs.videoLen[i] > 0)
			{
			    videoData[i] = new char [rhs.videoLen[i]];
			    memcpy(videoData[i], rhs.videoData[i], rhs.videoLen[i]);
			}
		}

        return *this;

    }

private:
    void Copy(const CCar& rhs)
    {
        machineID = rhs.machineID;
        monitorID = rhs.monitorID;
        cameraNo = rhs.cameraNo;
        virtualRoadNo = rhs.virtualRoadNo;
        roadNo = rhs.roadNo; /*路口编号*/
        direction = rhs.direction; /*方向编号*/

        memcpy(plate, rhs.plate, 16);

        plateX = rhs.plateX;
        plateY = rhs.plateY;
        plateWidth = rhs.plateWidth;
        plateHeight = rhs.plateHeight;
        confidence = rhs.confidence;

        plateColor = rhs.plateColor;
        style = rhs.style;
        color = rhs.color;

		detect = rhs.detect; /*检测方式0-视频，1-线圈*/
		isViolation = rhs.isViolation;

		tmRedBegin = rhs.tmRedBegin;
		strcpy(vioType, rhs.vioType);

		speed = rhs.speed;
		limitSpeed = rhs.limitSpeed;

		imageNum = rhs.imageNum;

		for (int i = 0; i < CAR_NUM; i++)
		{
			time[i] = rhs.time[i];
			imageLen[i] = rhs.imageLen[i];
		}

		videoNum = rhs.videoNum;
		for (int i = 0; i < MAX_VIDEO_NUM; i++)
		{
			tmVideoBegin[i] = rhs.tmVideoBegin[i];
			tmVideoEnd[i] = rhs.tmVideoEnd[i];
			strcpy(videoFormat[i], rhs.videoFormat[i]);
			videoLen[i] = rhs.videoLen[i];
		}
	}
};


#endif //CAR_H

