
#include "config.h"
#include "ConfFile.h"
#include <pdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct proxyconfig g_config;
static bool g_isconfgok = false;

static CConfFile g_file(CONFIG_FILE);

/** 
 * 读取某一组配置中的某一边的所有连接配置
 * 每个连接配置即一个 tcp_conn 结构
 * grp_idx：组序号，对应配置文件中的 LEFTX RIGHTX中的X
 * side : 1对应 LEFT 0对应RIGHT
 * conns: 出参，读取的配置放入这个set中
 */
static int cfg_init_conns(int grp_idx, int side, PXCONNS &conns)
{
	const int size = 256;
	char sect[size];
	char key[size];
	char value[size];
	const char* sstr = side ? "LEFT" : "RIGHT";
	sprintf(sect, "%s%d", sstr, grp_idx);

	PDEBUG("in group%d, side %s:\n", grp_idx, sstr);
	/* 读到某一项不存在的时候就退出 */
	int i;
	for(i = 1; ; i++) {
		struct tcp_conn conn;
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

static int cfg_init_groups()
{
	const int size = 256;
	char key[size];
	char value[size];

	int i;
	for(i = 1; ; i++) {
		
		struct pxgroup group;

		sprintf(key, "cvtrule%d", i);
		if( g_file.getValue("SYSTEM", key, value, size) ) {
			int rule = atoi(value);
			switch(rule) {
				default:
					group.cvt_rule = RULE_EMPTY;
					break;
			}
			PDEBUG("%s: %d\n", key, group.cvt_rule);
		} else { 
			break;
		}
		
		if( 0 == cfg_init_conns(i, 1, group.left) ) {
			printf("Init config failed (group%d, left side).\n", i);
			return 0;
		}
		if( 0 == cfg_init_conns(i, 0, group.right) ) {
			printf("Init config failed (group%d, right side).\n", i);
			return 0;
		}
		
		g_config.groups.push_back(group);
	
	}

	return i-1;
}

static void cfg_init()
{
	memset(&g_config, 0, sizeof(g_config));
	int count = cfg_init_groups();

	PDEBUG("total groups: %d \n", count);

	g_isconfgok = (bool) count;
}


/*如果初始化成功则返回全局配置项的地址*/
const struct proxyconfig *cfg_get()
{
	if(!g_isconfgok)
		cfg_init();

	if(g_isconfgok)
		return &g_config;
	else
		return NULL;
}
