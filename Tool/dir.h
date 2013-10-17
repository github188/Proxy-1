#ifndef DIR_H
#define DIR_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> //mkdir声明
#include <dirent.h>
#include <string>
#include <list>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

class CDir
{
public:
    //只能创建一级目录
    int MakeDir(const string &strDir);
    //可以创建多级目录,bForce只能为true,false不起作用
    int MakeDir(const string &strDir, bool bForce);

    //只能删除一级目录
    int RemoveDir(const string &strDir);
    //可以删除多级目录,，包括里面的文件bForce只能为true,false不起作用
    int RemoveDir(const string &strDir, bool bForce);

    int RemoveFiles(list<string> &filesList);

    void SplitDir(const string &strDir, list<string> &dirs);

    //只能读取一级目录中的所有普通文件及目录，不能递归！返回的list包含路径信息
    bool ReadDir(const string &strDir, list<string> &filesList, list<string> &dirsList);

    //只能读取一级目录中的所有普通文件及目录，不能递归！返回的list包含路径信息
    int ReadDirByAlphasort(const string &strDir, list<string> &filenameList, list<string> &dirList);

    int ReadDirByVersionsort(const string &strDir, list<string> &filenameList, list<string> &dirList);

private:
    void PrintStringList(const list<string> &stringList);

};

#endif

