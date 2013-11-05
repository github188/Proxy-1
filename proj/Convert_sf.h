

#ifndef CONVERT_SF_H
#define CONVERT_SF_H

#include "../include/Car.h"

struct sf_protocol_data {
	int type;
	int length;
	char monitorID[32];
	char direction[8];
	int camera;
	int road;
	char plate[16];
	int plateX;
	int plateY;
	int plateWidth;
	int plateHeight;
	int confidence;
	short plateColor;
	short style;
	short color;
	char detect;
	char isViolation;
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
	short msecond;
	char vioType[32];
	int speed;
	int limitSpeed;
	char packetID[64];
	short imageNum;
} __attribute__((packed,aligned(1)));

struct sf_img_data {
	short year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
	short msecond;
	long length;
} __attribute__( ( packed, aligned(1) ) );

void backup_callback(CCar *pcar);

#endif
