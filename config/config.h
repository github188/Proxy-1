
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#define CONFIG_FILE "./config.ini"

struct tcp_conn;  
struct pxgroup; 
struct proxyconfig;

enum TCP_MOD { CLIENT = 0, SERVER = 1 };
enum CVT_RULE { RULE_EMPTY = 0 };

/**
 * 接口： 获取以及设置配置
 */
const struct proxyconfig *cfg_get();
bool cfg_set(const struct proxyconfig *conf);

struct tcp_conn {
	enum TCP_MOD mod;
	std::string ip;
	unsigned short port;
};

typedef std::vector<struct tcp_conn> PXCONNS;

struct pxgroup {
	PXCONNS left;
	PXCONNS right;
	enum CVT_RULE cvt_rule;
};

struct proxyconfig {
	bool enableMonitor;
	std::vector<pxgroup> groups;
};

#endif
