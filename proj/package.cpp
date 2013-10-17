/*************************************************************************
 * FILE            :  package.cpp
 * TASKS           :  1. 解析基于网络字节序的TLV数据包
 * 					  2. 组装基于网络字节序的TLV数据包
 *
 * PROJECT         :
 * MODULE NAME     :  数据包
 *
 * DATE            :  22,01,2011
 * COPYRIGHT (C)   :  Kise
 * AUTHOR          :  cchao
 *
 * ENVIRONMENT     :
 * MODULES CALLED  :
 * NOTES           :
 * CURRENT VERSION :  1.0
 * HISTORY :
 *   <author>        <time>       <version>        <desc>
 *  Chao Chen       22,01,2011      1.0
 **************************************************************************/

#include "package.h"
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <stdio.h>
using namespace std;

/*******************************************************************************
 * @Name: CTlvNetPackage(class CTlvNetPackage)
 *
 * @Purpose: 使用网络TLV数据创建一个只读网络字节序数据包
 * @Returns: 无
 * @Parameters: char* | tlvNetData[IN] 网络字节序TLV数据
 * @Remarks: 1. TLV即：type|length|data格式
 * 			 2. 对于只读类型数据包，Put系列的方法失效
 ********************************************************************************/
CTlvNetPackage::CTlvNetPackage(char* tlvNetData)
	: m_tlvData(tlvNetData)
	, m_size(0)
	, m_rdPos(2 * sizeof(int))
	, m_rwFlag(RW_FLAG_RONLY)
{
	int len;

	len = ntohl(*(int*)(m_tlvData + sizeof(int)));
	m_size = len + 2 * sizeof(int);
}

/*******************************************************************************
 * @Name: CTlvNetPackage(class CTlvNetPackage)
 *
 * @Purpose: 创建一个指定大小的TLV只写网络字节序数据包
 * @Returns: 无
 * @Parameters: int | type[IN] TLV数据中的type
 * 				int | length[IN] TLV灵气中的length
 * @Remarks: 1. type和length均为本机字节序
 * 			 2. 对于只写类型数据包，Get系列数据包失效
 ********************************************************************************/
CTlvNetPackage::CTlvNetPackage(int type, int length)
	: m_tlvData(NULL)
	, m_size(0)
	, m_wrPos(2 * sizeof(int))
	, m_rwFlag(RW_FLAG_WONLY)
{
	m_size = length + 2 * sizeof(int);
	m_tlvData = new char[m_size];
	memset(m_tlvData, 0, m_size);

	int* pType;
	int* pLength;

	pType = (int*)(m_tlvData);
	pLength = (int*)(m_tlvData + sizeof(int));

	*pType = htonl(type);
	*pLength = htonl(length);
}

/*******************************************************************************
 * @Name: ~CTlvNetPackage(class CTlvNetPackage)
 *
 * @Purpose: 资源清理
 * @Returns: 无
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
CTlvNetPackage::~CTlvNetPackage()
{
	if (m_rwFlag == RW_FLAG_WONLY)
	{
		delete[] m_tlvData;
	}
}

/*******************************************************************************
 * @Name: Size(class CTlvNetPackage)
 *
 * @Purpose: TLV数据的长度
 * @Returns:	int | 整个TLV数据的长度
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
int CTlvNetPackage::Size() const
{
	return m_size;
}

/*******************************************************************************
 * @Name: Type(class CTlvNetPackage)
 *
 * @Purpose: TLV数据中的type
 * @Returns: int | TLV数据中的type
 * @Parameters: 无
 * @Remarks: 返回值为本机字节序
 ********************************************************************************/
int CTlvNetPackage::Type() const
{
	return ntohl(*(int*)(m_tlvData));
}

/*******************************************************************************
 * @Name: Length(class CTlvNetPackage)
 *
 * @Purpose: 获得TLV数据中的L的值
 * @Returns: unsigned int | TLV数据中的L的值
 * @Parameters: 无
 * @Remarks: 返回值为本机字节序
 ********************************************************************************/
int CTlvNetPackage::Length() const
{
	return m_size - 2 * sizeof(int);	// 2 * sizof(int) == TLV(TL)
}

/*******************************************************************************
 * @Name: Data(class CTlvNetPackage)
 *
 * @Purpose: 返回TLV数据中的V的首地址
 * @Returns: char* | TLV中V数据的首地址
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
char* CTlvNetPackage::Data() const
{
	if (m_size <= sizeof(int) * 2)
	{
		return NULL;
	}

	return m_tlvData + 2 * sizeof(int);
}

/*******************************************************************************
 * @Name: TlvData(class CTlvNetPackage)
 *
 * @Purpose: 返回整个TLV数据的首地址
 * @Returns: char* | TLV数据的首地址
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
char* CTlvNetPackage::TlvData() const
{
	return m_tlvData;
}

/*******************************************************************************
 * @Name: SeekToBegin(class CTlvNetPackage)
 *
 * @Purpose: 将当前读写指针重新定位到TLV数据中V的首地址
 * @Returns: void
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
void CTlvNetPackage::SeekToBegin()
{
	m_rdPos = 2 * sizeof(int);
}

/*******************************************************************************
 * @Name: GetByte(class CTlvNetPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个字节的数据
 * @Returns: char | 取得的字节值
 * @Parameters: bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
char CTlvNetPackage::GetByte(bool* ok)
{
	if (m_rdPos + sizeof(char) > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}
		return -1;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		char byte;

		byte = *(m_tlvData + m_rdPos);
		m_rdPos++;

		if (ok != NULL)
		{
			*ok = true;
		}

		return byte;
	}

	if (ok != NULL)
	{
		*ok = false;
	}
	return -1;
}

short CTlvNetPackage::GetShort(bool* ok)
{
	if (m_rdPos + sizeof(short) > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}

		return -1;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		short integer;

		integer = *(short*)(m_tlvData + m_rdPos);
		integer = ntohs(integer);

		m_rdPos += sizeof(short);

		if (ok != NULL)
		{
			*ok = true;
		}

		return integer;
	}

	if (ok != NULL)
	{
		*ok = false;
	}

	return -1;
}

/*******************************************************************************
 * @Name: GetInt(class CTlvNetPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个int类型的数据
 * @Returns: int | 取得的int值
 * @Parameters: bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
int CTlvNetPackage::GetInt(bool* ok)
{
	if (m_rdPos + sizeof(int) > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}

		return -1;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		int integer;

		integer = *(int*)(m_tlvData + m_rdPos);
		integer = ntohl(integer);

		m_rdPos += sizeof(int);

		if (ok != NULL)
		{
			*ok = true;
		}

		return integer;
	}

	if (ok != NULL)
	{
		*ok = false;
	}

	return -1;
}

/*******************************************************************************
 * @Name: GetLong(class CTlvNetPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个long类型的数据
 * @Returns: long | 取得的long类型值
 * @Parameters: bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
long CTlvNetPackage::GetLong(bool* ok)
{
	if (m_rdPos + sizeof(long) > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}

		return -1;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		long integer;

		integer = *(long*)(m_tlvData + m_rdPos);
		integer = ntohl(integer);

		m_rdPos += sizeof(long);

		if (ok != NULL)
		{
			*ok = true;
		}

		return integer;
	}

	if (ok != NULL)
	{
		*ok = false;
	}

	return -1;
}

/*******************************************************************************
 * @Name: GetBytes(class CTlvNetPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个指定长度的数据
 * @Returns: char* | 取得指定长度的字节序列的首地址
 * @Parameters: int | size[IN] 字节数
 * 				bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
char* CTlvNetPackage::GetBytes(int size, bool* ok)
{
	if (m_rdPos + size > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}

		return NULL;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		char* bytes;

		bytes = m_tlvData + m_rdPos;

		m_rdPos += size;

		if (ok != NULL)
		{
			*ok = true;
		}

		return bytes;
	}

	if (ok != NULL)
	{
		*ok = false;
	}

	return NULL;
}

/*******************************************************************************
 * @Name: PutByte(class CTlvNetPackage)
 *
 * @Purpose: 向数据包添加一个字节数据
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: char | byte[IN] 字节数据
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 ********************************************************************************/
bool CTlvNetPackage::PutByte(char byte)
{
	if (m_wrPos + sizeof(char) > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		*(m_tlvData + m_wrPos) = byte;
		m_wrPos++;

		return true;
	}

	return false;
}

bool CTlvNetPackage::PutShort(short integer)
{
	if (m_wrPos + sizeof(short) > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		short* p;

		p = (short*)(m_tlvData + m_wrPos);
		*p = htons(integer);
		m_wrPos += sizeof(short);

		return true;
	}
}

/*******************************************************************************
 * @Name: PutInt(class CTlvNetPackage)
 *
 * @Purpose: 向数据包添加一int类型数据
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: int | integer[IN] int数据
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 ********************************************************************************/
bool CTlvNetPackage::PutInt(int integer)
{
	if (m_wrPos + sizeof(int) > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		int* p;

		p = (int*)(m_tlvData + m_wrPos);
		*p = htonl(integer);
		m_wrPos += sizeof(int);

		return true;
	}

	return false;
}

/*******************************************************************************
 * @Name: PutLong(class CTlvNetPackage)
 *
 * @Purpose: 向数据包添加一long类型数据
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: long | integer[IN] long数据
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 ********************************************************************************/
bool CTlvNetPackage::PutLong(long integer)
{
	if (m_wrPos + sizeof(long) > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		long* p;

		p = (long*)(m_tlvData + m_wrPos);
		*p = htonl(integer);
		m_wrPos += sizeof(long);

		return true;
	}

	return false;
}

/*******************************************************************************
 * @Name: PutBytes(class CTlvNetPackage)
 *
 * @Purpose: 向数据包添加多个字节
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: char* | bytes[IN] 添加进数据包的数据
 * 				int | size[IN] 添加数据的字节数
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 * 			 3. 只适用于不需要字节序转换的数据
 ********************************************************************************/
bool CTlvNetPackage::PutBytes(char* bytes, int size)
{
	if (m_wrPos + size > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		char* p;

		p = m_tlvData + m_wrPos;
		memcpy(p, bytes, size);
		m_wrPos += size;

		return true;
	}

	return false;
}


/*******************************************************************************
 * @Name: CTlvLEPackage(class CTlvLEPackage)
 *
 * @Purpose: 使用网络TLV数据创建一个只读小头数据包
 * @Returns: 无
 * @Parameters: char* | tlvLEData[IN] 小头字节序TLV数据
 * @Remarks: 1. TLV即：type|length|data格式
 * 			 2. 对于只读类型数据包，Put系列的方法失效
 ********************************************************************************/
CTlvLEPackage::CTlvLEPackage(char* tlvLEData)
	: m_tlvData(tlvLEData)
	, m_size(0)
	, m_rdPos(2 * sizeof(int))
	, m_rwFlag(RW_FLAG_RONLY)
{
	int len;

	len = LEToHost((*(int*)(m_tlvData + sizeof(int))));
	m_size = len + 2 * sizeof(int);
}

/*******************************************************************************
 * @Name: CTlvLEPackage(class CTlvLEPackage)
 *
 * @Purpose: 创建一个指定大小的TLV只写小头字节序数据包
 * @Returns: 无
 * @Parameters: int | type[IN] TLV数据中的type
 * 				int | length[IN] TLV灵气中的length
 * @Remarks: 1. type和length均为本机字节序
 * 			 2. 对于只写类型数据包，Get系列数据包失效
 ********************************************************************************/
CTlvLEPackage::CTlvLEPackage(int type, int length)
	: m_tlvData(NULL)
	, m_size(0)
	, m_wrPos(2 * sizeof(int))
	, m_rwFlag(RW_FLAG_WONLY)
{
	m_size = length + 2 * sizeof(int);
	m_tlvData = new char[m_size];
	memset(m_tlvData, 0, m_size);

	int* pType;
	int* pLength;

	pType = (int*)(m_tlvData);
	pLength = (int*)(m_tlvData + sizeof(int));

	*pType = HostToLE(type);
	*pLength = HostToLE(length);
}

/*******************************************************************************
 * @Name: ~CTlvLEPackage(class CTlvLEPackage)
 *
 * @Purpose: 资源清理
 * @Returns: 无
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
CTlvLEPackage::~CTlvLEPackage()
{
	if (m_rwFlag == RW_FLAG_WONLY)
	{
		delete[] m_tlvData;
	}
}

/*******************************************************************************
 * @Name: Size(class CTlvLEPackage)
 *
 * @Purpose: TLV数据的长度
 * @Returns:	int | 整个TLV数据的长度
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
int CTlvLEPackage::Size() const
{
	return m_size;
}

/*******************************************************************************
 * @Name: Type(class CTlvLEPackage)
 *
 * @Purpose: TLV数据中的type
 * @Returns: int | TLV数据中的type
 * @Parameters: 无
 * @Remarks: 返回值为本机字节序
 ********************************************************************************/
int CTlvLEPackage::Type() const
{
	return LEToHost(*(int*)(m_tlvData));
}

/*******************************************************************************
 * @Name: Length(class CTlvLEPackage)
 *
 * @Purpose: 获得TLV数据中的L的值
 * @Returns: unsigned int | TLV数据中的L的值
 * @Parameters: 无
 * @Remarks: 返回值为本机字节序
 ********************************************************************************/
int CTlvLEPackage::Length() const
{
	return m_size - 2 * sizeof(int);	// 2 * sizof(int) == TLV(TL)
}

/*******************************************************************************
 * @Name: Data(class CTlvLEPackage)
 *
 * @Purpose: 返回TLV数据中的V的首地址
 * @Returns: char* | TLV中V数据的首地址
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
char* CTlvLEPackage::Data() const
{
	if (m_size <= sizeof(int) * 2)
	{
		return NULL;
	}

	return m_tlvData + 2 * sizeof(int);
}

/*******************************************************************************
 * @Name: TlvData(class CTlvLEPackage)
 *
 * @Purpose: 返回整个TLV数据的首地址
 * @Returns: char* | TLV数据的首地址
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
char* CTlvLEPackage::TlvData() const
{
	return m_tlvData;
}

/*******************************************************************************
 * @Name: SeekToBegin(class CTlvLEPackage)
 *
 * @Purpose: 将当前读写指针重新定位到TLV数据中V的首地址
 * @Returns: void
 * @Parameters: 无
 * @Remarks: 无
 ********************************************************************************/
void CTlvLEPackage::SeekToBegin()
{
	m_rdPos = 2 * sizeof(int);
}

/*******************************************************************************
 * @Name: GetByte(class CTlvLEPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个字节的数据
 * @Returns: char | 取得的字节值
 * @Parameters: bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
char CTlvLEPackage::GetByte(bool* ok)
{
	if (m_rdPos + sizeof(char) > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}
		return -1;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		char byte;

		byte = *(m_tlvData + m_rdPos);
		m_rdPos++;

		if (ok != NULL)
		{
			*ok = true;
		}

		return byte;
	}

	if (ok != NULL)
	{
		*ok = false;
	}
	return -1;
}

/*******************************************************************************
 * @Name: GetInt(class CTlvLEPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个int类型的数据
 * @Returns: int | 取得的int值
 * @Parameters: bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
int CTlvLEPackage::GetInt(bool* ok)
{
	if (m_rdPos + sizeof(int) > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}

		return -1;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		int integer;

		integer = *(int*)(m_tlvData + m_rdPos);
		integer = LEToHost(integer);

		m_rdPos += sizeof(int);

		if (ok != NULL)
		{
			*ok = true;
		}

		return integer;
	}

	if (ok != NULL)
	{
		*ok = false;
	}

	return -1;
}

/*******************************************************************************
 * @Name: GetBytes(class CTlvLEPackage)
 *
 * @Purpose: 从TLV数据中的V取得一个指定长度的数据
 * @Returns: char* | 取得指定长度的字节序列的首地址
 * @Parameters: int | size[IN] 字节数
 * 				bool* | ok[OUT] 是否成功获得数据
 * @Remarks: 1. 对于只写类型数据包，该方法只返回false
 * 			 2. 对于没有足够数据获取的情况，返回值为false
 ********************************************************************************/
char* CTlvLEPackage::GetBytes(int size, bool* ok)
{
	if (m_rdPos + size > m_size)
	{
		if (ok != NULL)
		{
			*ok = false;
		}

		return NULL;
	}

	if (m_rwFlag == RW_FLAG_RONLY)
	{
		char* bytes;

		bytes = m_tlvData + m_rdPos;

		m_rdPos += size;

		if (ok != NULL)
		{
			*ok = true;
		}

		return bytes;
	}

	if (ok != NULL)
	{
		*ok = false;
	}

	return NULL;
}

/*******************************************************************************
 * @Name: PutByte(class CTlvLEPackage)
 *
 * @Purpose: 向数据包添加一个字节数据
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: char | byte[IN] 字节数据
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 ********************************************************************************/
bool CTlvLEPackage::PutByte(char byte)
{
	if (m_wrPos + sizeof(char) > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		*(m_tlvData + m_wrPos) = byte;
		m_wrPos++;

		return true;
	}

	return false;
}

/*******************************************************************************
 * @Name: PutInt(class CTlvLEPackage)
 *
 * @Purpose: 向数据包添加一int类型数据
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: int | integer[IN] int数据
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 ********************************************************************************/
bool CTlvLEPackage::PutInt(int integer)
{
	if (m_wrPos + sizeof(int) > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		int* p;

		p = (int*)(m_tlvData + m_wrPos);
		*p = HostToLE(integer);
		m_wrPos += sizeof(int);

		return true;
	}

	return false;
}

/*******************************************************************************
 * @Name: PutBytes(class CTlvLEPackage)
 *
 * @Purpose: 向数据包添加多个字节
 * @Returns: bool | 是否成功向数据包放入数据
 * @Parameters: char* | bytes[IN] 添加进数据包的数据
 * 				int | size[IN] 添加数据的字节数
 * @Remarks: 1. 对于只读类型数据包，该方法只返回false
 * 			 2. 对于没有足够空间存储放入数据的情况，返回值为false
 * 			 3. 只适用于不需要字节序转换的数据
 ********************************************************************************/
bool CTlvLEPackage::PutBytes(char* bytes, int size)
{
	if (m_wrPos + size > m_size)
	{
		return false;
	}

	if (m_rwFlag == RW_FLAG_WONLY)
	{
		char* p;

		p = m_tlvData + m_wrPos;
		memcpy(p, bytes, size);
		m_wrPos += size;

		return true;
	}

	return false;
}

/*******************************************************************************
 * @Name: HostToLE(class CTlvLEPackage)
 *
 * @Purpose: 将本机字节序整数转换为小头字节序
 * @Returns: int | 小头字节顺序表示的整数
 * @Parameters: int | num[IN] 本机字节序整数
 * @Remarks: 无
 ********************************************************************************/
int CTlvLEPackage::HostToLE(int num)
{
	static unsigned short a = 0xcc;
	static unsigned char ch = (unsigned char)a;
	static bool bLittleEndian = ((ch | a) == 0xcc);

	if (bLittleEndian)
	{
		return num;
	}

	return ConvertByteOrder(num);
}

/*******************************************************************************
 * @Name: LEToHost(class CTlvLEPackage)
 *
 * @Purpose: 将小头字节序整数转换为本机字节序
 * @Returns: int | 小头字节顺序表示的整数
 * @Parameters: int | num[IN] 小头字节序整数
 * @Remarks: 无
 ********************************************************************************/
int CTlvLEPackage::LEToHost(int num)
{
	static unsigned short a = 0xcc;
	static unsigned char ch = (unsigned char)a;
	static bool bLittleEndian = ((ch | a) == 0xcc);

	if (bLittleEndian)
	{
		return num;
	}

	return ConvertByteOrder(num);
}

/*******************************************************************************
 * @Name: ConvertByteOrder(class CTlvLEPackage)
 *
 * @Purpose: 改变整数的字节序
 * @Returns: int | 改变字节序后的整数
 * @Parameters: int | num[IN] 需要改变字节序的整数
 * @Remarks: 大头字节序整数会被转换为小头，反之亦然
 ********************************************************************************/
int CTlvLEPackage::ConvertByteOrder(int num)
{
	char data[4] = {0};

	for (int i = sizeof(int) - 1; i >= 0; i--)
	{
		data[i] = num & 0xff;
		num = num >> 8;

		printf("data[%d]\n", data[i]);
	}

	return *(int*)data;
}

