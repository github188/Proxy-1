#include "CodeConvert.h"

unsigned short CCodeConvert::ConvertUnicode(unsigned short a)
{
    unsigned short table[72][2] = {{0xA7D1,0x5B66},{0xDBB8,0x6E2F},{0xC4B0,0x6FB3},
        {0xDFB1,0x8FB9},{0xFBCF,0x6D88},{0xF0BD,0x91D1},
        {0xE7B5,0x7535},{0xA8CD,0x901A},{0xA8B4,0x5DDD},
        {0xE5D3,0x6E1D},{0xE6CF,0x6E58},{0xF0B9,0x6842},
        {0xD8B2,0x85CF},{0xC1D4,0x7CA4/*粤*/},{0xCAB8,0x7518},
        {0xF3B9,0x8D35},{0xDABA,0x9ED1},{0xBDBC,0x5180},
        {0xEDC7,0x743C},{0xB3C2,0x9C81},{0xF2BD,0x6D25},
        {0xD5CB,0x82CF},{0xC6D4,0x4E91},{0xA5D4,0x8C6B},
        {0xC9C1,0x8FBD},{0xAABC,0x5409},{0xA6BB,0x6CAA},
        {0xA9BE,0x4EAC},{0xFABD,0x664B},{0xC9C3,0x8499},
        {0xE3D5,0x6D59},{0xEECD,0x7696},{0xF6C3,0x95FD},
        {0xD3B8,0x8D63},{0xF5B6,0x9102},{0xC2C9,0x9655},
        {0xE0C7,0x9752},{0xFEC4,0x5B81},{0xC2D0,0x65B0},
        {0xAFBE,0x8B66/*警*/},{0xD7BC,0x7532},{0xD2D2,0x4E59},
        {0xFBB1,0x4E19},{0xA1B6,0x4E01},{0xECCE,0x620A},
        {0xBABC,0x5DF1},{0xFDB8,0x5E9A},{0xC1D0,0x8F9B},
        {0xC9C8,0x58EC},{0xEFB9,0x7678},{0xD3D7,0x5B50},
        {0xF3B3,0x4E11},{0xFAD2,0x5BC5},{0xAEC3,0x536F},
        {0xBDB3,0x8FB0},{0xC8CB,0x5DF3},{0xE7CE,0x5348},
        {0xB4CE,0x672A},{0xEAC9,0x7533},{0xCFD3,0x9149},
        {0xE7D0,0x620C},{0xA5BA,0x4EA5},{0xFCBE,0x519B/*军*/},
        {0xA3BA,0x6D77/*海*/},{0xD5BF,0x7A7A/*空*/},{0xB1B1,0x5317/*北*/},
        {0xF2C9,0x6C88/*沈*/},{0xBCC0,0x5170/*兰*/},{0xC3BC,0x6D4E/*济*/},
        {0xCFC4,0x5357/*南*/},{0xE3B9,0x5E7F/*广*/},{0xC9B3,0x6210/*成*/}};

	for(int i=0; i<72; i++)
	{
		if(a == table[i][0])
			return table[i][1];

	}
	return (unsigned short)'?';
}


size_t CCodeConvert::GetLastPos(const string& str, const string& strFind)
{
	if (str.empty() || strFind.empty())
	{
		return 0;
	}

	size_t found = 0;
	size_t lastFound = 0;
	found = str.find(strFind);
	if (found != string::npos)
	{
		lastFound = found;
	}

	while (found != string::npos)
	{
		found = str.find(strFind, found+strFind.length());
		//cout<<"found= "<<found<<endl;
		if (found != string::npos)
		{
			lastFound = found;
		}
	}

	return lastFound;
}

string CCodeConvert::UnicodetoUTF8(const string& str)
{
	string strUTF8 = str;
	string strTmpUTF8;
	size_t bfound = 0;
	size_t efound = 0;
	bfound = strUTF8.find("&#");
	if (bfound == string::npos)
	{
		return strUTF8;
	}

	while (bfound!=string::npos && efound!=string::npos)
	{
		efound = strUTF8.find(";", bfound);
		if (efound != string::npos)
		{
			//取得最后一个&#的位置
			bfound += GetLastPos(strUTF8.substr(bfound, efound-bfound), "&#");
			
			strTmpUTF8.clear();
			UnicodetoUTF8(strUTF8.substr(bfound, efound-bfound+1), strTmpUTF8);
			
			strUTF8.replace(bfound, efound-bfound+1, strTmpUTF8);
			//cout<<"replace = "<<strUTF8<<endl;
			bfound = strUTF8.find("&#", bfound + strTmpUTF8.length()-1);
			if (bfound == string::npos)
			{
				break;
			}
			
		}

	}

	return strUTF8;
}


//传入的plate为图像检测到的车牌号码
void CCodeConvert::CarPlatetoUnicode(const char* plate, string& strUnicodeCarPlate)
{
	strUnicodeCarPlate.clear();

	unsigned long nUnicode = 0;
	char temp[32];

	//int np = 0;
	for(unsigned int i=0; i<strlen(plate); i++)
	{
		if((plate[i]&0x80) == 0)
		{
			nUnicode = (unsigned short)(*(plate+i));
			//cout<<"i = "<<i<<" "<<nUnicode<<endl;
		}
		else
		{
			nUnicode = ConvertUnicode(*(unsigned short*)(plate+i));
			//cout<<"*(short*)(plate+"<<i<<")= "<<nUnicode<<endl;
			i++;
		}
		
		//printf("-----------------------------------%04X\n", nUnicode);
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%lu", nUnicode);
		strUnicodeCarPlate += "&#";
		strUnicodeCarPlate += temp;
		strUnicodeCarPlate += ";";
		//np++;
		//if(np > 9) break;
	}

	//cout<<"strUnicodeCarPlate= "<<strUnicodeCarPlate<<endl;
}


void CCodeConvert::CarPlatetoUnicode(const char* plate, vector<wchar_t>& wUnicode)
{
    wUnicode.clear();

    unsigned long nUnicode = 0;

    //int np = 0;
    for (unsigned int i = 0; i < strlen(plate); i++)
    {
        if ((plate[i] & 0x80) == 0)
        {
            nUnicode = (unsigned short) (*(plate + i));
            //cout<<"i = "<<i<<" "<<nUnicode<<endl;
        }
        else
        {
            nUnicode = ConvertUnicode(*(unsigned short*) (plate + i));
            //cout<<"*(short*)(plate+"<<i<<")= "<<nUnicode<<endl;
            i++;
        }

        wUnicode.push_back(nUnicode);
    }
}


//strUnicode format must be &#1234;&#1234; etc...
//没有对传入的strUnicode格式进行正确性检测
void CCodeConvert::UnicodetoUTF8(const string& strUnicode, string& strUTF8)
{
	strUTF8.clear();

	string strTemp = strUnicode;
	size_t found  = 0;
	char utf8[6];
	memset(utf8, 0, sizeof(utf8));

	while (strTemp.length())
	{
		found = strTemp.find(';');
		if (found != string::npos)
		{
			memset(utf8, 0, sizeof(utf8));
			UnicodetoUTF8(atoi(strTemp.substr(2, found-2).c_str()), utf8);
			strUTF8 += utf8;

			strTemp.erase(0, found+1);
		}
	}

	
}



//********************************************************
//Description:
//	根据utf8的头取得本次由几个字节构成一个完整的UTF字符
//Return:
//	0 为不正确的UTF8头
//	>0 字节数
//********************************************************
int CCodeConvert::GetUTF8ByteCount(const char& head)
{
	//0XXXXXXX
	if ((head&0x80) == 0)
	{
		return 1;
	}

	//10XXXXXX
	if ((head&0x40) == 0)
	{
		return 0;
	}

	int count = 2;
	int zeroFlag = 0x20;
	for(int i=3; i<=7; i++)
	{
		if ((head&zeroFlag) == 0)
		{
			return count;
		}

		zeroFlag = zeroFlag>>1;
		count++;
	}

	return 0;
}

//***************************************************
//Parameter:
//	strUTF8		[in] :UTF8字符(包括ASCII码部分)
//	strUnicode	[out]:Unicode字符,形式：&#(unsigned int);
//Return:
//	0 :完全转换成功
//	-1:传入的字符串中有部分非UTF8字符
//***************************************************
int CCodeConvert::UTF8toUnicode(const string& strUTF8, string& strUnicode)
{
	if (strUTF8.empty())
	{
		return 0;
	}

	bool bHasInValidCode = false;
	strUnicode.clear();

	string strTempUTF8 = strUTF8;
	
	for (unsigned int i=0; i<strTempUTF8.length(); i++)
	{
		int byteCount = GetUTF8ByteCount(strTempUTF8[i]);
		if (byteCount <= 0)
		{
			bHasInValidCode = true;
			continue;
		}

		int unicode = 0;

		//先取得头数据，再左移(byteCount-1)*6位
		unicode = (strTempUTF8[i]&(0xFF>>byteCount))<<((byteCount-1)*6);
		
		//用于标识本次utf8向unicode的转换是否有问题
		bool bFinish = true;	
		//取得其它位
		for (int count=1; count<byteCount; count++)
		{
			i++;
			//必须为10XXXXXX
			if ((strTempUTF8[i]&0x80)!=0 && (strTempUTF8[i]&0x40)==0)
			{
				unicode |= (strTempUTF8[i]&0x3F)<<((byteCount-1-count)*6);
			}
			else
			{
				bHasInValidCode = true;
				bFinish = false;
				break;
			}
		}

		if (bFinish)
		{
			char temp[256];
			memset(temp, 0, sizeof(temp));
			sprintf(temp, "%d", unicode);

			strUnicode += "&#";
			strUnicode += temp;
			strUnicode += ";";

			//cout<<"strUnicode = "<<strUnicode<<endl;
		}
	}

	return bHasInValidCode?-1:0;
}


int CCodeConvert::UTF8toUnicode(const string& strUTF8, wchar_t wUnicode[])
{
    if (strUTF8.empty() || wUnicode == NULL)
    {
        return 0;
    }

    bool bHasInValidCode = false;

    string strTempUTF8 = strUTF8;

    int index = 0;

    for (unsigned int i=0; i<strTempUTF8.length(); i++)
    {
        int byteCount = GetUTF8ByteCount(strTempUTF8[i]);
        if (byteCount <= 0)
        {
            bHasInValidCode = true;
            continue;
        }

        int unicode = 0;

        //先取得头数据，再左移(byteCount-1)*6位
        unicode = (strTempUTF8[i]&(0xFF>>byteCount))<<((byteCount-1)*6);

        //用于标识本次utf8向unicode的转换是否有问题
        bool bFinish = true;
        //取得其它位
        for (int count=1; count<byteCount; count++)
        {
            i++;
            //必须为10XXXXXX
            if ((strTempUTF8[i]&0x80)!=0 && (strTempUTF8[i]&0x40)==0)
            {
                unicode |= (strTempUTF8[i]&0x3F)<<((byteCount-1-count)*6);
            }
            else
            {
                bHasInValidCode = true;
                bFinish = false;
                break;
            }
        }

        if (bFinish)
        {
            wUnicode[index] = unicode;
            index++;
        }
    }

    wUnicode[index] = '\0';

    return bHasInValidCode?-1:0;
}


int CCodeConvert::UTF8toUnicode(const string& strUTF8, vector<wchar_t>& wUnicode)
{
    wUnicode.clear();

    if (strUTF8.empty())
    {
        return 0;
    }

    bool bHasInValidCode = false;

    string strTempUTF8 = strUTF8;

    //int index = 0;

    for (unsigned int i=0; i<strTempUTF8.length(); i++)
    {
        int byteCount = GetUTF8ByteCount(strTempUTF8[i]);
        if (byteCount <= 0)
        {
            bHasInValidCode = true;
            continue;
        }

        int unicode = 0;

        //先取得头数据，再左移(byteCount-1)*6位
        unicode = (strTempUTF8[i]&(0xFF>>byteCount))<<((byteCount-1)*6);

        //用于标识本次utf8向unicode的转换是否有问题
        bool bFinish = true;
        //取得其它位
        for (int count=1; count<byteCount; count++)
        {
            i++;
            //必须为10XXXXXX
            if ((strTempUTF8[i]&0x80)!=0 && (strTempUTF8[i]&0x40)==0)
            {
                unicode |= (strTempUTF8[i]&0x3F)<<((byteCount-1-count)*6);
            }
            else
            {
                bHasInValidCode = true;
                bFinish = false;
                break;
            }
        }

        if (bFinish)
        {
            wUnicode.push_back(unicode);
            //index++;
        }
    }

    //wUnicode[index] = '\0';

    return bHasInValidCode?-1:0;
}


//**************************************************
//Parameter:
//	nUnicode	[in] :unicode编码
//	pUtf8		[out]:调用者分配空间，分配6个字节就足够了
//**************************************************
void CCodeConvert::UnicodetoUTF8(unsigned int nUnicode, char* pUtf8)
{
	if (pUtf8 == NULL)
	{
		return;
	}

	int count = 0;
	unsigned int head = 0x00;

	//确定字节数
	if (/*nUnicode>=0x00 && */nUnicode<0x80)
	{
		pUtf8[0] = nUnicode;
		count = 1;
		return;
	}
	else if(nUnicode>=0x80 && nUnicode<0x800)
	{
		head = 0xC0;
		count = 2;
	}
	else if(nUnicode>=0x800 && nUnicode<0x10000)
	{
		head = 0xE0;
		count = 3;
	}
	else if(nUnicode>=0x10000 && nUnicode<0x200000)
	{
		head = 0xF0;
		count = 4;
	}
	else if(nUnicode>=0x200000 && nUnicode<0x4000000)
	{
		head = 0xF8;
		count = 5;
	}
	else if(nUnicode>=0x4000000 && nUnicode<=0x7FFFFFFF)
	{
		head = 0xFC;
		count = 6;
	}else
	{
		return;
	}

	pUtf8[0] = head|(nUnicode>>((count-1)*6));
	//printf("pUtf8[0] = %d\n", (unsigned char)pUtf8[0]);

	int offset = 0;
	for (int byteCount=1; byteCount<count; byteCount++)
	{
		offset = (count-1-byteCount)*6;
		if (offset > 0)
		{
			pUtf8[byteCount] = 0x80|((nUnicode>>offset)&0x3F);
		}
		else
		{
			pUtf8[byteCount] = 0x80|(nUnicode&0x3F);
		}
		//printf("pUtf8[%d] = %d\n", byteCount, (unsigned char)pUtf8[byteCount]);
	}
}

