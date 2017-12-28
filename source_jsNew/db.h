#ifndef GUARD_DB_H
#define GUARD_DB_H

// enums

typedef enum {MU_AUTH_FAIL, MU_AUTH_OK, MU_AUTH_NOACC} tAuthCode;

// structs

typedef struct _tDB {
	// functions
	BOOL (*init)(void);
	void (*close)(void);

	// MEMB_INFO
  tAuthCode (*isUser)(
		IN PCHAR szId,
		IN PCHAR szPass,
		OUT PCHAR szPersonalID,
		OUT PCHAR pBlockCode,
		OUT DWORD *pDbIdx
	);
	BOOL (*createAcc)(
		IN PCHAR szId,
		IN PCHAR szPass
	);
	BOOL (*blockAcc)(
		IN PCHAR szId,
		IN char cBlockCode
	);

	// MEMB_STAT
	void (*statConnect)(
		IN PCHAR szId,
		IN PCHAR szServerName,
		IN PCHAR szIP
	);
	void (*statDisconnect)(
		IN PCHAR szId
	);

	// fields
	tMySQL *pMySQL;

} tDB;

// functions

// externs
extern tDB g_sDB;

#endif // GUARD_DB_H

