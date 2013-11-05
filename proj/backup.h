
#ifndef BACKUP_LH
#define BACKUP_LH

#include "../include/Car.h"

#ifdef CONFIG_BACKUP

#include "../Backup/DataCache.h"
#include "../Backup/BackupModule.h"
#include "../config/config.h"

extern struct CBackupModule *g_pbackup;
#endif

extern void backup_callback(CCar *pcar);

inline void backup_init()
{
#ifdef CONFIG_BACKUP
	if(ConfigGet()->control.enable_backup) {
		const struct BackupConfig &cfg = *(ConfigGet()->backup);
		g_pbackup = new CBackupModule(cfg.max_backup_num, 
				cfg.backup_dir);
		g_pbackup->SetCallBack(backup_callback);
	} else {
		return;
	}
#endif
}

inline void backup_add(const CCar *pcar)
{
#ifdef CONFIG_BACKUP
	if( g_pbackup ) 
		g_pbackup->GetDataCache()->Append(pcar);
#endif
}

inline bool backup_remove(const char *packetID)
{
#ifdef CONFIG_BACKUP
	if( g_pbackup )
		return ( g_pbackup->GetDataCache()->Remove(packetID) );
	else
		return false;
#endif
	return false;
}

#endif

