#include "netclient.h"
#include "console.h"

tNetClientBase *netClientBase_create(long lMaxClients, DWORD dwStructSize, void (*onAdd)(long lIdx), void (*onDel)(long lIdx)) {
	tNetClientBase *pNetClientBase;

	pNetClientBase = HeapAlloc(GetProcessHeap(), 0, sizeof(tNetClientBase));
  pNetClientBase->lMaxClients = lMaxClients;
  pNetClientBase->lNextIdx = 0;
  pNetClientBase->dwStructSize = dwStructSize;
  pNetClientBase->onAdd = onAdd;
  pNetClientBase->onDel = onDel;

  pNetClientBase->pNetClients = HeapAlloc(GetProcessHeap(), 0, lMaxClients*sizeof(tNetClient *));
  while(lMaxClients--) {
		pNetClientBase->pNetClients[lMaxClients] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwStructSize);
  }

	return pNetClientBase;
};

void netClientBase_destroy(tNetClientBase *pNetClientBase) {
	long i;
  for(i = pNetClientBase->lMaxClients; i--;) {
		if(pNetClientBase->pNetClients[i]->bConnected)
			netClientBase_del(pNetClientBase, i);
		HeapFree(GetProcessHeap(), 0, pNetClientBase->pNetClients[i]);
  }
  HeapFree(GetProcessHeap(), 0, pNetClientBase->pNetClients);
  HeapFree(GetProcessHeap(), 0, pNetClientBase);
}

long netClientBase_add(tNetClientBase *pNetClientBase, PCHAR szIP) {
	long lIdx = pNetClientBase->lNextIdx;
	tNetClient *pNetClient;
	do {
		pNetClient = pNetClientBase->pNetClients[lIdx];
		if(pNetClient->bConnected == 0) {
			g_sConsole.writeText("[NETCLIENT][add] Found slot");
			ZeroMemory(pNetClient, sizeof(pNetClientBase->dwStructSize));
			pNetClient->bConnected = 1;
			strcpy(pNetClient->szIP, szIP);

			++pNetClientBase->lOnlineCount;
			g_sConsole.writeText("[NETCLIENT][add] OnAdd?");
			if(pNetClientBase->onAdd)
				pNetClientBase->onAdd(lIdx);
			g_sConsole.writeText("[NETCLIENT][add] Done");
			return lIdx;
		}

    if(++lIdx >= pNetClientBase->lMaxClients) {
			lIdx = 0;
    }
  } while(lIdx != pNetClientBase->lNextIdx);
  g_sConsole.writeError("[NETCLIENT][add] Can't add client @%s - clientBase @addr:%p full", szIP, pNetClientBase);
  return -1;
}

void netClientBase_del(tNetClientBase *pNetClientBase, long lIdx) {
	// sanity check
	if(lIdx < 0 || lIdx >= pNetClientBase->lMaxClients) {
		g_sConsole.writeError("[NETCLIENT][del] Index beyond range: 0 < %d < %d ?", lIdx, pNetClientBase->lMaxClients);
		return;
	}
	tNetClient *pNetClient = pNetClientBase->pNetClients[lIdx];
	if(!pNetClient->bConnected) {
		g_sConsole.writeError("[NETCLIENT][del] Client %u@%s:%u already disconnected", lIdx, pNetClient->szIP, pNetClient->wPort);
		return;
	}
	// close GS socket
	if(pNetClient->sCtxSocket.hSocket != INVALID_SOCKET) {
		if( closesocket(pNetClient->sCtxSocket.hSocket) == SOCKET_ERROR ) {
			if(WSAGetLastError() != 10038)
				return;
			else
				pNetClient->sCtxSocket.hSocket = INVALID_SOCKET;
		}
	}
	// remove GS from list
	g_sConsole.writeNotice("[NETCLIENT][del] Client disconnect:  %u@%s:", lIdx, pNetClient->szIP, pNetClient->wPort);
	pNetClient->bConnected = 0;
	--pNetClientBase->lOnlineCount;
	if(pNetClientBase->onDel) {
		pNetClientBase->onDel(lIdx);
	}
}
