
#define VERSION "0.0.1"

#include "../config/config.h"
#include "../include/pdebug.h"
#include "../Network/Network.h"
#include "../Log/Log.h"
#include "ProxyTaskDispatcher.h"
#include "ReconnectThread.h"

#ifdef CONFIG_MONITOR
#include "/usr/local/include/Kise/MonitorModule/MonitorModule.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <list>

#define CONFIG_PATH "./"

#ifdef CONFIG_BACKUP
#include "../Backup/BackupModule.h"
struct CBackupModule *g_pbackup = NULL;
#endif

#include "backup.h"

class CProxyTaskDispatcher *g_dispatcher = NULL;

void InitConnections(CNetwork *pNet);

/** main : 程序主入口
 *  1. 读取配置文件中设置的每一组需要转换的端(TCPConn)
 *  2. 初始化所有连接并分好边
 *  3. 启动TaskDispatcher处理网络数据
 *  4. 另外，在这里创建需要重连的连接链表及其操作用到的锁
 */
int main(int argc, char* argv[])
{
	const struct ProxyConfig* pcfg = ConfigGet();
	if(!pcfg) {
		printf("Read config(%s) failed.\n", CONFIG_FILE);
		exit(0);
	}
	
	int modID = 3;
	if (argc == 2) {
        modID = atoi(argv[1]);
        printf("modID = %d\n", modID);
    }

#ifdef CONFIG_MONITOR
	if (pcfg->control.enable_monitor) {
		Kise::Monitor::SetModuleID(modID);
		Kise::Monitor::MonitorLoop();
	}
#endif

	if ( pcfg->control.enable_backup ) {
		backup_init();
	}
	
	Kise::Log::CLog::Instance().SetLogPath("./logpath");
	Kise::Log::CLog::Instance().SetMaxLogDayNum(7);
	
	std::list<struct TCPConn> brokenconns;
	pthread_mutex_t conns_lock;
	pthread_mutex_init(&conns_lock, NULL);

	g_dispatcher = new CProxyTaskDispatcher;
	g_dispatcher->SetBrokenConns(&brokenconns, &conns_lock);
	g_dispatcher->Start();
	CNetwork network(g_dispatcher);

	InitConnections( &network );
	
	CReconnectThread reconnThread(network);
	reconnThread.SetBrokenConns(&brokenconns, &conns_lock);
	reconnThread.Start();

	network.Start();

	while (1) {
        sleep(5);
    }

	// network.Join();
	pthread_mutex_destroy(&conns_lock);
	exit(0);
}

void InitConnections(CNetwork *pNet)
{
	const struct ProxyConfig* pcfg = ConfigGet();
	PXGROUP::const_iterator it;
	const PXGROUP &gp = pcfg->groups;
	for(it = gp.begin(); it != gp.end(); it++) {
		const PXCONNS &v1 = (*it).left;
		const PXCONNS &v2 = (*it).right;
		PXCONNS::const_iterator temp;
		for(temp = v1.begin(); temp != v1.end(); temp++) {
			if( (*temp).mod == CLIENT ) {
				pNet->Client( (*temp).ip.c_str(), (*temp).port );
			} else if((*temp).mod == SERVER) {
				pNet->Server( (*temp).port );
			} else {

			}
		}
		for(temp = v2.begin(); temp != v2.end(); temp++) {
			if( (*temp).mod == CLIENT ) {
				pNet->Client( (*temp).ip.c_str(), (*temp).port );
			} else if((*temp).mod == SERVER) {
				pNet->Server( (*temp).port );
			} else {

			}
		}
	}

}


