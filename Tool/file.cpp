#include "file.h"

int CFile::WriteFile(const char* data, unsigned int length, const char *filename)
{
	string strDir(filename);
	size_t found = strDir.find_last_of("/");
	if (found != string::npos)
	{
		strDir = strDir.substr(0, found);
		CDir dir;
		dir.MakeDir(strDir, true);
	}
	
	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
	{
		return -1;
	}

	//printf("WriteFile: %s, fd=%d\n", filename, fp);
	

	unsigned int finishoffset = 0;
	int wlen = 0;
	int size = 0;
	while ( finishoffset != length ) 
	{
		size = length-finishoffset<MAXWRITESIZE?length-finishoffset:MAXWRITESIZE;
		wlen = fwrite(data + finishoffset, size, 1, fp);
		//printf("wlen=%d, size=%d\n", wlen, size);
		if (wlen < 0)
		{
			return -1;
		}

		finishoffset += wlen*size;
	}
	fclose(fp);

	return 0;
}

int CFile::RemoveFile(const string &strFilename)
{
	int nRet = 0;
	if (!strFilename.empty())
	{
		nRet = remove(strFilename.c_str());
		if (nRet == -1)
		{
			printf("remove %s\n", strFilename.c_str());
			perror("remove");
		}
	}

	return nRet;
}

int CFile::ClipFile(const string& strSrc, const string& strDest)
{
	//file exist or not
	int nRet = access(strSrc.c_str(), F_OK);
	if (nRet != 0)
	{
		//cout<<"file not exist!!!"<<endl;
		return -1;
	}

	size_t found = strDest.find_last_of("/");
	if (found != string::npos)
	{
		CDir dir;
		dir.MakeDir(strDest.substr(0, found), true);
	}

	nRet = rename(strSrc.c_str(), strDest.c_str());
	if (nRet != 0)
	{
	    perror("ClipFile rename:");
	}
	return nRet;
}

