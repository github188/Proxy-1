#include "stdio.h"
#include "stdlib.h"
#include "tools.h"
#include "string.h"

bool GetProfileString(char key[], char value[], char str[], int strLen, const char filename[])
{
    int i;

    FILE* file = fopen(filename, "r");
    if (file == 0)
        return false;

    char buffer[1024];
    while (!feof(file))
    {
        char * p = fgets(buffer, 1024, file);
        if (p == 0)
            break;

        if (buffer[0] == '[')
        {
            for (i = 0; buffer[i] != 0; i++)
                if (buffer[i] == ']')
                    break;

            if (buffer[i] != ']')
                continue;

            buffer[i] = 0;

            if (strcmp(key, &buffer[1]) != 0)
                continue;

            while (!feof(file))
            {
                char* p = fgets(buffer, 1024, file);
                if (p == 0)
                    break;
                if (buffer[0] == '[')
                    break;

                for (i = 0; i < buffer[i] != 0; i++)
                    if (buffer[i] == '=')
                        break;

                if (buffer[i] != '=')
                    break;
                buffer[i] = 0;
                if (strcmp(buffer, value) != 0)
                    continue;

                strncpy(str, &buffer[i + 1], strLen);

                if (strlen(str) >= 2 && str[strlen(str) - 2] == '\r')
                {
                    str[strlen(str) - 2] = '\0';
                }

                if (strlen(str) >= 1 && str[strlen(str) - 1] == '\n')
                {
                    str[strlen(str) - 1] = '\0';
                }
                fclose(file);
                return true;
            }
        }
    }

    fclose(file);

    return false;
}

bool PutProfileString(char key[], char value[], const char str[], const char filename[])
{
    int i;

    FILE* file = fopen(filename, "r");
    if (file == 0)
    {
        file = fopen(filename, "w");
        if (file == 0)
            return false;
        char buffer[256];
        sprintf(buffer, "[%s]\n", key);
        fwrite(buffer, 1, strlen(buffer), file);
        sprintf(buffer, "%s=%s\n", value, str);
        fwrite(buffer, 1, strlen(buffer), file);
        fclose(file);
        return true;
    }

    char buffer[1024];
    bool bHasSection = false;
    while (!feof(file))
    {
        char * p = fgets(buffer, 1024, file);
        if (p == 0)
            break;

        if (buffer[0] == '[')
        {
            for (i = 0; buffer[i] != 0; i++)
                if (buffer[i] == ']')
                    break;

            if (buffer[i] != ']')
                continue;

            buffer[i] = 0;

            if (strcmp(key, &buffer[1]) != 0)
                continue;

            bHasSection = true;
            break;
        }
    }

    if (!bHasSection)
    {
        fclose(file);
        file = fopen(filename, "r+");
        if (file == 0)
            return false;
        fseek(file, 0, SEEK_END);

        char buffer[256];
        sprintf(buffer, "[%s]\n", key);
        fwrite(buffer, 1, strlen(buffer), file);
        sprintf(buffer, "%s=%s\n", value, str);
        fwrite(buffer, 1, strlen(buffer), file);
        fclose(file);
        return true;
    }
    else
    {
        bool bHasKey = false;
        long pos1, pos2;

        pos1 = ftell(file);
        while (!feof(file))
        {
            pos1 = ftell(file);
            char* p = fgets(buffer, 1024, file);
            pos2 = ftell(file);
            if (p == 0)
                break;
            if (buffer[0] == '[')
                break;

            for (i = 0; i < buffer[i] != 0; i++)
                if (buffer[i] == '=')
                    break;

            if (buffer[i] != '=')
                break;
            buffer[i] = 0;

            if (strcmp(buffer, value) != 0)
                continue;

            bHasKey = true;
            break;
        }

        fseek(file, 0L, SEEK_END);
        long len = ftell(file);

        char* pbuf = new char[len];
        fseek(file, 0L, SEEK_SET);
        fread(pbuf, 1, len, file);
        fclose(file);

        file = fopen(filename, "w");

        if (bHasKey)
        {
            fwrite(pbuf, 1, pos1, file);
            char buffer[256];
            sprintf(buffer, "%s=%s\n", value, str);
            fwrite(buffer, 1, strlen(buffer), file);
            fwrite(pbuf + pos2, 1, len - pos2, file);
        }
        else
        {
            fwrite(pbuf, 1, pos1, file);
            char buffer[256];
            sprintf(buffer, "%s=%s\n", value, str);
            fwrite(buffer, 1, strlen(buffer), file);
            fwrite(pbuf + pos1, 1, len - pos1, file);
        }

        delete pbuf;
    }

    fclose(file);

    return true;
}

CPtrList::CPtrList()
{
    m_nCount = 0;
    pthread_mutex_init(&m_mutex, NULL);
}

CPtrList::~CPtrList()
{
    pthread_mutex_destroy(&m_mutex);
}

bool CPtrList::AddTail(void* ptr)
{
    bool r = false;

    pthread_mutex_lock(&m_mutex);

    if (m_nCount < 1024)
    {
        m_ptr[m_nCount] = ptr;
        m_nCount++;
        r = true;
    }

    pthread_mutex_unlock(&m_mutex);

    return r;
}

void* CPtrList::RemoveHead()
{
    pthread_mutex_lock(&m_mutex);

    void* p = NULL;
    if (m_nCount > 0)
    {
        p = m_ptr[0];
        for (int i = 1; i < m_nCount; i++)
        {
            m_ptr[i - 1] = m_ptr[i];
        }
        m_nCount--;
    }
    pthread_mutex_unlock(&m_mutex);

    return p;
}

int CPtrList::GetCount()
{
    return m_nCount;
}

void* CPtrList::GetAt(int index)
{
    if (index > m_nCount - 1)
        return NULL;
    void* p = NULL;
    pthread_mutex_lock(&m_mutex);
    p = m_ptr[index];
    pthread_mutex_unlock(&m_mutex);
    return p;
}

void* CPtrList::RemoveAt(int index)
{
    if (index > m_nCount - 1)
        return NULL;
    void* p = NULL;
    pthread_mutex_lock(&m_mutex);
    p = m_ptr[index];
    for (int i = index; i < m_nCount - 1; i++)
        m_ptr[i] = m_ptr[i + 1];
    m_nCount--;
    pthread_mutex_unlock(&m_mutex);
    return p;
}

bool CPtrList::InsertAt(void* ptr, int index)
{
    if (m_nCount >= 1024)
        return false;
    if (index > m_nCount)
        return false;

    pthread_mutex_lock(&m_mutex);
    for (int i = m_nCount - 1; i >= index; i--)
    {
        m_ptr[i + 1] = m_ptr[i];
    }
    m_ptr[index] = ptr;
    m_nCount++;
    pthread_mutex_unlock(&m_mutex);

    return true;
}

int min(int a1, int a2)
{
    return (a1 < a2) ? a1 : a2;
}

int max(int a1, int a2)
{
    return (a1 > a2) ? a1 : a2;
}

//return remote filename
string LocalFilename2RemoteFilename(const string &strLocalFile, string &strRemoteFile)
{
    string strTemp = strLocalFile;

    //去掉前面BACKUPDATA
    //    size_t found = strTemp.find(BACKUPDIR);
    //    if (found != string::npos)
    //    {
    //        strTemp = strTemp.substr(found+BACKUPDIR.size(), strTemp.length()-1);
    //    }

    size_t found = strTemp.find("/");
    if (found != string::npos)
    {
        strTemp[found] = '\\';
    }

    while (found != string::npos)
    {
        found = strTemp.find("/");
        if (found != string::npos)
        {
            strTemp[found] = '\\';
        }
    }

    strRemoteFile = strTemp.substr(0, strTemp.length());

    //cout<<"CCreateFilename::LocalFilename2RemoteFilename: strLocalFile "<<strLocalFile<<endl;
    //cout<<"CCreateFilename::LocalFilename2RemoteFilename: strRemoteFile "<<strRemoteFile<<endl;

    return strRemoteFile;

}

//return local file name
string RemoteFilename2LocalFilename(const string &strRemoteFile, string &strLocalFile)
{
    string strTemp = strRemoteFile;

    size_t found = strTemp.find("\\");
    if (found != string::npos)
    {
        strTemp[found] = '/';
    }

    while (found != string::npos)
    {
        found = strTemp.find("\\");
        if (found != string::npos)
        {
            strTemp[found] = '/';
        }
    }

    if (strTemp[0] == '/')
    {
        strLocalFile = string("/") + strTemp.substr(1, strTemp.length() - 1);
    }
    else
    {
        strLocalFile = string("/") + strTemp.substr(0, strTemp.length());
    }

    //cout<<"CCreateFilename::RemoteFilename2LocalFilename: strRemoteFile "<<strRemoteFile<<endl;
    //cout<<"CCreateFilename::RemoteFilename2LocalFilename: strLocalFile "<<strLocalFile<<endl;

    return strLocalFile;
}

