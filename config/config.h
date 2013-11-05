
#ifndef CONFIG_H
#define CONFIG_H

#include "../include/constant.h"
#include <string>
#include <vector>

#define CONFIG_FILE "./config.ini"

struct TCPConn;
struct ProxyGroup; 
struct ProxyConfig;

enum TCP_MOD { CLIENT = 0, SERVER = 1 };
enum GRP_SIDE {LEFTSIDE, RIGHTSIDE};

/* ConfigGet & ConfigSet
 * 获取与设置系统的配置项
 */
const struct ProxyConfig *ConfigGet();
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
typedef std::vector<struct TCPConn> PXCONNS;

/* TCPConnEq : 判断两个网络配置是否一样 
 * 相等的规则是: 模式一致，在客户端模式下 ip和port必须一样
 * 在服务器模式下监听端口port必须一样
 */
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


/* ProxyGroup
 * 代理中的一个组，每个组有两边
 * 每一边可能有N个连接
 * 两边通过转化规则rule交换数据
 */
struct ProxyGroup {
	int id;
	PXCONNS left;
	PXCONNS right;
	enum CVT_RULE rule;
};
typedef std::vector<struct ProxyGroup> PXGROUP;

/* BackupConfig : 备份需要用到的配置 */
struct BackupConfig {
	int max_backup_num;     // 最大备份记录条数
	std::string backup_dir; // 备份路径
	int backup_group_id;    // 备份的组
	enum GRP_SIDE backup_side; // 备份的一边
};

struct SystemControlConfig{
	bool enable_monitor;
	bool enable_backup;

	int cache_live;
};

struct ProxyConfig {
	struct SystemControlConfig control;
	struct BackupConfig *backup;
	PXGROUP groups;
};

#endif

