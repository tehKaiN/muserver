#ifndef GUARD_DB_H
#define GUARD_DB_H

/*
 * DataServer DB functions
 */

// structs

// externs

extern void db_create(void);
extern void db_destroy(void);
extern BYTE db_getCharacterPreview(
	IN char *szId,
	OUT SDHP_CHARLIST *pChars
);
extern void db_createChar(
	IN LPSDHP_CREATECHAR pRequest,
	OUT LPSDHP_CREATECHARRESULT pResult
);
extern BYTE db_deleteChar(
	IN LPSDHP_CHARDELETE pMsg
);

extern BYTE db_getCharacterInfo(
	IN LPSDHP_DBCHARINFOREQUEST pMsg,
	OUT LPSDHP_DBCHARINFORESULT pResponse
);

extern tMySQL *g_pMySQL;

#endif // GUARD_DB_H

