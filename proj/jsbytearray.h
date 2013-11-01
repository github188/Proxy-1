#ifndef _BYTEBUFFER_
#define _BYTEBUFFER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

#define INCREASESIZE 128


#define MSG_FLAG_APPLY      0
#define MSG_FLAG_REPLY      1
/*
 *   以下四个宏是type字段中在掩码
 */
#define TYPEFLAGMASK    0X80000000
#define MODIDMASK       0X7F000000
#define SUBMODIDMASK    0X00FF0000
#define MESSAGEIDMASK   0X0000FFFF

/*
 *  下面参数宏对type字段进行填充与解析
 */
#define INITTYPE(type)          ( type&=0 )//初始化type字段
#define SETTYPEFLAG(type)       (type |= TYPEFLAGMASK)//flag=1
#define RESETTYPEFLAG(type)     (type &= ~TYPEFLAGMASK)//flag=0

#define GETTYPEFLAG(type)               (((type)&TYPEFLAGMASK)>>31)//取type中flag字段

#define GETMODID(type)                  (((type)&MODIDMASK)>>24)// get module id
#define PUTMODID(type,modId)            (type |=(modId<<24))//put module id into type

#define GETSUBMODID(type)               (((type)&SUBMODIDMASK)>>16)//get submodule id
#define PUTSUBMODID(type,subModId)      (type |=(subModId)<<16)//put submodule id into type

#define GETMESSAGEID(type)              ((type)&MESSAGEIDMASK)//get message id
#define PUTMESSAGEID(type,messageId)    (type |=messageId)//put message id into type

int CreateType(int flag,int modID,int subModID,int messageID);
void GetType(int type,int *flag,int *modID,int *subModID,int *messageID);


//这个类的局限性：只能按照包的格式一项一项存放数据和读取数据。

//内部会有一个读写指针，指示读写位置。
//读写指针位置原则：
//写指针：总是在数据的最后，已经指到缓冲区最后，不会重置到开头，因为这一个类只保存一个数据包。
//读指针：未读时放在最前，读过则后移，读完则重置到开头

//resize会保存原有数据，缓冲区长度也会变化,写指针会在数据的最后，读指针指向数据开头。
class CJSByteArray
{
public:
	CJSByteArray();
	CJSByteArray(const CJSByteArray &byteArr);
	CJSByteArray(int nLen);
	~CJSByteArray();

	int PutInt(int n);
	int PutShort(short n);
	int PutLong(long n);
	int PutFloat(float num);
	int PutString(const char str[], int nLen);
	int PutByte(unsigned char byte[], int nLen);
	int PutByte(unsigned char byte);

	int GetInt();
	short GetShort();
	long GetLong();
	float GetFloat();
	char* GetString(int nLen);
	int GetString(char* string, int nLen);
	int GetByte(unsigned char byte[], int nLen);
	int GetByte(unsigned char &byte);

	//将写指针移动到pos位置，即下次写的时候会从pos处开始写
	int ModeWritePos(int pos);

	//放入整个数据包，取整个数据包
	//把从网络上接收到的数据包放到ByteArray中，返回放入的长度,可以重复放入
	int PutData(const char str[], int nLen);
	char* GetData(void);

	//释放上次申请的内存，重新申请n大小内存
	int Resize(int n);

	//TLV的总长度
	int Length(void);

	//清除数据，不释放内存
	int Clear(void);
	//int Destroy(void);

	int End(void);

	int GetT(void);
	int GetL(void);

	void UnpackBegin(void);
	CJSByteArray& operator=(const CJSByteArray& byteArr);

private:
	int GetValidSize(void);
	bool EnoughGet(int size);


private:
	unsigned char *m_pBuffer;
	unsigned char *m_pCurrentWritePos;
	unsigned char *m_pCurrentReadPos;

	char *p;

	int m_nLength;		//buffer里面数据的长度
	int m_nBufferSize;	//buffer的总长度
};


#endif //_BYTEBUFFER_
