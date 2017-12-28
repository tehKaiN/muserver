#ifndef GUARD_UTILS_H
#define GUARD_UTILS_H

#include <stdio.h>
#include <windows.h>

/**
 * Small util functions
 */

// externs
extern BOOL WINAPI WritePrivateProfileInt(
	IN LPCSTR lpAppName,
	IN LPCSTR lpKeyName,
	IN long lInt,
	IN LPCSTR lpFileName
);

extern UINT WINAPI GetIntINI(
	IN LPCSTR lpAppName,
	IN LPCSTR lpKeyName,
	IN UINT uDefault,
	IN LPCSTR lpFileName
);

extern void WINAPI GetStringINI(
	IN LPCSTR lpAppName,
	IN LPCSTR lpKeyName,
	IN LPCSTR lpDefault,
	OUT LPSTR lpRetStr,
	IN DWORD nSize,
	IN LPCSTR lpFileName
);

extern void MsgBox(
	IN PCHAR szFmt,
	IN ...
);

#endif // GUARD_UTILS_H

