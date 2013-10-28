#include "BackupUtils.h"

#include "BackupCommon.h"
#include "../Tool/timeclass.h"
#include "../include/Car.h"
// #include "../CodeConvert/CodeConvert.h"

void CreateLocalFilename(const string& strBackupDir, const CCar* pCar, vector<string>& imageFiles, vector<string>& videoFiles)
{
    assert(pCar != NULL);

    char tmp[128];

    string filename;

    //构建目录
    filename = strBackupDir+"/";

    if (pCar->isViolation)
    {
        snprintf(tmp, 128, "%s/", pCar->vioType);
        filename += tmp;
    }
    else
    {
        snprintf(tmp, 128, "%s/", "0");
        filename += tmp;
    }

    //年月日+时分(分钟只取0或30)
    CTime t(pCar->time[0]);
    snprintf(tmp, 128, "%.4d%.2d%.2d/%.2d%.2d/%d/", \
                t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute()>=30?30:0, pCar->roadNo);
    filename += tmp;

    //构建文件名

    //年月日时分秒-毫秒
    snprintf(tmp, 128, "%.4d%.2d%.2d%.2d%.2d%.2d-%.3d-", \
            t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second(), t.MilliSecond());
    filename += tmp;

    //监控点方向编号车道编号-
    snprintf(tmp, 128, "%s%s%.2d-", pCar->monitorID.c_str(), pCar->direction.c_str(), pCar->roadNo);
    filename += tmp;

    //车牌号码
	/*
    CCodeConvert codec;
    string strUnicodeCarPlate;
    codec.CarPlatetoUnicode(pCar->plate, strUnicodeCarPlate);
    string strUtf8Plate = codec.UnicodetoUTF8(strUnicodeCarPlate);
	*/
    filename += "000000";// strUtf8Plate;
    filename += "-";

    for (unsigned short i=0; i<pCar->imageNum; i++)
    {
        snprintf(tmp, 32, "%d.jpg", i+1);
        imageFiles.push_back(filename+tmp);
    }

    for (int i=0; i<pCar->videoNum; i++)
    {
        snprintf(tmp, 32, "%d.%s", i+1, pCar->videoFormat[i]);
        videoFiles.push_back(filename+tmp);
    }
}

