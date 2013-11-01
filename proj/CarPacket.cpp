#include "CarPacket.h"
#include <assert.h>
#include <arpa/inet.h>
#include "../Message.h"
#include "../../Tool/timeclass.h"

extern int g_nRoadNo;
extern int g_nCameraNo;
// extern int g_nProtocol;

void ResetValueLength(CJSByteArray & pack)
{
    pack.End();
}

void PackCarInfo(CJSByteArray& pack, const CCar* pCar)
{
    if (pCar == NULL)
    {
        return;
    }

    int type = 0;
    type = CreateType(1, 0, 0, MSG_TYPE_CAR);
    pack.PutInt(type);  // T 4
    pack.PutInt(0);     // L 4

    pack.PutInt(2);     //   4
    
    char monitorID[32];
    monitorID[0] = 0;
    strcpy(monitorID, pCar->monitorID.c_str());
    pack.PutString(monitorID, 32); // 32 

    char direction[8];
    direction[0] = 0;
    strcpy(direction, pCar->direction.c_str());
    pack.PutString(direction, 8);   // 8
    pack.PutInt(pCar->cameraNo-1);  // 4 川大智胜，从0开始

    pack.PutInt(pCar->roadNo);      // 4

    pack.PutString(pCar->plate, 16);  // 16
    pack.PutInt(pCar->plateX);        // 4
    pack.PutInt(pCar->plateY);        // 4
    pack.PutInt(pCar->plateWidth);    // 4
    pack.PutInt(pCar->plateHeight);   // 4
    pack.PutInt(pCar->confidence);    // 4
    pack.PutShort(pCar->plateColor);  // 2

    pack.PutShort(pCar->style);       // 2
    pack.PutShort(pCar->color);       // 2

    pack.PutByte(0x00);               // 1
 
    pack.PutByte(pCar->detect);       // 1

    pack.PutInt(pCar->speed);         // 4  *
    pack.PutInt(pCar->limitSpeed);    // 4  *

    string packetID;
    pCar->CreatePacketID(pCar, packetID);

    char pID[64];
    pID[0] = 0;
    strcpy(pID, packetID.c_str());
    pack.PutString(pID, 64);          // 64

    pack.PutShort(pCar->imageNum);    // 2

    for (int i=0; i<pCar->imageNum; i++)
    {
        //时间
        CTime t(pCar->time[i]);
        pack.PutShort(t.Year());      // 2   *
        pack.PutShort(t.Month());     // 2
        pack.PutShort(t.Day());       // 2
        pack.PutShort(t.Hour());      // 2
        pack.PutShort(t.Minute());    // 2
        pack.PutShort(t.Second());    // 2
        pack.PutShort(t.MilliSecond()); // 2

        //图片长度
        pack.PutLong(pCar->imageLen[i]);

        //jpeg图片数据
        if (pCar->imageLen[i] > 0 && pCar->image[i] != NULL)
        {
            pack.PutByte((unsigned char*)(pCar->image[i]), pCar->imageLen[i]);
        }
    }

    pack.End();
}

void UnpackCarInfo(CJSByteArray& pack, CCar* pCar)
{}

void UnpackCarInfoAck(CJSByteArray& pack, char* packetID)
{}

void PackTrafficFlowData(CJSByteArray& pack, Traffic_Flow_Data& trafficFlowData)
{
    int type = 0;
    type = CreateType(MSG_FLAG_REPLY, 0, 0, MSG_TYPE_TRAFFICFLOW);
    pack.PutInt(type);
    pack.PutInt(0);

    char packetID[32];
    CreatePacketID(trafficFlowData, packetID);
    pack.PutString(packetID, 32);
    pack.PutInt(trafficFlowData.monitorID);
    pack.PutString(trafficFlowData.StartTime, 28);
    pack.PutString(trafficFlowData.EndTime, 28);
    pack.PutByte((unsigned char)trafficFlowData.RoadNum);

    for (int i=0; i<(int)trafficFlowData.RoadNum; i++)
    {
        pack.PutByte((unsigned char)(trafficFlowData.data[i].RoadNo));
        pack.PutInt(trafficFlowData.data[i].Flow);
        pack.PutByte((unsigned char)trafficFlowData.data[i].OptRate);
        pack.PutByte((unsigned char)trafficFlowData.data[i].AverSpeed);
        pack.PutByte((unsigned char)trafficFlowData.data[i].AverCarLenth);
        pack.PutInt(trafficFlowData.data[i].AverCarDistance);
        pack.PutByte((unsigned char)trafficFlowData.data[i].Available);
        pack.PutString(trafficFlowData.data[i].Reserved, 16);
    }

    pack.End();
}

void UnPackModeManagerPack(CJSByteArray& pack, ModeChangePack& modeChangePack)
{
    //长度
    pack.GetInt();

    modeChangePack.cameraNo = pack.GetShort();
    pack.GetByte(modeChangePack.detect);
}

void PackModeManagerPackAck(CJSByteArray& pack, unsigned char flag)
{
    int type = 0;
    type = CreateType(MSG_FLAG_REPLY, 0, 0, MSG_TYPE_MODECHANGE_ACK);
    pack.PutInt(type);
    pack.PutInt(0);

    pack.PutByte(flag);
    pack.End();
}

void PackDeviceState(CJSByteArray& pack, const DeviceState& deviceState)
{

}

void CreatePacketID(const Traffic_Flow_Data& trafficFlowData, char packetID[])
{
    memset(packetID, 'a', 31);
    packetID[31] = 0;
}

void PackCameraState(CJSByteArray& pack, const string& strMonitorID, const string& strDirectionNo, int cameraNo, \
        const char* cameraIP, unsigned char detect, unsigned char state)
{
    assert(cameraIP != NULL);

    int type = 0;
    type = CreateType(MSG_FLAG_REPLY, 0, 0, MSG_TYPE_CAMERASTATE);
    pack.PutInt(type);
    pack.PutInt(0);

    char monitorID[32];
    monitorID[0] = 0;
    strcpy(monitorID, strMonitorID.c_str());
    pack.PutString(monitorID, 32);

    char direction[8];
    direction[0] = 0;
    strcpy(direction, strDirectionNo.c_str());
    pack.PutString(direction, 8);

    pack.PutInt(cameraNo-1);  //川大智胜，从0开始

    unsigned int ip = inet_addr(cameraIP);
    pack.PutInt(ip);

    pack.PutByte(detect);
    pack.PutByte(state);

    pack.End();
}


void UnPackSyncTimePack(CJSByteArray& pack, struct timeval& tv)
{
    //长度
    pack.GetInt();

    short year = pack.GetShort();
    short month = pack.GetShort();
    short day = pack.GetShort();
    short hour = pack.GetShort();
    short minute = pack.GetShort();
    short second = pack.GetShort();
    short millsecond = pack.GetShort();

    printf("sync time, %.4d-%.2d-%.2d %.2d:%.2d:%.2d:%.3d\n", year, month, day, hour, minute, second, millsecond);

    CTime t;
    tv.tv_sec = t.UnixTime(year, month, day, hour, minute, second);
    tv.tv_usec = millsecond*1000;
}

void PackSyncTimeAck(CJSByteArray& pack, struct timeval tv, bool syncFlag)
{
    CTime t(tv);
    short year = t.Year();
    short month = t.Month();
    short day = t.Day();
    short hour = t.Hour();
    short minute = t.Minute();
    short second = t.Second();
    short millsecond = t.MilliSecond();

    int type = CreateType(MSG_FLAG_REPLY, 0, 0, MSG_TYPE_SYNCTIME_ACK);
    pack.PutInt(type);
    pack.PutInt(0);

    pack.PutShort(year);
    pack.PutShort(month);
    pack.PutShort(day);
    pack.PutShort(hour);
    pack.PutShort(minute);
    pack.PutShort(second);
    pack.PutShort(millsecond);
    pack.PutShort(syncFlag?1:0);
    pack.End();
}



