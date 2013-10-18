
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#define CONFIG_FILE "./config.ini"

struct TCPConn;
struct ProxyGroup; 
struct ProxyConfig;

enum TCP_MOD { CLIENT = 0, SERVER = 1 };
enum CVT_RULE {
	RULE_EMPTY = 0 
#ifdef CONVERT_ZJ
		, RULE_ZJ = 1
#endif 
#ifdef CONVERT_SF
		, RULE_SF = 2
#endif
};

enum GRP_SIDE {LEFTSIDE, RIGHTSIDE};

/* ConfigGet
 * 获取系统的配置项，返回的结构体中定义配置的内容
 */
const struct ProxyConfig *ConfigGet();

/*
 *
 */
bool ConfigSet(const struct ProxyConfig *conf);

/* ConfigLookup
 * 在系统的配置中查找某个网络连接的属性
 * con: 要查询的连接
 * groupid [out]: 返回所属的组ID
 * side [out]: 返回该连接属于组的哪一边
 * rule [out]: 
 */
bool ConfigLookup(const struct TCPConn &con, 
		int *groupid, enum GRP_SIDE *side, enum CVT_RULE *rule );

/* TCPConn
 * 表示配置文件中的一个网络配置项
 * mod : 模式，作为客户端或者服务端
 * ip & port : 地址和端口 
 */
struct TCPConn {
	enum TCP_MOD mod; 
	std::string ip;
	unsigned short port;
};

inline bool TCPConnEq(const struct TCPConn &lhs,
		const struct TCPConn &rhs)
{
	if(lhs.mod == rhs.mod) {
		if(lhs.mod == CLIENT)
			return (lhs.ip == rhs.ip && lhs.port == rhs.port);
		else if(lhs.mod == SERVER)
			return (lhs.port == rhs.port);
	}
	return false;
}

typedef std::vector<struct TCPConn> PXCONNS;

/* ProxyGroup
 * 代理中的一个组，每个组中的两边通过转化规则rule
 * 交换数据
 */
struct ProxyGroup {
	int id;
	PXCONNS left;
	PXCONNS right;
	enum CVT_RULE rule;
};
typedef std::vector<struct ProxyGroup> PXGROUP;

/* CommonConfig
 * 系统的普通配置项全部在这里定义
 */
struct CommonConfig {
	bool enableMonitor;
};

struct ProxyConfig {
	struct CommonConfig system;
	PXGROUP groups;
};

#endif

