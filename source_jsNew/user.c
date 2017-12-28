#include "main.h"

// misc functions

// g_sJsUserBase functions
void sJsUserBase_init(void) {
  WORD i;
  g_sJsUserBase.dwNextUserIdx = 0;
  g_sJsUserBase.dwTotalCount = 0;
  g_sJsUserBase.pUsers = HeapAlloc(GetProcessHeap(), 0, g_sConfig.dwMaxUsers * sizeof(tJsUser*));
	for(i = g_sConfig.dwMaxUsers; i--;) {
		g_sJsUserBase.pUsers[i] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(tJsUser));
	}
	g_sConsole.writeSuccess("[USER][init]Allocated %u slots for users", g_sConfig.dwMaxUsers);
}

void sJsUserBase_close(void) {
	if(g_sJsUserBase.pUsers){
		WORD i;
		for(i = g_sConfig.dwMaxUsers; i--;) {
			HeapFree(GetProcessHeap(), 0, g_sJsUserBase.pUsers[i]);
		}
		HeapFree(GetProcessHeap(), 0, g_sJsUserBase.pUsers);
		g_sJsUserBase.pUsers = 0;
	}
}

long sJsUserBase_add(PCHAR szId, PCHAR szPass, long iDBIdx, BYTE bGsIdx, PCHAR szIP, int iGaIdx) {
	long i;
	tJsUser *pUser;
	i = g_sJsUserBase.dwNextUserIdx;
  do {
    if(!g_sJsUserBase.pUsers[i]->bConnected) {
			pUser = g_sJsUserBase.pUsers[i];
			ZeroMemory(pUser, sizeof(tJsUser));

			pUser->bConnected = 1;
			strcpy(pUser->szId, szId);
			strcpy(pUser->szIP, szIP);
			pUser->iDBIdx = iDBIdx;
			pUser->bGsIdx = bGsIdx;
			pUser->iGaIdx = iGaIdx;
			pUser->dwPlayTime = GetTickCount();

			g_sJsUserBase.dwNextUserIdx = i+1;
			++g_sJsUserBase.dwTotalCount;
			g_sConsole.writeText("[USER][add]User %s added", szId);

			tGS *pGS = (tGS *)g_sIOCPSock.pNetClientBase->pNetClients[bGsIdx];
			// UseLog
			g_sUseLog.connect(
				pGS->szName, // Server Name
				pUser->szId, 												    // Login
				pUser->szIP 												    // IP Address
			);
			g_sConsole.writeText("[USER][add]UseLog updated");

			// MembStat - related with MuManager according to kor comments
			g_sDB.statConnect(pUser->szId, pGS->szName, pUser->szIP);
			g_sConsole.writeText("[USER][add]statConnect pass");
			return i;
    }

    if(++i >= g_sConfig.dwMaxUsers) {
			i = 0;
    }
  } while(i != g_sJsUserBase.dwNextUserIdx);
  g_sConsole.writeNotice("[USER][add]Can't connect user %s@%s - JoinServer full", szId, szIP);
	return -1;
}

BOOL sJsUserBase_delById(PCHAR szId) {
  long idx = g_sJsUserBase.searchByIdRet(szId);
  if(idx == -1) {
		g_sConsole.writeError("[USER][delById] Can't delete user %s - not found", szId);
		return 0;
  }
  return g_sJsUserBase.delByIdx(idx, g_sJsUserBase.pUsers[idx]->iDBIdx);
}

BOOL sJsUserBase_delByIdx(long iIdx, long iDBIdx) {
  if(iIdx < 0 || iIdx > g_sConfig.dwMaxUsers) {
		g_sConsole.writeError("[USER][delByIdx] Index beyond range: 0 <= %d < %d ?", iIdx, g_sConfig.dwMaxUsers);
		return 0;
  }
  tJsUser *pUser = g_sJsUserBase.pUsers[iIdx];
  if(iDBIdx != pUser->iDBIdx) {
		g_sConsole.writeError("[USER][delByIdx] DB Number mismatch: %s vs %s", iDBIdx, pUser->iDBIdx);
		return 0;
  }

  DWORD dwPlayTime = ((GetTickCount()-pUser->dwPlayTime)/1000)/60;

	pUser->bConnected = FALSE;
  g_sConsole.writeText("User closed: %s@%s after %umin of play", pUser->szId, pUser->szIP, dwPlayTime);

	// UseLog
	tGS *pGS = (tGS*)g_sIOCPSock.pNetClientBase->pNetClients[pUser->bGsIdx];
	g_sUseLog.disconnect(
		pGS->szName, // Server Name
		pUser->szId,                                   // Login
		pUser->szIP,                                   // IP Address
		dwPlayTime                                     // Play time
	);
	g_sDB.statDisconnect(pUser->szId);
	return 1;
}

long sJsUserBase_searchById(PCHAR szId) {
	long lIdx = g_sJsUserBase.searchByIdRet(szId);
	if(lIdx != -1) {
		 protocol_internal_warnConnectedUsr(g_sJsUserBase.pUsers[lIdx]->bGsIdx, szId);
	}
	return lIdx;
}

long sJsUserBase_searchByIdRet(PCHAR szId) {
	long i;
	tJsUser *pUser;
	for(i = 0; i < g_sConfig.dwMaxUsers; ++i) {
		pUser = g_sJsUserBase.pUsers[i];
		if(pUser->bConnected) {
			if( pUser->szId[0] == szId[0] ) {
				if(lstrcmpi( pUser->szId, szId) == 0) {
					return i;
				}
			}
		}
	}
	if(!g_sConfig.dwCreateAcc)
		g_sConsole.writeNotice("[USER][searchByIdRet] User '%s' not found", szId);
	return -1;
}

BOOL sJsUserBase_joinFail(long iIdx, PCHAR szId, long iDBIdx) {
  g_sConsole.writeNotice("Authentication fail, disconnecting user %s", szId);
	g_sJsUserBase.delByIdx(iIdx, iDBIdx);
	return 1;
}

BOOL sJsUserBase_accountBlock(long iIdx, PCHAR szId, long iDBIdx, char cBlockCode) {
  if(iIdx < 0 || iIdx > g_sConfig.dwMaxUsers) {
		g_sConsole.writeError("[USER][accountBlock] Index beyond range: 0 <= %d < %d ?", iIdx, g_sConfig.dwMaxUsers);
		return 0;
  }
  tJsUser *pUser = g_sJsUserBase.pUsers[iIdx];
  if(iDBIdx != pUser->iDBIdx) {
		g_sConsole.writeError("[USER][accountBlock] DB Number mismatch: %s vs %s", iDBIdx, pUser->iDBIdx);
		return 0;
  }
  if(lstrcmpi(szId, pUser->szId)) {
		g_sConsole.writeError("[USER][accountBlock] ID Mismatch: %s vs %s", szId, pUser->szId);
		return 0;
  }

	g_sDB.blockAcc(szId, cBlockCode);
	return 1;
}

void sJsUserBase_proc(void) {

}


// g_sJsUserBase definition
tJsUserBase g_sJsUserBase = {
	sJsUserBase_init,
	sJsUserBase_close,

	sJsUserBase_add,
	sJsUserBase_delById,
	sJsUserBase_delByIdx,
	sJsUserBase_searchById,
	sJsUserBase_searchByIdRet,
	sJsUserBase_joinFail,
	sJsUserBase_accountBlock,
	sJsUserBase_proc
};
