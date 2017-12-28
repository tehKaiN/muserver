#include "main.h"

/** \brief Parses param string for JS port and CS ip/port
 *
 * \param szCmdLine PCHAR pointer to param string
 * \return BOOL true if param string was parsed successfully
 *
 */
// TODO(#1): Rewrite
BOOL CmdLineParse(PCHAR szCmdLine) {
	char szTemp[255];
	int index = 0;
	int iTokenCount;
	int iTokenType;
	BOOL	bIsExistConnectServerIP = 0;

	while(szCmdLine[index]) {
		if( szCmdLine[index] == '/' ) {
			index++;
			if( szCmdLine[index] == 'p' ) {	// JS port
				index++;
				iTokenType = 0;
			}
			else if( szCmdLine[index] == 'c' ) {	// connectServer
				index++;
				if( szCmdLine[index] == 'a' ) {	// IP
					index++;
					iTokenType = 1;
				}
				else if( szCmdLine[index] == 'p' ) {	// Port
					index++;
					iTokenType = 2;
				}
			}

			iTokenCount = 0;
			while(1) {
				if( szCmdLine[index] == 0 ) {
					break;
				}
				else if( szCmdLine[index] == ' ') {
					index++;
					break;
				}
				else {
					szTemp[iTokenCount++] = szCmdLine[index++];
				}
			}
			szTemp[iTokenCount++] = 0;
			if( iTokenType == 0 ) {
				sscanf(szTemp, "%hu",&g_sConfig.wCsPort);
			}
			else if( iTokenType == 1 ) {
				sscanf(szTemp, "%s",g_sConfig.szCsIp);
				bIsExistConnectServerIP = 1;
			}
			else if( iTokenType == 2 ) {
				sscanf(szTemp, "%hu",&g_sConfig.wJsPort);
			}
		}
	}
	return bIsExistConnectServerIP;
}

// sConfig functions

void sConfig_init(PCHAR szCmdLine) {
	// read command line for ip and ports
	// if not found then fall back to default
	if( strlen(szCmdLine) < 1 || !CmdLineParse(szCmdLine)) {
		g_sConsole.writeError("Invalid commandline parameters");
		g_sConsole.writeText(
				"Usage: JoinServer.exe /%u /ca127.0.0.1 /cp%u\n"
				"\t/p : JoinServer Port\n"
				"\t/ca : ConnectServer IP\n"
				"\t/cp : ConnectServer Port",
			DEFJOINSERVERPORT,
			DEFCONNSERVERPORT
		);
		g_sConfig.readConnectionINI();
	}
	else
		g_sConsole.writeSuccess("Commandline parameters read successfully");
	g_sConfig.readINI();
	g_sConsole.writeText(
			"Current settings:\n"
			"\tJoinServer Port: %u\n"
			"\tConnectServer IP: %s\n"
			"\tConnectServer Port: %u",
		g_sConfig.wJsPort,
		g_sConfig.szCsIp,
		g_sConfig.wCsPort
);
	// read config files
}

void sConfig_close(void) {

}

void sConfig_readConnectionINI(void) {
	g_sConsole.writeNotice("Falling back to .INI");
	char szConfigName[255];
	GetCurrentDirectory(255, szConfigName);
	strcat(szConfigName, "\\jsConfig.ini");

	// JS port
	g_sConfig.wJsPort = GetIntINI("Connection", "dwJoinServerListenPort", DEFJOINSERVERPORT, szConfigName);

	// CS IP & port
	GetStringINI("Connection", "szConnectServerIP", "127.0.0.1", g_sConfig.szCsIp, 32, szConfigName);
	g_sConfig.wCsPort = GetIntINI("Connection", "dwConnectServerPort", DEFCONNSERVERPORT, szConfigName);
}

void sConfig_readINI(void) {
	char szConfigName[255];
	GetCurrentDirectory(255, szConfigName);
	strcat(szConfigName, "\\jsConfig.ini");
	// SQL
	GetStringINI("SQL", "szDSN", "remu", g_sConfig.szSqlDSN, 255, szConfigName);
	GetStringINI("SQL", "szHost", "127.0.0.1", g_sConfig.szSqlHost, 255, szConfigName);
	GetStringINI("SQL", "szDB", "remu", g_sConfig.szSqlDB, 255, szConfigName);
	GetStringINI("SQL", "szUser", "remu_js", g_sConfig.szSqlUser, 255, szConfigName);
	GetStringINI("SQL", "szPass", "dupa.8", g_sConfig.szSqlPass, 255, szConfigName);
	// Limits
	g_sConfig.dwMaxUsers = GetIntINI("Limits", "dwMaxUsers", 100, szConfigName);
	g_sConfig.dwMaxServers = GetIntINI("Limits", "dwMaxServers", 5, szConfigName);
	g_sConfig.dwWorkerThreadCount = GetIntINI("Limits", "dwWorkerThreadCount", 0, szConfigName);
	// Tweaks
	g_sConfig.dwCreateAcc = GetIntINI("Tweaks", "dwCreateAcc", 1, szConfigName);
}

tConfig g_sConfig = {
	sConfig_init,
	sConfig_close,
	sConfig_readConnectionINI,
	sConfig_readINI
};
