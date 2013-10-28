#include "ClearDirThread.h"

#include <stdio.h>

#include "../Tool/dir.h"

CClearDirThread::CClearDirThread(const string& strDir/*="./backup"*/):
m_bQuit(false),
m_strBackupDir(strDir)
{
}

CClearDirThread::~CClearDirThread()
{
    m_bQuit = true;

    Join();
}

void CClearDirThread::Run()
{
    m_bQuit = false;

    while (!m_bQuit)
    {
        CDir dir;
        list<string> fileList;
        list<string> dirList;

        dir.ReadDirByVersionsort(m_strBackupDir, fileList, dirList);

        if (!dirList.empty())
        {
            string strDir = dirList.front();
            while (!strDir.empty())
            {
                fileList.clear();
                dirList.clear();

                dir.ReadDirByVersionsort(strDir, fileList, dirList);

                //如果此层目录跟文件都不存在，则删除此目录
                if (fileList.empty() && dirList.empty())
                {
                    dir.RemoveDir(strDir);
                    break;
                }

                //如果此层目录为空，文件还存在 ，说明到达最底层，备份还在，退出此次判断
                //如果此层目录不为空，进行下一层目录判断
                if (dirList.empty())
                {
                    break;
                }
                else
                {
                    strDir = dirList.front();
                }
            }
        }

        sleep(20);
    }
}

