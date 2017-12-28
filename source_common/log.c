#include "log.h"

// TODO(#3): Reopen file if date changed

void sLog_init(PCHAR szDir, PCHAR szSuff) {
	char szDateBfr[11];
	char szPathBfr[255];
  GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, "yyyy-MM-dd", szDateBfr, 12);
  CreateDirectory(szDir, NULL);
	wsprintf(szPathBfr, "%s\\%s%s.log", szDir, szDateBfr, szSuff);
	g_sLog.hLog = fopen(szPathBfr, "at");
	if(!g_sLog.hLog)
		MsgBox("Can't create log file at %s", szPathBfr);
	else
		g_sLog.write("\n==== LOG START ====\n");
}

void sLog_close(void) {
		g_sLog.write("\n==== LOG CLOSE ====\n");
	fclose(g_sLog.hLog);
}

void sLog_write(PCHAR szFmt, ...) {
	char szBfr[255];
	va_list pArgs;
	va_start(pArgs, szFmt);
	vsprintf(szBfr, szFmt, pArgs);
	va_end(pArgs);
	strcat(szBfr, "\n");
	fputs(szBfr, g_sLog.hLog);
	fflush(g_sLog.hLog);
}

tLog g_sLog = {
	sLog_init,
	sLog_close,
	sLog_write
};
