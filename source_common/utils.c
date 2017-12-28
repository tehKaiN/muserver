#include "utils.h"

// ------ INI Functions ------ //

BOOL WINAPI WritePrivateProfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, long lInt, LPCSTR lpFileName) {
  char szBfr[50];
  sprintf(szBfr, "%ld", lInt);
  return WritePrivateProfileString(lpAppName, lpKeyName, szBfr, lpFileName);
}

UINT WINAPI GetIntINI(LPCSTR lpAppName, LPCSTR lpKeyName, UINT uDefault, LPCSTR lpFileName) {
	UINT uRet = GetPrivateProfileInt(lpAppName, lpKeyName, uDefault, lpFileName);
	if(uRet == uDefault) {
		WritePrivateProfileInt(lpAppName, lpKeyName, uRet, lpFileName);
	}
	return uRet;
}

void WINAPI GetStringINI(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpRetStr, DWORD nSize, LPCSTR lpFileName) {
	GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpRetStr, nSize, lpFileName);
	if(!strcmp(lpRetStr, lpDefault)) {
		WritePrivateProfileString(lpAppName, lpKeyName, lpRetStr, lpFileName);
	}
}

// ------ System Functions ------ //

void MsgBox(PCHAR szFmt, ...) {
	char szBfr[255];
	va_list pArgs;
	va_start(pArgs, szFmt);
	vsprintf(szBfr, szFmt, pArgs);
	va_end(pArgs);
	MessageBox(0, szBfr, "Error", MB_OK);
}
