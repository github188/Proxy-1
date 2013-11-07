
#include "config.h"
#include "ConfFile.h"
#include "../proj/Convert.h"
#include <pdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct ProxyConfig g_config;
static bool g_isconfgok = false;
static CConfFile g_file(CONFIG_FILE);
static void cfg_init();

/*如果初始化成功则返回全局配置项的地址*/
const struct ProxyConfig *ConfigGet()
{
	if(!g_isconfgok)
		cfg_init();

	if(g_isconfgok)
		return &g_config;
	else
		return NULL;
}

bool ConfigLookup(const struct TCPConn &con, 
		int *groupid, enum GRP_SIDE *side, enum CVT_RULE *rule )
{
	if(!g_isconfgok) return false;

	PXGROUP::const_iterator it;
	const PXGROUP &grp = g_config.groups;
	for(it = grp.begin(); it != grp.end();it++) {
		PXCONNS::const_iterator temp;
		const PXCONNS &p = (*it).left;
		for(temp = p.begin(); temp != p.end(); temp++) {
			if( TCPConnEq( con, *temp) ) {
				if(groupid) *groupid = (*it).id;
				if(side) *side = LEFTSIDE;
				if(rule) *rule = (*it).rule;
				return true;
			}
		}
		const PXCONNS &p2 = (*it).right;
		for(temp = p2.begin(); temp != p2.end(); temp++) {
			if( TCPConnEq( con, *temp) ) {
				if(groupid) *groupid = (*it).id;
				if(side) *side = RIGHTSIDE;
				if(rule) *rule = (*it).rule;
				return true;
			}
		}
	}
	return false;
}

/************下面是内部实现部分**********************/

/** 
 * 读取某一组配置中的某一边的所有连接配置
 * 每个连接配置即一个 TCPConn 结构
 * grp_idx：组序号，对应配置文件中的 LEFTX RIGHTX中的X
 * side : 1对应 LEFT 0对应RIGHT
 * conns: 出参，读取的配置放入这个set中
 */
static int cfg_init_conns(int grp_idx, enum GRP_SIDE side, PXCONNS &conns)
{
	const int size = 256;
	char sect[size];
	char key[size];
	char value[size];
	const char* sstr = side == LEFTSIDE ? "LEFT" : "RIGHT";
	sprintf(sect, "%s%d", sstr, grp_idx);

	PDEBUG("in group%d, side %s:\n", grp_idx, sstr);
	/* 读到某一项不存在的时候就退出 */
	int i;
	for(i = 1; ; i++) {
		struct TCPConn conn;
		sprintf(key, "mod%d", i);
		
		if( g_file.getValue(sect, key, value, size) ) {
			int mod = atoi(value);
			switch(mod) {
				case 0:
					conn.mod = CLIENT;
					break;
				case 1:
					conn.mod = SERVER;
					break;
				default:
					printf("Iint config failed, wrong value (group%d, %s side, %s).\n",
							grp_idx, sstr, key);
				return 0;
			}
		} else {
			break;
		}

		sprintf(key, "ip%d", i);
		if( g_file.getValue(sect, key, value, size) )
			conn.ip = std::string(value);
		else 
			break;

		sprintf(key, "port%d", i);
		if( g_file.getValue(sect, key, value, size) )
			conn.port =(unsigned short) atoi(value);
		else 
			break;

		PDEBUG("endpoint%d: mod=%d, ip=%s, port=%u\n", 
				i, conn.mod, conn.ip.c_str(), conn.port);
		conns.push_back(conn);
	}

	return i - 1;
}

/* 根据配置初始化所有的分组信息 
 * 每个组包含： 一个转化规则，两边的配置信息
 */
static int cfg_init_groups()
{
	const int size = 256;
	char key[size];
	char value[size];

	int i;
	for(i = 1; ; i++) {
		
		struct ProxyGroup group;
		group.id = i;

		sprintf(key, "cvtrule%d", i);
		if( g_file.getValue("SYSTEM", key, value, size) ) {
			group.rule = (enum CVT_RULE)atoi(value);
			PDEBUG("%s: %d\n", key, group.rule);
		} else { 
			break;
		}
		
		if( 0 == cfg_init_conns(i, LEFTSIDE, group.left) ) {
			printf("Init config failed (group%d, left side).\n", i);
			return 0;
		}

		if( 0 == cfg_init_conns(i, RIGHTSIDE, group.right) ) {
			printf("Init config failed (group%d, right side).\n", i);
			return 0;
		}
		
		g_config.groups.push_back(group);
	}

	return i-1;
}

/* 填充 g_config.control 中的内容 */
static int cfg_init_control()
{
	const int size = 256;
	char value[size];

	g_config.control.enable_monitor = false;
	sprintf(value, "0");
	if( g_file.getValueWithDefault("SYSTEM", "EnableMonitor",
		 value, size) ) {
		g_config.control.enable_monitor = (bool)atoi(value);
	}

	g_config.control.enable_backup = false;
	sprintf(value, "0");
	if( g_file.getValueWithDefault("SYSTEM", "EnableBackup",
		 value, size) ) {
		g_config.control.enable_backup = (bool)atoi(value);
	}

	g_config.control.cache_live = 60;
	sprintf(value, "60");
	if( g_file.getValueWithDefault("SYSTEM", "CacheLiveSecs",
				value, size) ) {
		g_config.control.cache_live = atoi(value);
	}

	return 1;
}

/* 读取备份相关的配置,并初始化指针g_config.backup 
 * 分配的指针不需要释放，让其存活到系统退出
 */
static void cfg_init_backup()
{
	g_config.backup = new struct BackupConfig;

	const int size = 256;
	char value[size];
	
	g_config.backup->max_backup_num = 10000;
	sprintf(value, "%d", 10000);
	if( g_file.getValueWithDefault("Backup", "MaxBackupNum", 
		value, size) ) 
		g_config.backup->max_backup_num = atoi(value);

	sprintf(value, "%s", "/home/ftp/ProxyBackup/");
	if( g_file.getValueWithDefault("Backup", "BackupDir", 
		value, size) )
		g_config.backup->backup_dir = std::string(value);

	g_config.backup->backup_group_id = 0;
	sprintf(value, "%d", 0);
	if( g_file.getValueWithDefault("Backup", "GroupID", 
		value, size) )
		g_config.backup->backup_group_id = atoi(value);

	g_config.backup->backup_side = RIGHTSIDE;
	sprintf(value, "%d", (int)g_config.backup->backup_side);
	if( g_file.getValueWithDefault("Backup", "BackupSide", 
		value, size) )
		g_config.backup->backup_side = (enum GRP_SIDE) atoi(value);

}

/* 填充g_config中的各个部分
 * 如果没有读取到任何一组配置，则将全局状态设置为false
 */
static void cfg_init()
{
	memset(&g_config, 0, sizeof(g_config));
	
	cfg_init_control();
	if(g_config.control.enable_backup) 
		cfg_init_backup();
	
	int count = cfg_init_groups();

	PDEBUG("total groups: %d \n", count);

	g_file.saveFile();
	g_isconfgok = (bool) count;
}


