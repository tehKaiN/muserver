#include "mysql.h"

void kn_mysql_cleanup(tMySQL *pMySQL) {
	if(pMySQL->pResult)
		mysql_free_result(pMySQL->pResult);
	pMySQL->pResult = 0;
	pMySQL->dwNumFields = 0;
	pMySQL->dwNumRows = 0;
}

void kn_mysql_close(tMySQL *pMySQL) {
	kn_mysql_cleanup(pMySQL);
	if(pMySQL->pConnHandle)
		mysql_close(pMySQL->pConnHandle);
	HeapFree(GetProcessHeap(), 0, pMySQL);
}

void kn_mysql_error(tMySQL *pMySQL) {
	g_sConsole.writeError("%s", mysql_error(pMySQL->pConnHandle));
}

tMySQL *kn_mysql_init(PCHAR szHost, PCHAR szUser, PCHAR szPwd, PCHAR szDB) {
	tMySQL *pMySQL = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(tMySQL));
	pMySQL->pConnHandle = mysql_init(NULL);
	if(pMySQL->pConnHandle == NULL) {
		kn_mysql_error(pMySQL);
		return 0;
	}
	if(!mysql_real_connect(pMySQL->pConnHandle, szHost, szUser, szPwd, szDB, 0, NULL, 0)) {
		kn_mysql_error(pMySQL);
		kn_mysql_close(pMySQL);
		return 0;
	}

	return pMySQL;
}

#define INSERT_STMT_FMT ("INSERT INTO %s (%s) VALUES (%s)")

BOOL kn_mysql_insert(tMySQL *pMySQL, PCHAR szInto, PCHAR szFields, PCHAR szValues) {
	// cleanup before exec
	kn_mysql_cleanup(pMySQL);
  char szStmt[strlen(INSERT_STMT_FMT) + strlen(szInto) + strlen(szFields) + strlen(szValues) + 1];
  sprintf(szStmt, INSERT_STMT_FMT, szInto, szFields, szValues);

  kn_mysql_cleanup(pMySQL);
	// statement exec
	if(mysql_query(pMySQL->pConnHandle, szStmt)) {
		kn_mysql_error(pMySQL);
		g_sConsole.writeError("[MYSQL][INSERT]Failed at: %s", szStmt);
		return 0;
	}
	return 1;
}

BOOL kn_mysql_call(tMySQL *pMySQL, PCHAR szProcName, BYTE bParamCount, ...) {
	// cleanup before exec
	kn_mysql_cleanup(pMySQL);
	char szStmt[1024];
	BYTE i;
	va_list pArgs;

	sprintf(szStmt, "CALL %s(", szProcName);
	va_start(pArgs, bParamCount);
	for(i = 0; i != bParamCount; ++i) {
		sprintf(&szStmt[strlen(szStmt)], "'%s'", va_arg(pArgs, PCHAR));
		if(i != bParamCount-1)
			strcat(szStmt, ", ");
	}
	va_end(pArgs);
	strcat(szStmt, ")");
	if(mysql_query(pMySQL->pConnHandle, szStmt)) {
		kn_mysql_error(pMySQL);
		g_sConsole.writeError("[MYSQL][CALL]Failed at: %s", szStmt);
		return 0;
	}
	return 1;
}

BOOL kn_mysql_select(tMySQL *pMySQL, PCHAR szFields, PCHAR szFrom, PCHAR szWhere, PCHAR szOrderBy, WORD wOffset, WORD wLimit) {
	// cleanup before exec
	kn_mysql_cleanup(pMySQL);
	// statement prepare
  char szStmt[4096];
  sprintf(szStmt, "SELECT %s FROM %s WHERE %s", szFields, szFrom, szWhere);
  if(szOrderBy) {
		strcat(szStmt, " ORDER BY ");
		strcat(szStmt, szOrderBy);
  }
  if(wLimit)
		sprintf(&szStmt[strlen(szStmt)], " LIMIT %u", wLimit);
  if(wOffset)
		sprintf(&szStmt[strlen(szStmt)], " OFFSET %u", wOffset);
	// statement exec
	if(mysql_query(pMySQL->pConnHandle, szStmt)) {
		kn_mysql_error(pMySQL);
		g_sConsole.writeError("[MYSQL][SELECT]Failed at: %s", szStmt);
		return 0;
	}
	pMySQL->pResult = mysql_store_result(pMySQL->pConnHandle);
	pMySQL->dwNumFields = mysql_num_fields(pMySQL->pResult);
	pMySQL->dwNumRows = mysql_num_rows(pMySQL->pResult);
	return 1;
}

BOOL kn_mysql_selectFn(tMySQL *pMySQL, PCHAR szFnName, BYTE bParamCount, ...) {
	// cleanup before exec
	kn_mysql_cleanup(pMySQL);
	// statement prepare
  char szStmt[4096];
  BYTE i;
  va_list pArgs;
  sprintf(szStmt, "SELECT %s(", szFnName);
	va_start(pArgs, bParamCount);
	for(i = 0; i != bParamCount; ++i) {
		sprintf(&szStmt[strlen(szStmt)], "'%s'", va_arg(pArgs, PCHAR));
		if(i != bParamCount-1)
			strcat(szStmt, ", ");
	}
	va_end(pArgs);
	strcat(szStmt, ")");

	if(mysql_query(pMySQL->pConnHandle, szStmt)) {
		kn_mysql_error(pMySQL);
		g_sConsole.writeNotice("SELECT FN %s: fail", szFnName);
		g_sConsole.writeError("[MYSQL][SELECTFN]Failed at: %s", szStmt);
		return 0;
	}
	pMySQL->pResult = mysql_store_result(pMySQL->pConnHandle);
	pMySQL->dwNumFields = mysql_num_fields(pMySQL->pResult);
	pMySQL->dwNumRows = mysql_num_rows(pMySQL->pResult);
	return 1;
}

#define DELETE_STMT_FMT ("DELETE FROM %s WHERE %s")

BOOL kn_mysql_delete(tMySQL *pMySQL, PCHAR szFrom, PCHAR szWhere) {
	char szStmt[strlen(DELETE_STMT_FMT) + strlen(szFrom) + strlen(szWhere) + 1];
	sprintf(szStmt, DELETE_STMT_FMT, szFrom, szWhere);

	if(mysql_query(pMySQL->pConnHandle, szStmt)) {
		kn_mysql_error(pMySQL);
		g_sConsole.writeError("[MYSQL][DELETE]Failed at: %s", szStmt);
		return 0;
	}
	return 1;
}

BOOL kn_mysql_fetch(tMySQL *pMySQL) {
  pMySQL->sRow = mysql_fetch_row(pMySQL->pResult);
	if(pMySQL->sRow)
		pMySQL->pFieldLengths = mysql_fetch_lengths(pMySQL->pResult);
	return (BOOL)(pMySQL->sRow);
}

DWORD kn_mysql_getUInt(tMySQL *pMySQL, PCHAR szFieldName) {
	DWORD dwOut;
	DWORD i;
	PCHAR szBfr;
	MYSQL_FIELD *pField;

	mysql_field_seek(pMySQL->pResult, 0);
	for(i = 0; i != pMySQL->dwNumFields; ++i) {
			pField = mysql_fetch_field(pMySQL->pResult);
			if(!strcmp(pField->name, szFieldName)) {
				szBfr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pMySQL->pFieldLengths[i]);
				memcpy(szBfr, pMySQL->sRow[i], pMySQL->pFieldLengths[i]);
				sscanf(szBfr, "%lu", &dwOut);
				HeapFree(GetProcessHeap(), 0, szBfr);
				return dwOut;
			}
	}
	g_sConsole.writeNotice("Couldn't find field: %s", szFieldName);
	return 0;
}

void kn_mysql_getStr(tMySQL *pMySQL, PCHAR szFieldName, PCHAR szOut, WORD wBfrLen) {
	DWORD i;
	MYSQL_FIELD *pField;

	mysql_field_seek(pMySQL->pResult, 0);
	for(i = 0; i != pMySQL->dwNumFields; ++i) {
			pField = mysql_fetch_field(pMySQL->pResult);
			if(!strcmp(pField->name, szFieldName)) {
				if(pMySQL->pFieldLengths[i] > wBfrLen)
					g_sConsole.writeNotice("Field buffer too short: got %u, expected: %u", wBfrLen, pMySQL->pFieldLengths[i]);
				else
					memcpy(szOut, pMySQL->sRow[i], pMySQL->pFieldLengths[i]);
				return;
			}
	}
	g_sConsole.writeNotice("Couldn't find field: %s", szFieldName);
}

void kn_mysql_getBinary(tMySQL *pMySQL, PCHAR szFieldName, BYTE *pOut, WORD wBfrLen) {
	DWORD i;
	MYSQL_FIELD *pField;

	mysql_field_seek(pMySQL->pResult, 0);
	for(i = 0; i != pMySQL->dwNumFields; ++i) {
			pField = mysql_fetch_field(pMySQL->pResult);
			if(!strcmp(pField->name, szFieldName)) {
				if(pMySQL->pFieldLengths[i] > wBfrLen)
					g_sConsole.writeNotice("Field buffer too short: got %u, expected: %u", wBfrLen, pMySQL->pFieldLengths[i]);
				else
					memcpy(pOut, pMySQL->sRow[i], pMySQL->pFieldLengths[i]);
				return;
			}
	}
	g_sConsole.writeNotice("Couldn't find field: %s", szFieldName);
}
