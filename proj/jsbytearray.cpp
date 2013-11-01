/*
 * jsbytearray.cpp
 *
 *  Created on: 2009-6-30
 *      Author: root
 */

#include "jsbytearray.h"
#include <netinet/in.h>

int CreateType(int flag,int modID,int subModID,int messageID)
{
    int type;
    INITTYPE(type);
    if(flag)
    {
        SETTYPEFLAG(type);
    }
    else
    {
        RESETTYPEFLAG(type);
    }
    PUTMODID(type,modID);
    PUTSUBMODID(type,subModID);
    PUTMESSAGEID(type,messageID);

    return type;
}

void GetType(int type,int *flag,int *modID,int *subModID,int *messageID)
{
    if (flag != NULL)
    {
        *flag = GETTYPEFLAG(type);
    }

    if (modID != NULL)
    {
        *modID = GETMODID(type);
    }

    if (subModID != NULL)
    {
        *subModID = GETSUBMODID(type);
    }

    if (messageID != NULL)
    {
        *messageID = GETMESSAGEID(type);
    }
}

CJSByteArray::CJSByteArray()
{
	m_pBuffer = NULL;
	m_pCurrentWritePos = m_pBuffer;
	m_pCurrentReadPos = m_pBuffer;
	m_nLength = 0;
	m_nBufferSize = 0;
	this->p = NULL;
}
CJSByteArray::CJSByteArray(const CJSByteArray &byteArr)
{
	this->p = NULL;
	this->m_nLength = byteArr.m_nLength;
	this->m_nBufferSize = byteArr.m_nBufferSize;

	this->m_pBuffer = new unsigned char[m_nBufferSize];
	if (m_pBuffer == NULL)
	{
		return;
	}
	for (int i = 0; i < m_nLength; i++)
	{
		m_pBuffer[i] = byteArr.m_pBuffer[i];
	}
	this->m_pCurrentReadPos = m_pBuffer;
	this->m_pCurrentReadPos = m_pBuffer + (byteArr.m_pCurrentReadPos
			- byteArr.m_pBuffer);

	this->m_pCurrentWritePos = m_pBuffer;
	this->m_pCurrentWritePos = m_pBuffer + (byteArr.m_pCurrentWritePos
				- byteArr.m_pCurrentWritePos);

}

CJSByteArray& CJSByteArray::operator=(const CJSByteArray &byteArr)
{
	this->p = NULL;
	this->m_nLength = byteArr.m_nLength;
	this->m_nBufferSize = byteArr.m_nBufferSize;

	this->m_pBuffer = new unsigned char[m_nBufferSize];
	if (m_pBuffer == NULL)
	{
		return *this;
	}
	for (int i = 0; i < m_nLength; i++)
	{
		m_pBuffer[i] = byteArr.m_pBuffer[i];
	}

	this->m_pCurrentReadPos = m_pBuffer;
	this->m_pCurrentReadPos = m_pBuffer + (byteArr.m_pCurrentReadPos
			- byteArr.m_pBuffer);

	this->m_pCurrentWritePos = m_pBuffer;
	this->m_pCurrentWritePos = m_pBuffer + (byteArr.m_pCurrentWritePos
				- byteArr.m_pCurrentWritePos);
	return *this;
}
CJSByteArray::CJSByteArray(int nLen)
{
	if (nLen <= 0)
	{
		return;
	}

	this->m_pBuffer = new unsigned char[nLen];
	if (this->m_pBuffer == NULL)
	{
		printf("malloc byte array error\n");
		return;
	}

	m_pCurrentWritePos = m_pBuffer;
	m_pCurrentReadPos = m_pBuffer;
	m_nLength = 0;
	m_nBufferSize = nLen;

	this->p = NULL;
}

CJSByteArray::~CJSByteArray()
{
	if (this->m_pBuffer != NULL)
	{
		delete[] this->m_pBuffer;
		this->m_pBuffer = NULL;
	}

	if (this->p != NULL)
	{
		delete this->p;
		this->p = NULL;
	}

	this->m_pCurrentWritePos = NULL;
	this->m_pCurrentReadPos = NULL;
	this->m_nLength = 0;
	m_nBufferSize = 0;
}

int CJSByteArray::PutInt(int nValue)
{
	/*
	 if (this->m_pBuffer == NULL)
	 {
	 return -1;
	 }
	 */

	int size = sizeof(int);
	if (GetValidSize() < size)
	{
		Resize(m_nBufferSize + size + INCREASESIZE);
	}

	int *p = (int *) m_pCurrentWritePos;
	*p = htonl(nValue);

	//当前写指针后移
	m_pCurrentWritePos += size;

	//当前数据长度增加
	m_nLength += size;

	return 0;
}


int CJSByteArray::PutLong(long nValue)
{
	/*
	 if (this->m_pBuffer == NULL)
	 {
	 return -1;
	 }
	 */

	int size = sizeof(long);
	if (GetValidSize() < size)
	{
		Resize(m_nBufferSize + size + INCREASESIZE);
	}

	long *p = (long *) m_pCurrentWritePos;
	*p = htonl(nValue);

	//当前写指针后移
	m_pCurrentWritePos += size;

	//当前数据长度增加
	m_nLength += size;

	return 0;
}

int CJSByteArray::PutFloat(float num)
{
	int tmp;
	memcpy(&tmp, &num, sizeof(float));

	PutInt(tmp);

    return 0;
}

int CJSByteArray::PutShort(short nValue)
{
	int size = sizeof(short);
	if (GetValidSize() < size)
	{
		Resize(m_nBufferSize + size + INCREASESIZE);
	}

	short *p = (short *) m_pCurrentWritePos;
	*p = htons(nValue);

	//当前写指针后移
	m_pCurrentWritePos += size;

	//当前数据长度增加
	m_nLength += size;

	return 0;
}

int CJSByteArray::PutString(const char str[], int nLen)
{
	/*
	 if (this->m_pBuffer == NULL)
	 {
	 return -1;
	 }
	 */

	if (str == NULL || nLen <= 0)
	{
		return -1;
	}

	int size = nLen;
	if (GetValidSize() < size)
	{
		Resize(m_nBufferSize + size + INCREASESIZE);
	}

//	for (int i = 0; i < nLen; i++)
//	{
//		m_pCurrentWritePos[i] = str[i];
//	}
	memcpy(m_pCurrentWritePos, str, nLen);

	m_pCurrentWritePos += nLen;

	//当前数据长度增加
	m_nLength += size;
	return nLen;
}

int CJSByteArray::PutByte(unsigned char byte[], int nLen)
{
	if (byte == NULL || nLen <= 0)
	{
		return -1;
	}

	int size = nLen;
	if (GetValidSize() < size)
	{
		Resize(m_nBufferSize + size + INCREASESIZE);
	}

//	for (int i = 0; i < nLen; i++)
//	{
//		m_pCurrentWritePos[i] = byte[i];
//	}
	memcpy(m_pCurrentWritePos, byte, nLen);

	m_pCurrentWritePos += nLen;

	//当前数据长度增加
	m_nLength += size;

	return nLen;
}


int CJSByteArray::PutByte(unsigned char byte)
{
	if (GetValidSize() < 1)
	{
		Resize(m_nBufferSize + 1 + INCREASESIZE);
	}

	m_pCurrentWritePos[0] = byte;

	m_pCurrentWritePos += 1;

	//当前数据长度增加
	m_nLength += 1;

	return 0;
}



int CJSByteArray::GetInt()
{
	if (m_pBuffer == NULL)
	{
		return -1;
	}

	if (!EnoughGet(sizeof(int)))
	{
		return -1;
	}

	int *p = (int *) m_pCurrentReadPos;

	*p = ntohl(*p);
	//读指针后移
	m_pCurrentReadPos += sizeof(int);

	return *p;
}

short CJSByteArray::GetShort()
{
	if (m_pBuffer == NULL)
	{
		printf("m_pBuffer==NULL\n");
		return -1;
	}

	if (!EnoughGet(sizeof(short)))
	{
		printf("not enough get\n");
		return -1;
	}

	short *p = (short *) m_pCurrentReadPos;
	*p = ntohs(*p);

	//读指针后移
	m_pCurrentReadPos += sizeof(short);

	return *p;
}


long CJSByteArray::GetLong()
{
	if (m_pBuffer == NULL)
	{
		return -1;
	}

	if (!EnoughGet(sizeof(long)))
	{
		return -1;
	}

	long *p = (long *) m_pCurrentReadPos;
	*p = ntohl(*p);

	//读指针后移
	m_pCurrentReadPos += sizeof(long);

	return *p;
}

float CJSByteArray::GetFloat()
{
	int num = GetInt();

	float tmp;
	memcpy(&tmp, &num, sizeof(int));

	return tmp;
}

char* CJSByteArray::GetString(int nLen)
{
	if (m_pBuffer == NULL)
	{
		return NULL;
	}

	if ((nLen > this->m_nLength) || nLen < 0)
	{
		return NULL;
	}

	if (!EnoughGet(nLen))
	{
		return NULL;
	}

	if (p != NULL)
	{
		delete[] p;
		p = NULL;
	}

	p = new char[nLen + 1];
	if (p == NULL)
	{
		return NULL;
	}

	memset(p, 0, nLen + 1);

	for (int i = 0; i < nLen; i++)
	{
		p[i] = m_pCurrentReadPos[i];
	}
	p[nLen] = '\0';

	m_pCurrentReadPos += nLen;

	//读指针到结尾，则重置到开头
	if (m_pCurrentReadPos - m_nBufferSize == m_pBuffer)
	{
		m_pCurrentReadPos = m_pBuffer;
	}

	return p;
}

int CJSByteArray::GetString(char* string, int nLen)
{
    if (m_pBuffer == NULL || string == NULL)
    {
        return -1;
    }

    if ((nLen > this->m_nLength) || nLen < 0)
    {
        return -1;
    }

    if (!EnoughGet(nLen))
    {
        return -1;
    }

    memcpy(string, m_pCurrentReadPos, nLen);

//    for (int i = 0; i < nLen; i++)
//    {
//        p[i] = m_pCurrentReadPos[i];
//    }
//    p[nLen] = '\0';

    m_pCurrentReadPos += nLen;

    //读指针到结尾，则重置到开头
    if (m_pCurrentReadPos - m_nBufferSize == m_pBuffer)
    {
        m_pCurrentReadPos = m_pBuffer;
    }

    return 0;
}

int CJSByteArray::GetByte(unsigned char byte[], int nLen)
{
	if (m_pBuffer == NULL)
	{
		return -1;
	}

	if (!EnoughGet(nLen))
	{
		return -1;
	}

	memcpy(byte, m_pCurrentReadPos, nLen);
//	for (int i = 0; i < nLen; i++)
//	{
//		byte[i] = m_pCurrentReadPos[i];
//	}

	m_pCurrentReadPos += nLen;

	return nLen;
}


int CJSByteArray::GetByte(unsigned char &byte)
{
	if (m_pBuffer == NULL)
	{
		return -1;
	}

	if (!EnoughGet(1))
	{
		return -1;
	}

	byte = m_pCurrentReadPos[0];

	m_pCurrentReadPos++;

	return 0;
}

//把从网络上接收到的数据包放到ByteArray中，返回放入的长度
//可以放入多个数据包
int CJSByteArray::PutData(const char str[], int nLen)
{
	if (str == NULL)
	{
		return -1;
	}

	//申请新的空间
	unsigned char *pTempBuffer = new unsigned char[nLen + Length()];
	if (pTempBuffer == NULL)
	{
		return -1;
	}
	memset(pTempBuffer, 0, nLen + Length());

	//把原来数据放进去，并记录下标
	//int i = 0;
	memcpy(pTempBuffer, m_pBuffer, m_nLength);
//	for (i = 0; i < m_nLength; i++)
//	{
//		pTempBuffer[i] = m_pBuffer[i];
//	}

	//将传入的str中的数据放入尾端
	memcpy(pTempBuffer+m_nLength, str, nLen);
//	int j = 0;
//	while (j < nLen)
//	{
//		pTempBuffer[i + j] = str[j];
//		j++;
//	}

	//原来有数据则删掉
	if (m_pBuffer != NULL)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

	//重新定位指针
	m_pBuffer = pTempBuffer;

	m_pCurrentWritePos = m_pBuffer;
	m_pCurrentReadPos = m_pBuffer;

	//重新数据计算长度
	m_nLength += nLen;

	return nLen;
}

char* CJSByteArray::GetData(void)
{
	return (char *) m_pBuffer;
}

//返回改变的长度，必须大于0
//resize的时候，数据会被保留
int CJSByteArray::Resize(int n)
{
	//错误的参数
	if (n <= 0)
	{
		return -1;
	}

	//与原来长度相同，则不用resize
	if (n == this->m_nLength)
	{
		return m_nLength;
	}

	unsigned char *pTemp = new unsigned char[n];
	memset(pTemp, 0, n);

	int min = (m_nLength < n) ? m_nLength : n;

	memcpy(pTemp, m_pBuffer, min);
//	int i = 0;
//	while (i < min)
//	{
//		pTemp[i] = m_pBuffer[i];
//		i++;
//	}

	if (m_pBuffer != NULL)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

	m_pBuffer = pTemp;
	m_nLength = min;
	m_nBufferSize = n;

	m_pCurrentWritePos = m_pBuffer + min;
	m_pCurrentReadPos = m_pBuffer; //指向新的m_pBuffer

	return n;

}

int CJSByteArray::Length(void)
{
	return this->m_nLength;
}

//数据清空，长度不变。读写指针重新赋值
int CJSByteArray::Clear(void)
{
	if (this->m_pBuffer != NULL)
	{
		memset(m_pBuffer, 0, this->m_nLength);
	}

	this->m_pCurrentWritePos = m_pBuffer;
	this->m_pCurrentReadPos = m_pBuffer;

	return 0;
}

/*
 int CJSByteArray::Destroy()
 {
 delete this;
 return 0;
 }
 */

int CJSByteArray::GetValidSize(void)
{
	return m_nBufferSize - m_nLength;
}

bool CJSByteArray::EnoughGet(int size)
{
	int valid = m_nLength - (m_pCurrentReadPos - m_pBuffer);
	if (valid >= size)
	{
		return true;
	}

	return false;
}

int CJSByteArray::ModeWritePos(int pos)
{
    if (pos < 0 || pos > m_nBufferSize)
    {
        return -1;
    }

    m_pCurrentWritePos = m_pBuffer + pos;

    return 0;
}

int CJSByteArray::End(void)
{
	//如果整个数组的长度小于8，则不能取L的空间
	if (m_nBufferSize < 8)
	{
		return -1;
	}

	int *p = (int *) (m_pBuffer + sizeof(int));

	//V的长度 = 整个数据包的长度-TL的长度
	*p = htonl(m_nLength - 2* sizeof (int));

	return 0;
}

int CJSByteArray::GetT(void)
{
	if (m_pBuffer == NULL)
	{
		return -1;
	}

	int *p = (int*) m_pBuffer;

	return ntohl(*p);
}

int CJSByteArray::GetL(void)
{
	if (m_pBuffer == NULL)
	{
		return -1;
	}

	int *p = (int*) (m_pBuffer + 4);

	return ntohl(*p);
}

void CJSByteArray::UnpackBegin(void)
{
	m_pCurrentReadPos = m_pBuffer+2*sizeof(int);
}

