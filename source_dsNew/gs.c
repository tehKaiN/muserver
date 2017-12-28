#include "main.h"

BOOL gs_setInfo(long lIdx, WORD wPort, BYTE bType, PCHAR szName) {
	if(lIdx < 0 || lIdx > g_sIOCPSock.pNetClientBase->lMaxClients) {
		g_sConsole.writeError("[GS][setInfo] Index beyond range: 0 < %d < %d ?", lIdx, g_sIOCPSock.pNetClientBase->lMaxClients);
		return 0;
	}
	tGS *pGS;
	pGS = (tGS *)(g_sIOCPSock.pNetClientBase->pNetClients[lIdx]);
	if(!pGS->sNetClient.bConnected) {
		g_sConsole.writeError("[GS][setInfo] GS #%u@%s:%u is not connected", lIdx, pGS->sNetClient.szIP, pGS->sNetClient.wPort);
		return 0;
	}
	if(pGS->sNetClient.bConnected == 2) {
		g_sConsole.writeError("[GS][setInfo] GS #%u@%s:%u info already set", lIdx, pGS->sNetClient.szIP, pGS->sNetClient.wPort);
		return 0;
	}
  pGS->sNetClient.bConnected = 2;
  pGS->sNetClient.wPort = wPort;
  pGS->bType = bType;
  strcpy(pGS->szName, szName);
	g_sConsole.writeSuccess("Server #%u configured to %s@%s:%u ", lIdx, szName, pGS->sNetClient.szIP, wPort);
	return 1;
}
