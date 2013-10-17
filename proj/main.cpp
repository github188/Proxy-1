
#define VERSION "0.0.1"

#include "../config/config.h"
#include "../include/pdebug.h"
#include "../Network/Network.h"
#include "../Log/Log.h"
#include "ProxyTaskDispatcher.h"
#include "ReconnectThread.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <list>

#define CONFIG_PATH "./"

void InitConnections(CNetwork *pNet);

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
	
	if (pcfg->system.enableMonitor) {
		// Kise::Monitor::SetModuleID(modID);
		// Kise::Monitor::MonitorLoop();
	}
	
	Kise::Log::CLog::Instance().SetLogPath("./logpath");
	Kise::Log::CLog::Instance().SetMaxLogDayNum(7);
	
	std::list<struct TCPConn> brokenconns;
	pthread_mutex_t conns_lock;
	pthread_mutex_init(&conns_lock, NULL);

	CProxyTaskDispatcher taskDispatcher;
	taskDispatcher.SetBrokenConns(&brokenconns, &conns_lock);
	taskDispatcher.Start();
	CNetwork network(&taskDispatcher);

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

