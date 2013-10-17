
#include <config/config.h>
#include <pdebug.h>
#include <Network/Network.h>
#include "ProxyTaskDispatcher.h"
#define VERSION "0.0.1"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <list>

#define CONFIG_PATH "./"

void InitConnections(CNetwork *pNet);

int main(int argc, char* argv[])
{
	const struct proxyconfig* pcfg = cfg_get();
	if(!pcfg) {
		printf("read configure file(%s) failed.\n", CONFIG_FILE);
		exit(0);
	}
	
	int modID = 3;
	if (argc == 2) {
        modID = atoi(argv[1]);
        printf("modID = %d\n", modID);
    }
	
	if (pcfg->enableMonitor) {
		// Kise::Monitor::SetModuleID(modID);
		// Kise::Monitor::MonitorLoop();
	}
	
	sleep(3);

	// Kise::Log::CLog::Instance().SetLogPath("./logpath");
	// Kise::Log::CLog::Instance().SetMaxLogDayNum(7);
    // printf("NetworkProxy log path = %s, log num = %d\n", logPath.c_str(), logNum);
	
	std::list<struct tcp_conn> brokenconns;
	pthread_mutex_t conns_lock;
	pthread_mutex_init(&conns_lock, NULL);

	CProxyTaskDispatcher taskDispatcher;
	taskDispatcher.SetBrokenConns(&brokenconns);
	taskDispatcher.Start();
	CNetwork network(&taskDispatcher);

	InitConnections( &network );
	
	/*
	CReconnectThread reconnThread(network);
	reconnThread.setBrokenConns(&brokenconns, conns_lock);
	reconnThread.Start();
	*/

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
	const struct proxyconfig* pcfg = cfg_get();
	std::vector<pxgroup>::const_iterator it;
	for(it = pcfg->groups.begin(); it != pcfg->groups.end(); it++) {
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
