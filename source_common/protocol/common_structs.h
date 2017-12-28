#ifndef GUARD_COMMON_STRUCTS_H
#define GUARD_COMMON_STRUCTS_H

// 0x00 GS->JS/DS: Register server at JS/DS
typedef struct {
	PBMSG_HEAD h;

	BYTE bType;        // 0:Join Server, 1: GameServer, 2: ManagerServer
	WORD wPort;        // Server Port Number
	char szGsName[50]; // Server Name
	BYTE bServerCode;
} SDHP_SERVERINFO, * LPSDHP_SERVERINFO;

#endif // GUARD_COMMON_STRUCTS_H
