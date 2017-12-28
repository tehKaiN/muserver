#ifndef GUARD_IOCP_SOCK_H
#define GUARD_IOCP_SOCK_H

#include "windows.h"
#include "winsock2.h"

#include "protocol/heads.h"
#include "console.h"
#include "queue.h"
#include "netclient.h"

/*
 * Structs for handling network IO
 */

typedef struct _tIOCPSock {
	// fields
	BOOL bActive;
	long lWorkerCount;
	DWORD dwRcvQueueSize;
	tNetClientBase *pNetClientBase;
	tQueue *pRcvQueue;

	CRITICAL_SECTION sCriticalSection;
  SOCKET hListenSocket;
	HANDLE hCompletionPort;
	HANDLE hListenThread;
  HANDLE hQueueThread;
  HANDLE *pWorkerThreads;
  void (*protocol_process)(
		IN BYTE bProtoNum,
		IN BYTE *pData,
		IN long lSize,
		IN short iIdx
	);

} tIOCPSock;

// functions

// externs

extern tIOCPSock g_sIOCPSock;

// init / close
extern BOOL IOCPSock_create(
		IN WORD wPort,
		IN DWORD dwRcvQueueLength,
		IN long lWorkerCount,
		IN DWORD dwMaxClients,
		IN DWORD dwNetClientStructSize,
		IN void (*onAdd)(long),
		IN void (*onDel)(long),
		void (*protocol_process)(BYTE, BYTE *, long, short)
);
extern void IOCPSock_destroy(void);

// packet receive/send
extern BOOL parseRcvPacket(
		IN tOverlappedEx *pIO,
		IN BYTE bGsIdx
);
extern BOOL IOCPSock_send1(
		IN long lIdx,
		IN BYTE *pMsg,
		IN DWORD dwSize
);
extern BOOL IOCPSock_send2(
		IN tSocketContext *pCtxSocket
);
extern BOOL IOCPSock_send3(
		IN tSocketContext *pCtxSocket
);
extern BOOL IOCPSock_delClient(
		IN long lIdx;
);
extern BOOL IOCPSock_updateCompletionPort(
	IN SOCKET hSocket,
	IN long lIdx
);
extern BOOL IOCPSock_createWorkers(void);
extern BOOL IOCPSock_createQueueThread(void);
extern BOOL IOCPSock_createListenThread(
	IN WORD wPort
);
extern DWORD WINAPI IOCPSock_listenProc(
	IN void *p
);
extern void WINAPI IOCPSock_queueProc(void);
extern BOOL WINAPI IOCPSock_workerProc(void *pCompletionPort);
#endif // GUARD_IOCP_SOCK_H
