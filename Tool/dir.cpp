#include "dir.h"

//返回值：0成功 -1失败
int CDir::MakeDir(const string &strDir)
{
    if (strDir.empty())
    {
        return -1;
    }

    return mkdir(strDir.c_str(), S_IRWXU);
}

//可以创建多级目录,bForce只能为true,false不起作用
int CDir::MakeDir(const string &strDir, bool bForce)
{
    if (strDir.empty())
    {
        return -1;
    }

    int nRet = 0;

    if (bForce)
    {
        string dir;
        list<string> dirs;
        SplitDir(strDir, dirs);
        while (!dirs.empty())
        {
            dir = dirs.front();
            dirs.pop_front();
            
            if (access(dir.c_str(), F_OK) != 0)
            {
                nRet = MakeDir(dir);
                if (nRet == -1)
                {
                    break;
                }
            }
        }
    }

    return nRet;
}

//只有当目录为空的时候才能删除，实际上也能删除文件
//返回值：0成功 -1失败
int CDir::RemoveDir(const string &strDir)
{
    if (strDir.empty())
    {
        return -1;
    }

    return remove(strDir.c_str());
}

int CDir::RemoveFiles(list<string> &filesList)
{
    int nRet = 0;

    while (!filesList.empty())
    {
        string filename = filesList.front();
        filesList.pop_front();

        if (!filename.empty())
        {
            nRet = remove(filename.c_str());
            if (nRet == -1)
            {
                break;
            }
        }
    }

    return nRet;
}

int CDir::RemoveDir(const string &strDir, bool bForce)
{
    if (strDir.empty())
    {
        return -1;
    }

    int nRet = 0;
    if (bForce)
    {
        list<string> filesList;
        list<string> dirsList;
        list<string> topDirsList;       //栈顶结点
        list<string> tmpTopDirsList;    //临时保存栈顶结点
        stack< list<string> > dirsStack;//<>中一定得留空格，否则编译会出错.每一个栈元素是一个目录链表

        if (ReadDir(strDir, filesList, dirsList))
        {
            RemoveFiles(filesList);
            if (!dirsList.empty())
            {
                dirsStack.push(dirsList);
            }
        }
        while (!dirsStack.empty())
        {
            //弹出栈顶元素
            topDirsList = dirsStack.top();
            dirsStack.pop();

            //保存栈顶元素，以备在它的子目录中还找到下一级子目录时重新压栈
            tmpTopDirsList = topDirsList;
            //重新压栈时，不能重复压，所以设置标志位
            bool bHasRepush = false;
            
            //遍历栈顶元素的所有目录
            while (!topDirsList.empty())
            {
                string dir = topDirsList.front();
                topDirsList.pop_front();

                if (!dir.empty())
                {   
                    //查找dir目录下是否还有子目录
                    if (ReadDir(dir, filesList, dirsList))
                    {
                        RemoveFiles(filesList);
               
                        if (!dirsList.empty()) //还有子目录
                        {
                            if (!bHasRepush)
                            {
                                dirsStack.push(tmpTopDirsList);
                                bHasRepush = true;
                            }
                            dirsStack.push(dirsList);
                        }
                        else                   //没有子目录，则删除它
                        {
                            RemoveDir(dir);
                        }
                    }
                }
            }
        }

        //最后删除目录本身
        RemoveDir(strDir);
    }
    return nRet;
}

void CDir::SplitDir(const string &strDir, list<string> &dirs)
{
	if (strDir.empty())
	{
		return;
	}

	dirs.clear();

	size_t found;

	found = strDir.find_first_not_of("../");
	if (found == string::npos)
	{
		return;
	}

	found = strDir.find_first_of("/\\", found);
	if (found != string::npos)
	{
		if (!strDir.substr(0, found).empty())
		{
			dirs.push_back(strDir.substr(0, found));
		}
	}

	size_t lastfound = found;
	while (found != string::npos)
	{
		found = strDir.find_first_of("/\\", found+1);
		if (found != string::npos)
		{
			dirs.push_back(strDir.substr(0, found));
			lastfound = found;
		}
	}

	if (lastfound != strDir.length()-1)
	{
		dirs.push_back(strDir);
	}
}


//只能读取一级目录中的所有普通文件及目录，不能递归！返回的list包含路径信息
bool CDir::ReadDir(const string &sDir, list<string> &filesList, list<string> &dirsList)
{
    filesList.clear();
    dirsList.clear();
    
    string strDir = sDir;
    if (strDir.empty())
    {
        return false;
    }

    if (strDir[strDir.length()-1] != '/')
    {
        strDir += "/";
    }

    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    dir = opendir(strDir.c_str());
    if (dir == NULL)
    {
        return false;
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        //4为目录
        if (ptr->d_type != 4)
        {
            string filename = ptr->d_name;
            if (!filename.empty())
            {
                filesList.push_back(strDir+filename);
            }
            //printf("d_name=%s\t\td_type=%d \n", ptr->d_name, ptr->d_type);
        }

        if (ptr->d_type == 4)
        {
            if (strcmp(ptr->d_name, ".")!=0 && strcmp(ptr->d_name, "..")!=0)
            {
                string dirname = ptr->d_name;
                if (!dirname.empty())
                {
                    dirsList.push_back(strDir+dirname);
                }
            }
        }
    }

    closedir(dir);

    return true;
}


//只能读取一级目录中的所有普通文件及目录，不能递归！返回的list包含路径信息
int CDir::ReadDirByAlphasort(const string &sDir, list<string> &filenameList, list<string> &dirList)
{
	dirList.clear();
	filenameList.clear();

	string strDir = sDir;

	if (strDir.empty())
	{
		return -1;
	}

	if (strDir[strDir.length()-1] != '/')
    {
        strDir += "/";
    }
	

	struct dirent **namelist;
	int n;

	n = scandir(strDir.c_str(), &namelist, 0, alphasort);
	if (n < 0)
	{
	    printf("ReadDirByAlphasort: %s\n", sDir.c_str());
		perror("scandir");
	}
	else 
	{
		for (int i=0; i<n; i++) 
		{
			//file
			if (namelist[i]->d_type == 8)
			{
				//printf("%s\n", namelist[i]->d_name);
				filenameList.push_back(strDir+namelist[i]->d_name);
			}

			//dir
			if (namelist[i]->d_type == 4)
			{
				if (strcmp(namelist[i]->d_name, ".")!=0 && strcmp(namelist[i]->d_name, "..")!=0)
				{
					dirList.push_back(strDir+namelist[i]->d_name);
				}
			}

			free(namelist[i]);
		}
		free(namelist);
	}

	return 0;

}


int CDir::ReadDirByVersionsort(const string &sDir, list<string> &filenameList, list<string> &dirList)
{
    dirList.clear();
    filenameList.clear();

    string strDir = sDir;

    if (strDir.empty())
    {
        return -1;
    }

    if (strDir[strDir.length()-1] != '/')
    {
        strDir += "/";
    }


    struct dirent **namelist;
    int n;

    n = scandir(strDir.c_str(), &namelist, 0, versionsort);
    if (n < 0)
    {
        printf("ReadDirByAlphasort: %s\n", sDir.c_str());
        perror("scandir");
    }
    else
    {
        for (int i=0; i<n; i++)
        {
            //file
            if (namelist[i]->d_type == 8)
            {
                //printf("%s\n", namelist[i]->d_name);
                filenameList.push_back(strDir+namelist[i]->d_name);
            }

            //dir
            if (namelist[i]->d_type == 4)
            {
                if (strcmp(namelist[i]->d_name, ".")!=0 && strcmp(namelist[i]->d_name, "..")!=0)
                {
                    dirList.push_back(strDir+namelist[i]->d_name);
                }
            }

            free(namelist[i]);
        }
        free(namelist);
    }

    return 0;

}



void CDir::PrintStringList(const list<string> &stringList)
{
    cout<<"-----------------------"<<endl;
    list<string> tList;
    tList = stringList;
    while (!tList.empty())
    {
        cout<<tList.front()<<endl;
        tList.pop_front();
    }
    cout<<"-----------------------"<<endl<<endl;
}
