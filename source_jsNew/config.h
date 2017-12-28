#ifndef GUARD_CONFIG_H
#define GUARD_CONFIG_H

// structs
typedef struct _tConfig {
	// functions
	void (*init)(char *szCmdLine);
	void (*close)(void);
	void (*readConnectionINI)(void);
	void (*readINI)(void);

	// fields

	// Connection
	WORD wCsPort;
	char szCsIp[32];
	WORD wJsPort;

	// SQL
	char szSqlDSN[255]; // ODBC only
	char szSqlHost[255]; // MySQL only
	char szSqlDB[255]; // MySQL only
	char szSqlUser[50];
	char szSqlPass[50];

	// Limits
	DWORD dwMaxUsers;
	DWORD dwMaxServers;
	DWORD dwWorkerThreadCount;

	// Tweaks
	DWORD dwCreateAcc;

} tConfig;

// functions

// externs

extern tConfig g_sConfig;

#endif // GUARD_CONFIG_H
