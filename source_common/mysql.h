#ifndef GUARD_MYSQL_H
#define GUARD_MYSQL_H

#include <stdio.h>
#include <windows.h>
#include <mysql.h>
#include "console.h"

// structs

typedef struct _tMySQL{
	MYSQL *pConnHandle;
	MYSQL_RES *pResult;
	MYSQL_ROW sRow;
	DWORD dwNumFields;
	DWORD dwNumRows;
	DWORD *pFieldLengths;
} tMySQL;

// functions

void kn_mysql_cleanup(
	IN tMySQL *pMySQL
);

void kn_mysql_close(
	IN tMySQL *pMySQL
);

void kn_mysql_error(
	IN tMySQL *pMySQL
);

tMySQL *kn_mysql_init(
	IN PCHAR szHost,
	IN PCHAR szUser,
	IN PCHAR szPwd,
	IN PCHAR szDB
);

BOOL kn_mysql_insert(
	tMySQL *pMySQL,
	PCHAR szInto,
	PCHAR szFields,
	PCHAR szValues
);

BOOL kn_mysql_call(
	IN tMySQL *pMySQL,
	IN PCHAR szProcName,
	IN BYTE bParamCount,
	IN ...
);

BOOL kn_mysql_select(
	IN tMySQL *pMySQL,
	IN PCHAR szFields,
	IN PCHAR szFrom,
	IN PCHAR szWhere,
	IN PCHAR szOrderBy,
	IN WORD wOffset,
	IN WORD wLimit
);

BOOL kn_mysql_selectFn(
	IN tMySQL *pMySQL,
	IN PCHAR szFnName,
	IN BYTE bParamCount,
	IN ...
);

BOOL kn_mysql_delete(
	IN tMySQL *pMySQL,
	IN PCHAR szFrom,
	IN PCHAR szWhere
);

BOOL kn_mysql_fetch(
	IN tMySQL *pMySQL
);

DWORD kn_mysql_getUInt(
	IN tMySQL *pMySQL,
	IN PCHAR szFieldName
);

void kn_mysql_getStr(
	IN tMySQL *pMySQL,
	IN PCHAR szFieldName,
	OUT PCHAR szOut,
	IN WORD wBfrLen
);

void kn_mysql_getBinary(
	IN tMySQL *pMySQL,
	IN PCHAR szFieldName,
	OUT BYTE *pOut,
	IN WORD wBfrLen
);

#endif // GUARD_MYSQL_H
