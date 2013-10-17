/*
 * MessageDef.h
 *
 *  Created on: 2011-1-22
 *      Author: cchao
 */

#ifndef MESSAGEDEF_H_
#define MESSAGEDEF_H_

enum MSG_TYPE {
	MSG_TYPE_UNKNOWN = -1,
	MSG_TYPE_CAR = 1000,	// 车辆信息
	MSG_TYPE_TRAFFIC_FLOW,	// 路口车流信息
	MSG_TYPE_DETECT_MODE,	// 检测模式
	MSG_TYPE_DEVICE_STATE, 	// 设备状态

	// Debug信息包

	MSG_TYPE_DEBUG_IMG = 2000,	// Debug图像数据
	MSG_TYPE_DEBUG_FPS,		// Debug帧率
	MSG_TYPE_DEBUG_REMAIN,	// Debug剩余未处理数据
	MSG_TYPE_DEBUG_HASCAR,	// Debug检测到车辆

	// 相机控制信息包

	MSG_TYPE_CAM_START = 3000,					// 相机参数下界
	MSG_TYPE_CAM_SWITCH = 3001,					// 相机切换
	MSG_TYPE_CAM_VIDEO = 3002,					// 相机视频信息
	MSG_TYPE_CAM_PARAM_QUERY = 3003,			// 查询相机参数
	MSG_TYPE_CAM_PARAM_SETTING = 3004,			// 相机参数设置
	MSG_TYPE_CAM_PARAM_SETTING_REPLY = 3004,	// 相机参数设置回应
	MSG_TYPE_CAM_PARAM_REPLY = 3005,			// 相机参数内容回应信息
	MSG_TYPE_CAM_RESET = 3006,					// 相机重启
	MSG_TYPE_CAM_TRIGGER = 3007,				// 相机软触发
	MSG_TYPE_CAM_BEAT = 3008,					// 相机心跳
	MSG_TYPE_CAM_BEAT_REPLY,					// 相机心跳回应信息
	MSG_TYPE_CAM_END							// 相机参数上界
};

namespace MSG_CAMERA_CD
{
	enum {
		TYPE_SWITCH_0 = 0,
		TYPE_SWITCH_1,
		TYPE_SWITCH_2,
		TYPE_SWITCH_3,
		TYPE_SWITCH_4,
		TYPE_HEADER = 0x55aa55aa,
		TYPE_PARAM = 0x55aa55aa,
		TYPE_RESET = 0xaaaa,
		TYPE_TRIGGER = 0xbbbb,
		TYPE_FRAME = 0x33333333,	// 任意值
		TYPE_BEAT = 0xaaaaaaaa
	};
	enum {
		TYPE_PARAM_SETTING = 0x55aa55aa,
		TYPE_PARAM_SETTING_REPLY = 0x55aa55aa,
		TYPE_PARAM_QUERY = 0x55aa55aa,
		TYPE_PARAM_REPLY = 0x55aa55aa
	};
}

namespace CAMERA_SETTING_CD
{
	enum {
		ITEM_GAIN = 0x90,
		ITEM_FPS = 0x92,
		ITEM_EXPOSURE = 0x93,
		ITEM_BALANCE = 0xd4,
		ITEM_CONTRAST = 0xe0,
		ITEM_JPEGQTY = 0x128,
		ITEM_LED = 0x130
	};
	enum {
		VAL_BALANCE_OFF = 0,
		VAL_BALANCE_ON = 1
	};
}

namespace CAMERA_SETTING_STD
{
	enum {
		ITEM_GAIN = 0,
		ITEM_FPS,
		ITEM_EXPOSURE,
		ITEM_BALANCE,
		ITEM_CONTRAST,
		ITEM_JPEGQTY,
		ITEM_LED
	};
	enum {
		VAL_BALANCE_OFF = 0,
		VAL_BALANCE_ON = 1
	};
}

namespace DEBUG_INFO
{
	enum {
		ITEM_COIL,
		ITEM_VIDEO
	};
}

#endif /* MESSAGEDEF_H_ */
