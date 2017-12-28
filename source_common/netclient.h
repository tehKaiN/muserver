#ifndef GUARD_NETCLIENT_H
#define GUARD_NETCLIENT_H

#include "windows.h"
#include "winsock2.h"

/*
 * Handling connected clients for given
 */

// defines

#define MAX_BUFF_SIZE 3000

// enums

typedef enum _tIoOpType {
	RECV_IO,
	SEND_IO
} tIoOpType;

// structs

typedef struct _tIOContext {
	OVERLAPPED sOverlapped;
	WSABUF sWSABuff;
	char pBuff1[MAX_BUFF_SIZE];
	char pBuff2[MAX_BUFF_SIZE];
	DWORD dwBuff1Length;
	DWORD dwBuff2Length;
	long lSentBytes;
	tIoOpType lIOOperation;
	long lWaitIO;
} tOverlappedEx;

typedef struct _tSocketContext {
	SOCKET hSocket;        //
	long lClientIdx;       // Index of client on sNetClientBase.pNetClients
	tOverlappedEx sRecvIO; //
	tOverlappedEx sSendIO; //
	DWORD dwIOCount;       // Pending IO operation count
} tSocketContext;        //

typedef struct _tNetClient {
 	tSocketContext sCtxSocket;
	BYTE bConnected; // 0: not connected; 1: connected, not configured; 2: connected, configured
	char szIP[32];   // IP/domain
	WORD wPort;			 // port
} tNetClient;

typedef struct _tNetClientBase {
	// fields
	LONG lMaxClients;
	LONG lOnlineCount;
	LONG lNextIdx;            // internal use
	DWORD dwStructSize; // tNetClient struct size
	tNetClient **pNetClients;

	// event handlers
	void (*onAdd)(IN long lIdx);
	void (*onDel)(IN long lIdx);
} tNetClientBase;

// externs

extern tNetClientBase *netClientBase_create(
	IN long lMaxClients,
	IN DWORD dwStructSize,
	IN void (*onAdd)(IN long lIdx),
	IN void (*onDel)(IN long lIdx)
);
extern void netClientBase_destroy(
	IN tNetClientBase *pNetClientBase
);
extern long netClientBase_add(
	IN tNetClientBase *pNetClientBase,
	IN PCHAR szIP
);
extern void netClientBase_del(
	IN tNetClientBase *pNetClientBase,
	IN long lIdx
);

#endif // GUARD_NETCLIENT_H
