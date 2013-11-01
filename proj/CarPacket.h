#ifndef CARPACKET_H
#define CARPACKET_H

#include "../../CarDetector/Car.h"
#include "../jsbytearray.h"
#include "../Message.h"

#include "../../Common.h"

//结果数据包
void PackCarInfo(CJSByteArray& pack, const CCar* pCar);
void UnpackCarInfo(CJSByteArray& pack, CCar* pCar);

void UnpackCarInfoAck(CJSByteArray& pack, char* packetID);

void PackTrafficFlowData(CJSByteArray& pack, Traffic_Flow_Data& trafficFlowData);

void UnPackModeManagerPack(CJSByteArray& pack, ModeChangePack& modeChangePack);
void PackModeManagerPackAck(CJSByteArray& pack, unsigned char flag);

void PackDeviceState(CJSByteArray& pack, const DeviceState& deviceState);

void ResetValueLength(CJSByteArray & pack);

void CreatePacketID(const Traffic_Flow_Data& trafficFlowData, char packetID[]);

void PackCameraState(CJSByteArray& pack, const string& strMonitorID, const string& strDirectionNo, int cameraNo, \
        const char* cameraIP, unsigned char detect, unsigned char state);

//校时包
void UnPackSyncTimePack(CJSByteArray& pack, struct timeval& tv);
void PackSyncTimeAck(CJSByteArray& pack, struct timeval tv, bool syncFlag);

#endif //CARPACKET_H
