#include "log.h"
#include "console.h"

// globals
	HANDLE g_hConsoleIn;
	HANDLE g_hConsoleOut;

// sConsole fields

BOOL sConsole_handler(DWORD CEvent) {
	switch(CEvent) {
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
			break;
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			g_sConsole.closeProc();
			break;
		default:
			return FALSE;
	}
	ExitProcess(0);
	return TRUE;
}

void sConsole_init(PCHAR szTitle, void (*closeProc)(void)) {
	g_sConsole.closeProc = closeProc;
	AllocConsole();
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)sConsole_handler, TRUE);
	SetConsoleTitle(szTitle);
	g_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
	g_hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

void sConsole_close(void) {
	g_sConsole.writeSuccess("Console close");
}

void sConsole_writeBinary(BYTE *pBinBfr, DWORD dwLength) {
	char *szStrBfr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 15 + dwLength*3 + 1);
	DWORD i;
	char *pTmpPos;

	if(!szStrBfr) {
		g_sConsole.writeError("[CONSOLE][writeBinary]Out of memory (%u)", dwLength);
	}
	sprintf(szStrBfr, "Bin(%lu): ", dwLength);
	pTmpPos = szStrBfr + strlen(szStrBfr);
	for(i = 0; i != dwLength; ++i) {
		sprintf(pTmpPos, "%02X ", pBinBfr[i]);
		pTmpPos += 3;
	}
	g_sConsole.writeText(szStrBfr);
	HeapFree(GetProcessHeap(), 0, szStrBfr);
}

void sConsole_writeText(PCHAR szText, ...) {
	char szBfr[4096];
	va_list pArgs;

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, "[HH:mm:ss]", szBfr, 255);
	va_start(pArgs, szText);
	vsprintf(&szBfr[strlen(szBfr)], szText, pArgs);
	va_end(pArgs);
	g_sLog.write(szBfr);
	strcat(szBfr, "\n");
	WriteConsole(g_hConsoleOut, szBfr, strlen(szBfr), NULL, NULL);
}

void sConsole_writeSuccess(PCHAR szText, ...) {
	SetConsoleTextAttribute(g_hConsoleOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	char szBfr[255];
	va_list pArgs;
	va_start(pArgs, NULL);
	vsprintf(szBfr, szText, pArgs);
	va_end(pArgs);
	g_sConsole.writeText("%s%s", "[SUCCESS]", szBfr);
	SetConsoleTextAttribute(g_hConsoleOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
};

void sConsole_writeError(PCHAR szText, ...) {
	SetConsoleTextAttribute(g_hConsoleOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
	char szBfr[255];
	va_list pArgs;
	va_start(pArgs, NULL);
	vsprintf(szBfr, szText, pArgs);
	va_end(pArgs);
	g_sConsole.writeText("%s%s", "[ERROR]", szBfr);
	SetConsoleTextAttribute(g_hConsoleOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
};

void sConsole_writeNotice(PCHAR szText, ...) {
	SetConsoleTextAttribute(g_hConsoleOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	char szBfr[255];
	va_list pArgs;
	va_start(pArgs, NULL);
	vsprintf(szBfr, szText, pArgs);
	va_end(pArgs);
	g_sConsole.writeText("%s%s", "[NOTICE]", szBfr);
	SetConsoleTextAttribute(g_hConsoleOut, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
}

// sConsole declaration

tConsole g_sConsole = {
	sConsole_init,
	sConsole_close,
	sConsole_handler,
	sConsole_writeBinary,
	sConsole_writeText,
	sConsole_writeSuccess,
	sConsole_writeError,
	sConsole_writeNotice
};
