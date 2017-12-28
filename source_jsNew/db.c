#include "main.h"

// g_sDB functions

BOOL sDB_init(void) {
	g_sDB.pMySQL = kn_mysql_init(g_sConfig.szSqlHost, g_sConfig.szSqlUser, g_sConfig.szSqlPass, g_sConfig.szSqlDB);
  return ((BOOL)g_sDB.pMySQL);
}

void sDB_close(void) {
	if(g_sDB.pMySQL) {
		kn_mysql_close(g_sDB.pMySQL);
		g_sDB.pMySQL = 0;
	}
}

tAuthCode sDB_isUser(PCHAR szId, PCHAR szPass, PCHAR szPersonalID, PCHAR pBlockCode, DWORD *pDbIdx) {
	// sanity check
	if(!strlen(szPass) || !strlen(szId))
		return MU_AUTH_FAIL;

	// Prepare query
	long lRet;
	CHAR szWhere[255];
	sprintf(szWhere, "name = '%s'", szId);
	lRet = kn_mysql_select(g_sDB.pMySQL, "idx, pwd, boxCode, banCode", "account", szWhere, 0, 0, 1);

	// Execute & fetch query
	if(!lRet) {
		g_sConsole.writeNotice("[DB][isUser]User not found: '%s' (exec fail)", szId);
		return MU_AUTH_NOACC;
	}
	lRet = kn_mysql_fetch(g_sDB.pMySQL);
	if(!lRet) {
		g_sConsole.writeNotice("[DB][isUser]User not found: '%s'", szId);
		return MU_AUTH_NOACC;
	}
	// Password check
	char szPwdDB[MAX_IDSTRING+1];
	ZeroMemory(szPwdDB, MAX_IDSTRING+1);
	kn_mysql_getStr(g_sDB.pMySQL, "pwd", szPwdDB, MAX_IDSTRING+1);
	if(!strlen(szPwdDB)) {
		g_sConsole.writeNotice("[DB][isUser]User DB password blank: '%s'", szId);
		return MU_AUTH_FAIL;
	}
	if(strcmp(szPass, szPwdDB)){
		g_sConsole.writeNotice("[DB][isUser]User password mismatch: '%s'", szId);
		return MU_AUTH_FAIL;
	}

	// Pass fields
	char szBlockCode[2]="";
	*pDbIdx = kn_mysql_getUInt(g_sDB.pMySQL, "idx");
	kn_mysql_getStr(g_sDB.pMySQL, "boxCode", szPersonalID, MAX_IDSTRING);
	kn_mysql_getStr(g_sDB.pMySQL, "banCode", szBlockCode, 2);
	*pBlockCode = szBlockCode[0];

	return MU_AUTH_OK;
}

BOOL sDB_createAcc(PCHAR szId, PCHAR szPass) {
	g_sConsole.writeText("create acc start");
	if(!strlen(szPass) || !strlen(szId)) {
		g_sConsole.writeError("[DB][createAcc]Empty login or pwd");
		return 0;
	}

	if(!kn_mysql_selectFn(g_sDB.pMySQL, "JS_CREATE_ACC", 3, szId, szPass, "12345")) {
		g_sConsole.writeError("[DB][createAcc]Couldn't create account: %s", szId);
		return 0;
	}
	g_sConsole.writeText("[DB][createAcc]Created account: %s", szId);
	return 1;
}

BOOL sDB_blockAcc(PCHAR szId, char cBlockCode) {
	char szQuery[255];
	BYTE bRet;
	if(!strlen(szId))
		return 0;
	sprintf(
		szQuery, "UPDATE account SET banCode = '%c' WHERE name = '%s'",
		cBlockCode, szId
	);

	bRet = mysql_query(g_sDB.pMySQL->pConnHandle, szQuery);
	if(bRet) {
		g_sConsole.writeError("[DB][blockAcc]Account %s block fail", szId);
		kn_mysql_error(g_sDB.pMySQL);
		return 0;
	}
	g_sConsole.writeText("[DB][blockAcc]Account %s blocked with code %c", szId, cBlockCode);
	return 1;
}

void sDB_statConnect(PCHAR szId, PCHAR szServerName, PCHAR szIP) {
	if(!kn_mysql_call(g_sDB.pMySQL, "JS_CONNECT", 3, szId, szServerName, szIP))
		g_sConsole.writeError("[DB][statConnect]JS_CONNECT fail");
}

void sDB_statDisconnect(PCHAR szId) {
	if(!kn_mysql_call(g_sDB.pMySQL, "JS_DISCONNECT", 1, szId))
		g_sConsole.writeError("[DB][statDisconnect]JS_DISCONNECT fail");
}

// g_sDB declaration

tDB g_sDB = {
	sDB_init,
	sDB_close,

	sDB_isUser,
	sDB_createAcc,
	sDB_blockAcc,

	sDB_statConnect,
	sDB_statDisconnect
};
