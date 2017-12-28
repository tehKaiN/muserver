#include "iocp_sock.h"

// g_sIOCPListen declaration

BOOL IOCPSock_create(
	WORD wPort, DWORD dwRcvQueueSize, long lWorkerCount,
	DWORD dwMaxClients, DWORD dwNetClientStructSize,
	void (*onAdd)(long), void (*onDel)(long),
	void (*protocol_process)(BYTE, BYTE *, long, short)
) {
	g_sIOCPSock.bActive = 1;
	g_sIOCPSock.lWorkerCount = lWorkerCount;
	g_sIOCPSock.dwRcvQueueSize = dwRcvQueueSize;
	g_sIOCPSock.protocol_process = protocol_process;
	InitializeCriticalSection(&g_sIOCPSock.sCriticalSection);
	// queue
	g_sIOCPSock.pRcvQueue = HeapAlloc(GetProcessHeap(), 0, sizeof(tQueue));
	queue_init(g_sIOCPSock.pRcvQueue, dwRcvQueueSize);
	// netClientBase
	g_sIOCPSock.pNetClientBase = netClientBase_create(dwMaxClients, dwNetClientStructSize, onAdd, onDel);
	if(
		!IOCPSock_updateCompletionPort((SOCKET)INVALID_HANDLE_VALUE, 0) || // Completion port
		!IOCPSock_createQueueThread() ||                                   // Queue thread
		!IOCPSock_createWorkers() ||                                       // Worker threads
		!IOCPSock_createListenThread(wPort)                                // Accept/Listen thread
	){
		IOCPSock_destroy();
		return 0;
	}
	return 1;
}

void IOCPSock_destroy(void) {
	g_sIOCPSock.bActive = 0; // should handle closing all threads
	netClientBase_destroy(g_sIOCPSock.pNetClientBase);
	queue_close(g_sIOCPSock.pRcvQueue);
	HeapFree(GetProcessHeap(), 0, g_sIOCPSock.pRcvQueue);
	DeleteCriticalSection(&g_sIOCPSock.sCriticalSection);
}

// packet receive / send

BOOL IOCPSock_parseRcvPacket(tOverlappedEx *pIO, DWORD dwClientIdx) {
	if( pIO->lSentBytes < 3 ) {
		return 0;
	}

	BYTE *pRecvBuff;
	long lOffs=0;
	short iSize=0;
	BYTE bHeadCode;

	pRecvBuff = (BYTE*)pIO->pBuff1;

	while(1) {
		switch(pRecvBuff[lOffs]) {
			case PMHC_BYTE: { // C1
				LPPBMSG_HEAD pHead = (LPPBMSG_HEAD)&pRecvBuff[lOffs];
				iSize = pHead->bSize;
				bHeadCode = pHead->bHeadCode;
			} break;
			case PMHC_WORD: { // C2
				LPPWMSG_HEAD pHead = (LPPWMSG_HEAD)&pRecvBuff[lOffs];
				iSize = ((WORD)(pHead->bSizeH)<<8);
				iSize |= (WORD)(pHead->bSizeL);
				bHeadCode = pHead->bHeadCode;
			} break;
			default: {
				g_sConsole.writeError("[IOCPSOCK][recvDataParse]Unknown packet type: lOfs:%d, size:%d", lOffs, pIO->lSentBytes);
				g_sConsole.writeBinary(&pRecvBuff[lOffs], pIO->lSentBytes);
				pIO->lSentBytes = 0;
				return 0;
			}
		}
		if( iSize <= 0 ) {
			g_sConsole.writeError("[IOCPSOCK][recvDataParse]Negative size: %d", iSize);
			return 0;
		}
		if( iSize <= pIO->lSentBytes ) {
			queue_push(g_sIOCPSock.pRcvQueue, &pRecvBuff[lOffs], iSize, bHeadCode, dwClientIdx);
			lOffs += iSize;
			pIO->lSentBytes -= iSize;
			if( pIO->lSentBytes <= 0 )
				break;
		}
		else {	// received data not complete
			if( lOffs > 0 ) {
				if( pIO->lSentBytes < 1 )
					g_sConsole.writeError("[IOCPSOCK][recvDataParse]RecvBufLen < 1");
				else if( pIO->lSentBytes < MAX_BUFF_SIZE ) {
					memcpy(pRecvBuff, (pRecvBuff+lOffs), pIO->lSentBytes);
					g_sConsole.writeNotice("[IOCPSOCK][recvDataParse]Message copy %d",pIO->lSentBytes);
				}
			}
			break;
		}
	}
	return 0;
}

BOOL IOCPSock_send1(long lIdx, BYTE *pMsg, DWORD dwSize) {
	EnterCriticalSection(&g_sIOCPSock.sCriticalSection);
	DWORD dwSentBytes;

	if(!g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->bConnected) {
		LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
		return 0;
	}

	// Check if message is too long
	if( dwSize > MAX_BUFF_SIZE ) {
		g_sConsole.writeError("[NETIO][send1]Msg too long (%u)", dwSize);
		netClientBase_del(g_sIOCPSock.pNetClientBase, lIdx);
		LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
		return 0;
	}

	tOverlappedEx *pSendIO = &g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->sCtxSocket.sSendIO;
	if(pSendIO->lWaitIO)	{
		// write IO op has not completed
		if( (pSendIO->dwBuff2Length + dwSize) >= MAX_BUFF_SIZE) {
			g_sConsole.writeError("[NETIO][send2]Buffer 2 overflow!");
			pSendIO->lWaitIO = 0;
			netClientBase_del(g_sIOCPSock.pNetClientBase, lIdx);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 1;
		}
		else {
			// copy MSG to buff2
			memcpy(pSendIO->pBuff2 + pSendIO->dwBuff2Length, pMsg, dwSize);
			pSendIO->dwBuff2Length += dwSize;
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 1;
		}
	}
	else {
		// write IO op completed
		pSendIO->dwBuff1Length = 0;
		if( pSendIO->dwBuff2Length > 0 ) {
			// copy Buff2 to Buff1 (front)
			memcpy(pSendIO->pBuff1, pSendIO->pBuff2, pSendIO->dwBuff2Length);
			pSendIO->dwBuff1Length = pSendIO->dwBuff2Length;
			pSendIO->dwBuff2Length = 0;
		}
		// overflow check
		if( (pSendIO->dwBuff1Length + dwSize) >= MAX_BUFF_SIZE) {
			g_sConsole.writeError("[NETIO][send1]Buffer 1 overflow!");
			pSendIO->lWaitIO = 0;
			netClientBase_del(g_sIOCPSock.pNetClientBase, lIdx);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 0;
		}
		else {
			// copy message to buffer 1
			memcpy(pSendIO->pBuff1 + pSendIO->dwBuff1Length, pMsg, dwSize);
			pSendIO->dwBuff1Length += dwSize;
		}
	}

	pSendIO->sWSABuff.buf	= (char*)pSendIO->pBuff1;
	pSendIO->sWSABuff.len	= pSendIO->dwBuff1Length;
	pSendIO->lSentBytes	= 0;
	pSendIO->lIOOperation	= SEND_IO;

	long lRes = WSASend(
		g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->sCtxSocket.hSocket,
		&(pSendIO->sWSABuff),
		1, &dwSentBytes,
		0, &(pSendIO->sOverlapped), NULL
	);

	if (lRes == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			g_sConsole.writeError("[NETIO][send1]WSASend() failed: %d", WSAGetLastError());
			//pSendIO->nWaitIO = 3;
			netClientBase_del(g_sIOCPSock.pNetClientBase, lIdx);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 0;
		}
	}
	else {
		++g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->sCtxSocket.dwIOCount;
	}
	pSendIO->lWaitIO = 1;
	LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
	return 1;
}

BOOL IOCPSock_send2(tSocketContext *pCtxSocket) {
	EnterCriticalSection(&g_sIOCPSock.sCriticalSection);
	tOverlappedEx *pSendIO = &(pCtxSocket->sSendIO);

	// IO op not complete?
	if(pSendIO->lWaitIO > 0) {
		LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
		return 0;
	}
	// IO ops complete - assume blank buff1
	pSendIO->dwBuff1Length = 0;
	if( pSendIO->dwBuff2Length > 0 ) {
		// copy contents of buff2 to buff1, clear buff2
		memcpy(pSendIO->pBuff1, pSendIO->pBuff2, pSendIO->dwBuff2Length);
		pSendIO->dwBuff1Length = pSendIO->dwBuff2Length;
		pSendIO->dwBuff2Length = 0;
	}
	else {
		// buff2 is empty too - no data to send
		LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
		return 0;
	}

	pSendIO->sWSABuff.buf	= (char*)pSendIO->pBuff1;
	pSendIO->sWSABuff.len	= pSendIO->dwBuff1Length;
	pSendIO->lSentBytes	= 0;
	pSendIO->lIOOperation	= SEND_IO;

	DWORD dwSentBytes;
	long lIdx = pCtxSocket->lClientIdx;
	long lRes = WSASend(
		g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->sCtxSocket.hSocket,
		&(pSendIO->sWSABuff),
		1, &dwSentBytes,
		0, &(pSendIO->sOverlapped), NULL
	);

	if (lRes == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			g_sConsole.writeError("[NETIO][send2]WSASend() failed: %d", WSAGetLastError());
			//pSendIO->nWaitIO = 3;
			netClientBase_del(g_sIOCPSock.pNetClientBase, lIdx);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 0;
		}
	}
	else {
		pCtxSocket->dwIOCount++;
	}
	pSendIO->lWaitIO = 1;
	LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
	return 1;
}

BOOL IOCPSock_send3(tSocketContext *pCtxSocket) {
	EnterCriticalSection(&g_sIOCPSock.sCriticalSection);

	tOverlappedEx *pCtxIOSend = &(pCtxSocket->sSendIO);
	DWORD dwRemainingBytes = pCtxIOSend->dwBuff1Length - pCtxIOSend->lSentBytes;

	if( dwRemainingBytes < 0 ) { // Check for bytes remaining for send
		LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
		return 0;
	}

	g_sConsole.writeNotice("[NETIO][send3]Bytes remaining for sending: %d", dwRemainingBytes);
	pCtxIOSend->sWSABuff.buf	= pCtxIOSend->pBuff1 + pCtxIOSend->lSentBytes;
	pCtxIOSend->sWSABuff.len	= dwRemainingBytes;
	pCtxIOSend->lIOOperation	= SEND_IO;

	DWORD dwBytesSend;
	long lIdx = pCtxSocket->lClientIdx;
	long lRes = WSASend(
		g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->sCtxSocket.hSocket,
		&(pCtxIOSend->sWSABuff),
		1, &dwBytesSend,
		0, &(pCtxIOSend->sOverlapped), NULL
	);

	if (lRes == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			g_sConsole.writeError("[NETIO][send3]SASend() failed: %d", WSAGetLastError());
			netClientBase_del(g_sIOCPSock.pNetClientBase, lIdx);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 0;
		}
	}
	else {
		pCtxSocket->dwIOCount++;
	}
	pCtxIOSend->lWaitIO = 1;
	LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
	return 1;
}

BOOL IOCPSock_updateCompletionPort(SOCKET hSocket, long lIdx) {
	g_sIOCPSock.hCompletionPort = CreateIoCompletionPort(
		(HANDLE)hSocket,
		g_sIOCPSock.hCompletionPort,
		(DWORD)lIdx, 0
	);

	if (g_sIOCPSock.hCompletionPort == NULL) {
		g_sConsole.writeError("[IOCPSOCK][updateCompletionPort] CP create error: %d", GetLastError());
		return 0;
	}
	g_sIOCPSock.pNetClientBase->pNetClients[lIdx]->sCtxSocket.dwIOCount = 0;
	return 1;
}

BOOL IOCPSock_createQueueThread(void) {
	DWORD dwThreadID;
	g_sIOCPSock.hQueueThread = CreateThread(
		NULL, 0,
		(LPTHREAD_START_ROUTINE)IOCPSock_queueProc,
		NULL,0, &dwThreadID
	);
	if (g_sIOCPSock.hQueueThread == NULL) {
		g_sConsole.writeError("[IOCP][proc]CreateThread() failed with error %d\n", GetLastError());
		IOCPSock_destroy();
		return 0;
	}
	return 1;
}

BOOL IOCPSock_createWorkers(void) {
	DWORD dwThreadID;

	// if .ini property <= 0 then calculate using original formula
	if(g_sIOCPSock.lWorkerCount <= 0) {
		SYSTEM_INFO sSysInfo;
		GetSystemInfo(&sSysInfo);
		g_sIOCPSock.lWorkerCount = sSysInfo.dwNumberOfProcessors * 2;
	}

	// workerProc init
	g_sIOCPSock.pWorkerThreads = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_sIOCPSock.lWorkerCount * sizeof(HANDLE *));
	DWORD i;
	for(i = 0; i < g_sIOCPSock.lWorkerCount; ++i) {
		// Create a server worker thread and pass the completion port to the thread.
		g_sIOCPSock.pWorkerThreads[i] = CreateThread(
			NULL, 0,
			(LPTHREAD_START_ROUTINE)IOCPSock_workerProc,
			g_sIOCPSock.hCompletionPort, 0, &dwThreadID
		);
		if (g_sIOCPSock.pWorkerThreads[i] == NULL) {
			g_sConsole.writeError("[NETIO][initWorkers]CreateThread() failed with error %d", GetLastError());
			IOCPSock_destroy();
			return 0;
		}
	}
	return 1;
}

BOOL IOCPSock_createListenThread(WORD wPort) {
	WSADATA sWSAData;
	if(WSAStartup(MAKEWORD(2, 2), &sWSAData) != NO_ERROR) {
		g_sConsole.writeError("[IOCP][createListenThread] WSAStartup() fail: %d", WSAGetLastError());
		return 0;
	}
	g_sIOCPSock.hListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (g_sIOCPSock.hListenSocket == INVALID_SOCKET) {
		g_sConsole.writeError("[IOCPSock][createListenThread] WSASocket() fail: %d", WSAGetLastError());
		return 0;
	}

	SOCKADDR_IN sInternetAddr;
	int nRet;
	sInternetAddr.sin_family = AF_INET;
	sInternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sInternetAddr.sin_port = htons(wPort);

	nRet = bind(g_sIOCPSock.hListenSocket, (PSOCKADDR) &sInternetAddr, sizeof(sInternetAddr));
	if( nRet == SOCKET_ERROR ) {
		g_sConsole.writeError("[IOCPSock][createListenThread] Winsock bind error.");
		return 0;
	}

	nRet = listen(g_sIOCPSock.hListenSocket, 5);
	if (nRet == SOCKET_ERROR) {
		g_sConsole.writeError("[IOCPSock][createListenThread] listen() failed with error %d", WSAGetLastError());
		return 0;
	}
	g_sConsole.writeSuccess("[IOCPSock]Listen socket created at port %u", wPort);

	DWORD dwThreadId;
	g_sIOCPSock.hListenThread = CreateThread(
		NULL, 0,
		IOCPSock_listenProc, NULL,
		0, &dwThreadId
	);
	return 1;
}

DWORD WINAPI IOCPSock_listenProc(void *p) {

	SOCKET hAcceptSocket;
	long lRet;
	long lClientIdx;
	SOCKADDR_IN sAddr;
	IN_ADDR sInAddr;
	int iAddrlen = sizeof(sAddr);

	// temp vars for receiving stuff
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;

	tSocketContext *pCtxSocket = NULL;
	CRITICAL_SECTION *pCrit = &g_sIOCPSock.sCriticalSection;
	g_sConsole.writeSuccess("[IOCPSOCK][listenProc] Init");
	while(g_sIOCPSock.bActive) {
		hAcceptSocket = WSAAccept(g_sIOCPSock.hListenSocket, (LPSOCKADDR)&sAddr, &iAddrlen, NULL, 0);
		g_sConsole.writeText("[IOCPSOCK][listenProc] accept ok");
		if (hAcceptSocket==SOCKET_ERROR) {
			EnterCriticalSection(pCrit);
			g_sConsole.writeError("[IOCPSOCK][listenProc] WSAAccept() fail: %d", WSAGetLastError());
			LeaveCriticalSection(pCrit);
			IOCPSock_destroy();
			return 0;
		}
		g_sConsole.writeText("[IOCPSOCK][listenProc] accept no error");
		EnterCriticalSection(pCrit);
		memcpy( &sInAddr, &sAddr.sin_addr.s_addr, 4 );
		g_sConsole.writeText("[IOCPSOCK][listenProc] adding to list");
		lClientIdx = netClientBase_add(g_sIOCPSock.pNetClientBase, inet_ntoa(sInAddr));
		if( lClientIdx == -1 ) {
			closesocket(hAcceptSocket);
			LeaveCriticalSection(pCrit);
			continue;
		}
		else {
			g_sConsole.writeSuccess("[IOCPSOCK][listenProc] Client #%d added", lClientIdx);
		}
		if(!IOCPSock_updateCompletionPort(hAcceptSocket, lClientIdx)) {
			g_sConsole.writeError("[IOCPSOCK][listenProc] UpdateCompletionPort failed with error %d", GetLastError());
			LeaveCriticalSection(pCrit);
			continue;
		}

		// for shortening dereference
		pCtxSocket = &g_sIOCPSock.pNetClientBase->pNetClients[lClientIdx]->sCtxSocket;
		pCtxSocket->hSocket = hAcceptSocket;
		pCtxSocket->lClientIdx = lClientIdx;

		// TODO(#1): make it more clean
		pCtxSocket->sRecvIO.sWSABuff.buf = pCtxSocket->sRecvIO.pBuff1;
		pCtxSocket->sRecvIO.sWSABuff.len = MAX_BUFF_SIZE;
		pCtxSocket->sRecvIO.dwBuff1Length = 0;
		pCtxSocket->sRecvIO.lSentBytes = 0;
		pCtxSocket->sRecvIO.lWaitIO    = 0;
		pCtxSocket->sRecvIO.dwBuff2Length = 0;
		pCtxSocket->sRecvIO.lIOOperation = RECV_IO;

		// shorted sendIO definition - no reason to make it other way
		pCtxSocket->sSendIO = pCtxSocket->sRecvIO;
		pCtxSocket->sSendIO.lIOOperation = SEND_IO;

		lRet = WSARecv(
			hAcceptSocket,
			&pCtxSocket->sRecvIO.sWSABuff, 1,
			&dwRecvBytes, &dwFlags,
			(OVERLAPPED*)&pCtxSocket->sRecvIO,
			NULL
		);

		if( lRet == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING ) {
			g_sConsole.writeError("[IOCPSOCK][listenProc] WSARecv() failed with error %d", WSAGetLastError());
			pCtxSocket->sRecvIO.lWaitIO = 4;
			netClientBase_del(g_sIOCPSock.pNetClientBase, pCtxSocket->lClientIdx);
			LeaveCriticalSection(pCrit);
			continue;
		}
		else {
			pCtxSocket->sRecvIO.lWaitIO = 1;
			pCtxSocket->dwIOCount++;
		}
		LeaveCriticalSection(pCrit);
	}
	// cleanup
	g_sConsole.writeNotice("[IOCPSOCK][listenProc] Close");
	IOCPSock_destroy();
	return 1;
}

void WINAPI IOCPSock_queueProc(void) {
	BYTE RecvData[1024];
	DWORD dwSize=0;
	BYTE bHeadCode;
	long  lIdx;
	DWORD dwQueueLength;

	while(g_sIOCPSock.bActive) {
		// Check queue length and inform about fillup
		dwQueueLength = queue_getCount(g_sIOCPSock.pRcvQueue);
		if( dwQueueLength > g_sIOCPSock.dwRcvQueueSize-1 ) {
			dwQueueLength = g_sIOCPSock.dwRcvQueueSize-1;
			g_sConsole.writeError("[NETIO][rcvProc]Queue overflow");
		}
		else if(dwQueueLength >= g_sIOCPSock.dwRcvQueueSize*0.9) {
			g_sConsole.writeNotice("[NETIO][rcvProc]Queue load %u/%ui");
		}
		else if( dwQueueLength > 5 ) {
			EnterCriticalSection(&g_sIOCPSock.sCriticalSection);
			g_sConsole.writeText("Queue load: %u/%u", dwQueueLength, g_sIOCPSock.dwRcvQueueSize);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
		}

		// Process queue
		if( dwQueueLength > 0 ) {
			if(queue_pop(g_sIOCPSock.pRcvQueue, (LPBYTE)RecvData, &dwSize, &bHeadCode, &lIdx)) {
				EnterCriticalSection(&g_sIOCPSock.sCriticalSection);
				g_sIOCPSock.protocol_process(bHeadCode, RecvData, dwSize, lIdx);
				LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			}
		}
		else {
			Sleep(1);
		}
	}
	g_sConsole.writeNotice("[NETIO][rcvProc]Thread closed");
}

BOOL WINAPI IOCPSock_workerProc(void *pCompletionPort) {
	DWORD dwIOSize;
	DWORD RecvBytes;
	DWORD Flags;
	BOOL bSuccess = FALSE;
	long lRet;
	DWORD dwClientIdx;

	tSocketContext *pCtxSocket = NULL;
	tOverlappedEx *pIO = NULL;

	while(g_sIOCPSock.bActive) {
		bSuccess = GetQueuedCompletionStatus(
			(HANDLE)pCompletionPort,
			&dwIOSize,
			&dwClientIdx,
			(OVERLAPPED **)&pIO,
			INFINITE
		);

		EnterCriticalSection(&g_sIOCPSock.sCriticalSection);
		if( !bSuccess && pIO != NULL && GetLastError() != 64) {
			g_sConsole.writeError( "[IOCPSOCK][workerProc]GetQueuedCompletionStatus (%d)", GetLastError() );
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			return 0;
		}

		pCtxSocket = &g_sIOCPSock.pNetClientBase->pNetClients[dwClientIdx]->sCtxSocket;
		--pCtxSocket->dwIOCount;
		if(!dwIOSize) {
			netClientBase_del(g_sIOCPSock.pNetClientBase, dwClientIdx);
			LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
			continue;
		}

		if( pIO->lIOOperation == SEND_IO ) {
			pIO->lSentBytes += dwIOSize;

			if(pIO->lSentBytes >= pIO->dwBuff1Length) {
				pIO->lWaitIO = 0;
				if( pIO->dwBuff2Length > 0 )
					IOCPSock_send2(pCtxSocket);
			}
			else
				IOCPSock_send3(pCtxSocket);
		}
		else if( pIO->lIOOperation == RECV_IO ) {
			RecvBytes = 0;
			pIO->lSentBytes += dwIOSize;
			IOCPSock_parseRcvPacket(pIO, dwClientIdx);

			pIO->lWaitIO = 0;
			Flags = 0;
			ZeroMemory(&(pIO->sOverlapped), sizeof(OVERLAPPED));

			pIO->sWSABuff.len		= MAX_BUFF_SIZE-pIO->lSentBytes;
			pIO->sWSABuff.buf		= pIO->pBuff1+pIO->lSentBytes;
			pIO->lIOOperation	= RECV_IO;

			lRet = WSARecv(pCtxSocket->hSocket, &(pIO->sWSABuff), 1, &RecvBytes, &Flags,
				&(pIO->sOverlapped), NULL);
			if( lRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING) ) {
				g_sConsole.writeError("[IOCPSOCK][workerProc]WSARecv() failed with error %d", WSAGetLastError());
				//pIO->nWaitIO = 2;
				netClientBase_del(g_sIOCPSock.pNetClientBase, dwClientIdx);
				LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
				continue;
			}
			else {
				pCtxSocket->dwIOCount++;
				pIO->lWaitIO = 1;
			}
		}
		LeaveCriticalSection(&g_sIOCPSock.sCriticalSection);
	}
	g_sConsole.writeNotice("[IOCPSOCK][workerProc]Thread closed");
	return 1;
}

tIOCPSock g_sIOCPSock;
