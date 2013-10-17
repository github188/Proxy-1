/*
 * package.h
 *
 *  Created on: 2011-1-22
 *      Author: cchao
 */

#ifndef PACKAGE_H_
#define PACKAGE_H_

class CTlvNetPackage
{
public:
	enum RW_FLAG { RW_FLAG_RONLY, RW_FLAG_WONLY};
public:
	CTlvNetPackage(char* tlvNetData);
	CTlvNetPackage(int type, int length);
	~CTlvNetPackage();
	int Size() const;
	int Type() const;
	int Length() const;
	char* Data() const;
	char* TlvData() const;
	void SeekToBegin();
	char GetByte(bool* ok = 0);
	short GetShort(bool* ok = 0);
	int GetInt(bool* ok = 0);
	long GetLong(bool* ok = 0);
	char* GetBytes(int size, bool* ok = 0);
	bool PutByte(char byte);
	bool PutShort(short integer);
	bool PutInt(int integer);
	bool PutLong(long integer);
	bool PutBytes(char* bytes, int size);
private:
	char* m_tlvData;
	int m_size;
	union
	{
		int m_rdPos;
		int m_wrPos;
	};
	RW_FLAG m_rwFlag;
};

class CTlvLEPackage
{
public:
	enum RW_FLAG { RW_FLAG_RONLY, RW_FLAG_WONLY};
public:
	CTlvLEPackage(char* tlvNetData);
	CTlvLEPackage(int type, int length);
	~CTlvLEPackage();
	int Size() const;
	int Type() const;
	int Length() const;
	char* Data() const;
	char* TlvData() const;
	void SeekToBegin();
	char GetByte(bool* ok = 0);
	int GetInt(bool* ok = 0);
	char* GetBytes(int size, bool* ok = 0);
	bool PutByte(char byte);
	bool PutInt(int integer);
	bool PutBytes(char* bytes, int size);
public:
	static int HostToLE(int num);
	static int LEToHost(int num);
private:
	static int ConvertByteOrder(int num);
private:
	char* m_tlvData;
	int m_size;
	union
	{
		int m_rdPos;
		int m_wrPos;
	};
	RW_FLAG m_rwFlag;
};

#endif /* PACKAGE_H_ */
