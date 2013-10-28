#include "Database.h"

#include <stdlib.h>
#include <string.h>
#include "../Log/Log.h"

#include "../include/Car.h"
#include "BackupCommon.h"

CDatabase::CDatabase(const string& strDbName):m_strDbName(strDbName)
{
    m_pDB = NULL;

    OpenDB(m_strDbName);

    CreateInfoTable();
}

CDatabase::~CDatabase()
{
    if (m_pErrMsg != NULL)
    {
        sqlite3_free(m_pErrMsg);
        m_pErrMsg = NULL;
    }

    if (m_pDB != NULL)
    {
        sqlite3_close(m_pDB);
        m_pDB = NULL;
    }
}


void CDatabase::OpenDB(const string& strDBname)
{
    int ret = sqlite3_open(strDBname.c_str(), &m_pDB);
    if (ret != SQLITE_OK)
    {
        ERRLOG("SQL error:%s, %s\n", sqlite3_errmsg(m_pDB), strDBname.c_str());
        sqlite3_close(m_pDB);

        m_pDB = NULL;
    }
}


void CDatabase::CreateInfoTable()
{
    string sql = "create table tb_info(id integer primary key autoincrement, \
            monitorID text, \
            cameraNo integer, \
            roadNo integer, \
            direction text, \
            plate text, \
            plateX integer, \
            plateY integer, \
            plateWidth integer, \
            plateHeight integer, \
            confidence integer, \
            plateColor integer, \
            vehicleStyle integer, \
            vehicleColor integer, \
            speed integer, \
            limitSpeed integer, \
            detect integer, \
            isViolation integer, \
            vioType text, \
            redBeginTime text, \
            imageNum integer, \
            imageTime text, \
            imageFile text, \
            videoNum integer, \
            videoFormat text, \
            videoBeginTime text, \
            videoEndTime text, \
            videoFile text);";

    ExecSql(sql);
}


int CDatabase::InsertRecord(const Record& record)
{
    if (m_pDB == NULL)
    {
        return -1;
    }

    assert(record.pCar != NULL);
    CCar* pCar = record.pCar;

    //insert into table
    char sql[2048];
    memset(sql, 0, sizeof(sql));

    int ret = snprintf(sql, sizeof(sql), "insert into tb_info(monitorID, cameraNo, roadNo, direction, \
            plate, plateX, plateY, plateWidth, plateHeight, confidence, plateColor, \
            vehicleStyle, vehicleColor, speed, limitSpeed, detect, \
            isViolation, vioType, redBeginTime, \
            imageNum, imageTime, imageFile, videoNum, videoFormat, videoBeginTime, videoEndTime, videoFile) \
            values \
            (\"%s\", %d, %d, \"%s\", \
            \"%s\", %d, %d, %d, %d, %d, %d, \
            %d, %d, %d, %d, %d, \
            %d, \"%s\", \"%s\", \
            %d, \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\", \"%s\");", \
            pCar->monitorID.c_str(), pCar->cameraNo, pCar->roadNo, pCar->direction.c_str(), \
            pCar->plate, pCar->plateX, pCar->plateY, pCar->plateWidth, pCar->plateHeight, pCar->confidence, pCar->plateColor, \
            pCar->style, pCar->color, pCar->speed, pCar->limitSpeed, pCar->detect, \
            pCar->isViolation, pCar->vioType, record.strRedBeginTime.c_str(), \
            pCar->imageNum, record.strImageTime.c_str(), record.strImageFile.c_str(), pCar->videoNum, pCar->videoFormat[0], \
            record.strVideoBeginTime.c_str(), record.strVideoEndTime.c_str(), record.strVideoFile.c_str());

    assert(ret > 0 && ret < 2048);

    return ExecSql(sql);
}



int CDatabase::DeleteHeadRecord(string& strImageFile, string& strVideoFile)
{
    int id = 0;
    strImageFile.clear();
    strVideoFile.clear();

    char sql[] = "select id, imageFile, videoFile from tb_info order by id limit 1";

    char **result = NULL;
    int row = 0;
    int column = 0;

    int rc = sqlite3_get_table(m_pDB, sql, &result, &row, &column, &m_pErrMsg);
    if (rc != SQLITE_OK)
    {
        ERRLOG("SQL %s error: %s\n", sql, m_pErrMsg);
        sqlite3_free(m_pErrMsg);
        m_pErrMsg = NULL;
    }

    if (result != NULL)
    {
        if (row > 0 && result[2] != NULL && result[3])
        {
			id = atoi(result[3]);
            strImageFile = result[4];
            strVideoFile = result[5];
        }
        sqlite3_free_table(result);
    }

    char delSql[256];
    sprintf(delSql, "delete from tb_info where id=%d", id);

    return ExecSql(delSql);
}


//根据数据库主键删除记录
int CDatabase::DeleteRecord(int id)
{
    char sql[256];
    sprintf(sql, "delete from tb_info where id=%d", id);

    return ExecSql(sql);
}


int CDatabase::GetFirstRecord(Record& record)
{
    assert(record.pCar != NULL);

    record.primaryKeyID = -1;

    if (m_pDB == NULL)
    {
        return -1;
    }

    int ret = -1;

    char **result = NULL;
    int row = 0;
    int column = 0;

    string strSql = "select * from tb_info order by id limit 1";

    int rc = sqlite3_get_table(m_pDB, strSql.c_str(), &result, &row, &column, &m_pErrMsg);
    if (rc != SQLITE_OK)
    {
        ERRLOG("SQL %s error: %s\n", strSql.c_str(), m_pErrMsg);
        sqlite3_free(m_pErrMsg);
        m_pErrMsg = NULL;
    }

    if (result != NULL)
    {
        if (row > 0)
        {
            ret = 0;

            record.primaryKeyID = atoi(result[column]);
            record.pCar->monitorID = result[column+1];
            record.pCar->cameraNo = atoi(result[column+2]);
            record.pCar->roadNo = atoi(result[column+3]);
            record.pCar->direction = result[column+4];
            strcpy(record.pCar->plate, result[column+5]);
            record.pCar->plateX = atoi(result[column+6]);
            record.pCar->plateY = atoi(result[column+7]);
            record.pCar->plateWidth = atoi(result[column+8]);
            record.pCar->plateHeight = atoi(result[column+9]);
            record.pCar->confidence = atoi(result[column+10]);
            record.pCar->plateColor = atoi(result[column+11]);
            record.pCar->style = atoi(result[column+12]);
            record.pCar->color = atoi(result[column+13]);
            record.pCar->speed = atoi(result[column+14]);
            record.pCar->limitSpeed = atoi(result[column+15]);
            record.pCar->detect = atoi(result[column+16]);
            record.pCar->isViolation = atoi(result[column+17]);
            strcpy(record.pCar->vioType, result[column+18]);
            record.strRedBeginTime = result[column+19];

            record.pCar->imageNum = atoi(result[column+20]);
            for (unsigned short i=0; i<record.pCar->imageNum; i++)
            {
                record.pCar->imageLen[i] = 0;
                record.pCar->image[i] = NULL;
            }
            record.pCar->CreatePacketID(record.pCar, record.pCar->packetID);

            record.strImageTime = result[column+21];
            record.strImageFile = result[column+22];

            record.pCar->videoNum = atoi(result[column+23]);
            for (int i=0; i<record.pCar->videoNum; i++)
            {
                record.pCar->videoLen[i] = 0;
                record.pCar->videoData[i] = 0;
                strcpy(record.pCar->videoFormat[i], result[column+24]);
            }
            record.strVideoBeginTime = result[column+25];
            record.strVideoEndTime = result[column+26];
            record.strVideoFile = result[column+27];
        }

        sqlite3_free_table(result);
    }

    return ret;
}


//搜索tb_info表，取得记录总条数
int CDatabase::GetTotal()
{
    if (m_pDB == NULL)
    {
        return -1;
    }

    int first_count = 0;
    int last_count = 0;
    bool bFirtGet = false;
    bool bLastGet = false;


    char sql[] = "select id from tb_info order by id limit 1";
    char sql_last[] = "select id from tb_info order by id desc limit 1";

    char **result = NULL;
    int row = 0;
    int column = 0;

    //取得第一条记录的id
    int rc = sqlite3_get_table(m_pDB, sql, &result, &row, &column, &m_pErrMsg);
    if (rc != SQLITE_OK)
    {
        ERRLOG("SQL %s error: %s\n", sql, m_pErrMsg);
        sqlite3_free(m_pErrMsg);
        m_pErrMsg = NULL;
        return 0;
    }

    if (result != NULL)
    {
        if (row>0 && result[1]!=NULL)
        {
            first_count = atoi(result[1]);
            bFirtGet = true;
        }

        sqlite3_free_table(result);
    }

    //取得最后一条记录的id
    rc = sqlite3_get_table(m_pDB, sql_last, &result, &row, &column, &m_pErrMsg);
    if (rc != SQLITE_OK)
    {
        ERRLOG("SQL %s error: %s\n", sql_last, m_pErrMsg);
        sqlite3_free(m_pErrMsg);
        m_pErrMsg = NULL;

        return 0;
    }

    if (result != NULL)
    {
        if (row > 0 && result[1] != NULL)
        {
            last_count = atoi(result[1]);
            bLastGet = true;
        }

        sqlite3_free_table(result);
    }

    int total = 0;
    if (bFirtGet && bLastGet)
    {
        total = last_count - first_count+1;
    }

    //printf("total = %d\n", total);

    return total;
}


int CDatabase::ExecSql(const string& sql)
{
    if (m_pDB == NULL)
    {
        return -1;
    }

    int rc = sqlite3_exec(m_pDB, sql.c_str(), NULL, 0, &m_pErrMsg);
    if (rc != SQLITE_OK)
    {
        ERRLOG("SQL %s error: %s\n", sql.c_str(), m_pErrMsg);
        sqlite3_free(m_pErrMsg);
        m_pErrMsg = NULL;

        return -1;
    }

    return 0;
}


