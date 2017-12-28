#include "main.h"

void sConfig_init(PCHAR szCmdLine) {
	if(1) // TODO: cmd line
		g_sConfig.readConnectionINI();
	g_sConfig.readINI();
}

void sConfig_close(void) {
}

void sConfig_readConnectionINI(void) {
	g_sConsole.writeNotice("Falling back to .INI");
	char szConfigName[255];
	GetCurrentDirectory(255, szConfigName);
	strcat(szConfigName, "\\dsConfig.ini");

	// DS Listen port
	g_sConfig.dwListenPort = GetIntINI("Connection", "dwListenPort", DEFDATASERVERPORT, szConfigName);
}

void sConfig_readINI(void) {
	char szConfigName[255];
	GetCurrentDirectory(255, szConfigName);
	strcat(szConfigName, "\\dsConfig.ini");
	// SQL
	GetStringINI("SQL", "szDSN", "remu", g_sConfig.szSqlDSN, 255, szConfigName);
	GetStringINI("SQL", "szHost", "127.0.0.1", g_sConfig.szSqlHost, 255, szConfigName);
	GetStringINI("SQL", "szDB", "remu", g_sConfig.szSqlDB, 255, szConfigName);
	GetStringINI("SQL", "szUser", "remu_ds", g_sConfig.szSqlUser, 50, szConfigName);
	GetStringINI("SQL", "szPass", "dupa.8", g_sConfig.szSqlPass, 50, szConfigName);
	// Limits
	g_sConfig.dwMaxServers = GetIntINI("Limits", "dwMaxServers", 4, szConfigName);
	g_sConfig.dwWorkerThreadCount = GetIntINI("Limits", "dwWorkerThreadCount", 0, szConfigName);
	g_sConfig.dwPacketQueueLength = GetIntINI("Limits", "dwPacketQueueLength", 20, szConfigName);
}

tConfig g_sConfig = {
	sConfig_init,
	sConfig_close,
	sConfig_readConnectionINI,
	sConfig_readINI
};
