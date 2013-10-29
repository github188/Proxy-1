
#include "../include/Car.h"

#ifdef WITH_BACKUP
#include "../Backup/DataCache.h"
#include "../Backup/BackupModule.h"
#include "../config/config.h"
extern CBackupModule *g_pbackup;
#endif

inline void backup_init()
{
#ifdef WITH_BACKUP
	if(ConfigGet()->control.enable_backup) {
		const struct BackupConfig &cfg = *(ConfigGet()->backup);
		g_pbackup = new CBackupModule(cfg.max_backup_num, 
				cfg.backup_dir);
	} else {
		return;
	}
#endif
}

inline void backup_add(const CCar *pcar)
{
#ifdef WITH_BACKUP
	if( g_pbackup ) 
		g_pbackup->GetDataCache()->Append(pcar);
#endif
}

inline bool backup_remove(const char *packetID)
{
#ifdef WITH_BACKUP
	if( g_pbackup )
		return ( g_pbackup->GetDataCache()->Remove(packetID) );
	else
		return false;
#endif
	return false;
}

