#ifndef DATABASE_H
#define DATABASE_H

#include <string>
using namespace std;

#include <sqlite3.h>

class CCar;
struct Record;

class CDatabase
{
public:
    CDatabase(const string& strDbName);
    ~CDatabase();

    int InsertRecord(const Record& record);

    //删除最早的记录,并返回本地文件名
    int DeleteHeadRecord(string& strImageFile, string& strVideoFile);

    //根据数据库主键删除记录
    int DeleteRecord(int id);

    int GetFirstRecord(Record& record);

    //取得记录总条数
    int GetTotal();

private:
    void OpenDB(const string& strDBname);

    void CreateInfoTable();
    int ExecSql(const string& sql);

private:
    string m_strDbName;
    sqlite3* m_pDB;
    char *m_pErrMsg;
};


#endif //DATABASE_H

