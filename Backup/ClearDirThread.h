#ifndef CLEAR_DIR_THREAD_H
#define CLEAR_DIR_THREAD_H

#include <string>
using namespace std;

#include "../Thread/AbstractThread.h"

class CClearDirThread : public CAbstractThread
{
public:
    CClearDirThread(const string& strDir="./backup");
    ~CClearDirThread();

protected:
    void Quit() { m_bQuit = true; }
    void Run();

private:
    bool m_bQuit;

    string m_strBackupDir;
};

#endif //CLEAR_DIR_THREAD_H
