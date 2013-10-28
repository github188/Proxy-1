#ifndef BACKUP_UTILS_H
#define BACKUP_UTILS_H

#include <string>
#include <vector>
using namespace std;

class CCar;

void CreateLocalFilename(const string& strBackupDir, const CCar* pCar, vector<string>& imageFiles, vector<string>& videoFiles);

#endif //

