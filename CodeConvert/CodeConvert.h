#ifndef CODECONVERT_H
#define CODECONVERT_H


#include <stdio.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

class CCodeConvert
{
public:
	CCodeConvert(){}
	~CCodeConvert(){}

	unsigned short ConvertUnicode(unsigned short a);

	void CarPlatetoUnicode(const char* plate, string& strUnicodeCarPlate);
	void CarPlatetoUnicode(const char* plate, vector<wchar_t>& wUnicode);
	
	int GetUTF8ByteCount(const char& head);
	int UTF8toUnicode(const string& strUTF8, string& strUnicode);
	int UTF8toUnicode(const string& strUTF8, wchar_t wUnicode[]);
	int UTF8toUnicode(const string& strUTF8, vector<wchar_t>& wUnicode);
	
	//这个函数传入的str可以只有部分是Unicode编码
	string UnicodetoUTF8(const string& str);

	//这个函数传入的strUnicodeCarPlate必须全部是Unicode编码
	void UnicodetoUTF8(const string& strUnicodeCarPlate, string& strUTF8CarPlate);
	void UnicodetoUTF8(unsigned int nUnicode, char* pUtf8);

	//查找stFind在str中出现的最后位置
	size_t GetLastPos(const string& str, const string& strFind);

};

#endif //CODECONVERT_H


