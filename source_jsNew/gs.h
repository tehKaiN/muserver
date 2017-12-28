#ifndef GUARD_GS_H
#define GUARD_GS_H

/*
 * tNetClient extension for handling GS
 */

#include "../source_common/netclient.h"

// structs

typedef struct _tGS {
	// common with tNetClient
 	tNetClient sNetClient;
	// tGS custom
	char szName[51]; // GS name
	BYTE bType;			 // 0:JoinServer, 1:MapServer, 2: ManagerServer
} tGS;

// externs
extern void gs_onServerDel(
	IN long lGsIdx
);
extern BOOL gs_setInfo(
	IN long lIdx,
	IN WORD wPort,
	IN BYTE bType,
	IN PCHAR szName
);

#endif // GUARD_GS_H
