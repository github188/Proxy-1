#ifndef FILE_H
#define FILE_H

#include "dir.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
using namespace std;

#define MAXWRITESIZE 8096

class CFile
{
public:
	int WriteFile(const char* data, unsigned int length, const char *filename);
	int RemoveFile(const string &strFilename);

	int ClipFile(const string& strSrc, const string& strDest);
};

#endif //FILE_H

