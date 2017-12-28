#ifndef GUARD_CONFIG_H
#define GUARD_CONFIG_H

// types

typedef struct _tConfig{
	// functions
	void (*init)(char *szCmdLine);
	void (*close)(void);
	void (*readConnectionINI)(void);
	void (*readINI)(void);

	// Connection
	DWORD dwListenPort;

	// Limits
	DWORD dwWorkerThreadCount;
	DWORD dwMaxServers;
	DWORD dwPacketQueueLength;

	// SQL
	char szSqlDSN[255]; // ODBC only
	char szSqlHost[255]; // MySQL only
	char szSqlDB[255]; // MySQL only
	char szSqlUser[51];
	char szSqlPass[51];

} tConfig;

// functions

// externs

extern tConfig g_sConfig;

#endif // GUARD_CONFIG_H
