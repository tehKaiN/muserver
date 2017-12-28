#include "main.h"

// g_sUseLog fields


/** \brief Checks if active log file corresponds with current date. If not, creates new.
 *
 * \param void
 * \return void
 *
 */
void sUseLog_refreshDate(void) {
	char szDateBfr[11];
	char szPathBfr[50];
	GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, "yyyy-MM-dd", szDateBfr, 12);
	if(lstrcmpi(g_sUseLog.szDateBfr, szDateBfr)) {
		if(g_sUseLog.hLog) {
			fclose(g_sUseLog.hLog);
		}
		sprintf(szPathBfr, "jsLog\\%s_use.log", szDateBfr);
		strcpy(g_sUseLog.szDateBfr, szDateBfr);
		g_sUseLog.hLog = fopen(szPathBfr, "a+t");
	}

}

void sUseLog_init(void) {
	g_sUseLog.bEnabled = 1;
	g_sUseLog.szDateBfr[0] = 0;
	g_sUseLog.hLog = 0;
}

void sUseLog_close(void) {
	if(g_sUseLog.hLog) {
		fclose(g_sUseLog.hLog);
		g_sUseLog.hLog = 0;
	}
}

void sUseLog_connect(PCHAR szServerName, PCHAR szId, PCHAR szIP) {
	if(!g_sUseLog.bEnabled)
		return;
	g_sUseLog.refreshDate();
	char szBfr[255];
	sprintf(
		szBfr, "[%s][CONNECT]GS %s: Player %s@%s\n",
		g_sUseLog.szDateBfr, szServerName, szId, szIP
	);
	fputs(szBfr, g_sUseLog.hLog);
	fflush(g_sUseLog.hLog);
}

void sUseLog_disconnect(PCHAR szServerName, PCHAR szId, PCHAR szIP, DWORD dwPlayTime) {
	if(!g_sUseLog.bEnabled)
		return;
	g_sUseLog.refreshDate();
	char szBfr[255];
	sprintf(
		szBfr, "[%s][DISCONNECT]GS %s: Player %s@%s after %lu minutes\n",
		g_sUseLog.szDateBfr, szServerName, szId, szIP, dwPlayTime
	);
	fputs(szBfr, g_sUseLog.hLog);
	fflush(g_sUseLog.hLog);
}

// g_sUseLog declaration

tUseLog g_sUseLog = {
	sUseLog_init,
	sUseLog_close,
	sUseLog_refreshDate,
	sUseLog_connect,
	sUseLog_disconnect
};
