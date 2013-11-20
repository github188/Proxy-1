
#ifndef CONVERT_ZJ_H
#define CONVERT_ZJ_H

struct zj_protocol_data {
	int type;
	int len;

	int flag;

	char monitorID[32];
	char direction[8];

	int cameraNo;
	int roadNo;

	char plate[16];
	int plateX;
	int plateY;
	int plateWidth;
	int plateHeight;
	int confidence;
	short plateColor;

	short style;
	short color;
	char reserved;

	char detect;

	int speed;
	int limitSpeed;

	char packetID[64];
	short imageNum;
} __attribute__( (packed, aligned(1) ) );

typedef struct zj_protocol_data ZJPRO;
const static size_t ZJ_PRO_SIZE = sizeof(struct zj_protocol_data);

struct zj_img_data {
	short year;
	short mon;
	short day;
	short hour;
	short min;
	short sec;
	short ms;
	long imageLen;
	char imgFirstByte;
} __attribute__((packed, aligned(1)));

typedef struct zj_img_data ZJIMG;
const static size_t ZJ_IMGI_SIZE = sizeof(struct zj_img_data) - 1;

#endif

