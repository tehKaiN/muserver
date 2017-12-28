#include "main.h"

void protocol_process(BYTE bProtoNum, BYTE *pData, long lSize, short iIdx) {
	switch(bProtoNum) {
		case 0x00: {
			protocol_checkSize(0x00, SDHP_SERVERINFO, pData, lSize);
			protocol_serverLogin((LPSDHP_SERVERINFO)pData, iIdx);
		} break;
		case 0x01: {
			protocol_checkSize(0x01, SDHP_GETCHARLIST, pData, lSize);
			protocol_getCharacterList((LPSDHP_GETCHARLIST)pData, iIdx);
		} break;
		case 0x04: {
			protocol_checkSize(0x04, SDHP_CREATECHAR, pData, lSize);
			protocol_createCharacter((LPSDHP_CREATECHAR)pData, iIdx);
		} break;
		case 0x05: {
			protocol_checkSize(0x05, SDHP_CHARDELETE, pData, lSize);
			protocol_deleteCharacter((LPSDHP_CHARDELETE)pData, iIdx);
		} break;
		case 0x06: {
			protocol_checkSize(0x06, SDHP_DBCHARINFOREQUEST, pData, lSize);
			protocol_charInfo((LPSDHP_DBCHARINFOREQUEST)pData, iIdx);
		} break;
		case 0x07: {
			protocol_checkSize(0x07, SDHP_DBCHARINFOSAVE, pData, lSize);
			protocol_charSave((LPSDHP_DBCHARINFOSAVE)pData, iIdx);
		} break;
		default:
			g_sConsole.writeError("[PROTOCOL][process]Unhandled protocol: %u", bProtoNum);
			g_sConsole.writeBinary(pData, lSize);
	}
}

/** \brief Attempts to set GS(/JS/MS) info in DS at given idx
 *
 * \param pMsg LPSDHP_SERVERINFO
 * \param iIdx short g_sServerBase.pServers[] index
 * \return void
 * On return sends SDHP_RESULT packet with dwResult set to:
 * 0x01 - success
 * 0x00 - fail
 */
void protocol_serverLogin(LPSDHP_SERVERINFO pMsg, short iIdx) {

	SDHP_RESULT sResult = {{
			PMHC_BYTE, // bTypeC
			sizeof(sResult), // bSize
			0x00 // bHeadCode
		},
		0x01, // dwResult
		0x00 // dwItemCount
	};
	if(!gs_setInfo(iIdx, pMsg->wPort, pMsg->bType, pMsg->szGsName)) {
		sResult.bResult = 0x00;
		g_sConsole.writeError("[PROTOCOL][getJoinInfo]g_sServerBase.setInfo fail");
	}
	IOCPSock_send1(iIdx, (BYTE*)&sResult, sResult.h.bSize);
}

void protocol_getCharacterList(LPSDHP_GETCHARLIST pMsg, short iIdx) {
//	g_sConsole.writeText("Char List request for acc: %s(idx:%d)", pMsg->szId, pMsg->iNumber); // DEBUG
	WORD wPacketSize = sizeof(SDHP_CHARLISTCOUNT) + 5*sizeof(SDHP_CHARLIST);
	BYTE *pBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, wPacketSize);
	LPSDHP_CHARLIST pChars = (SDHP_CHARLIST *)(pBuffer + sizeof(SDHP_CHARLISTCOUNT));
  LPSDHP_CHARLISTCOUNT pCharListCount = (LPSDHP_CHARLISTCOUNT)pBuffer;
	BYTE bCharCount = db_getCharacterPreview(pMsg->szId, pChars);

  pCharListCount->h.bTypeC = PMHC_WORD;
  pCharListCount->h.bSizeH = HIBYTE(wPacketSize);
  pCharListCount->h.bSizeL = LOBYTE(wPacketSize);
  pCharListCount->h.bHeadCode = 0x01;

  pCharListCount->iNumber = pMsg->iNumber; // GS internal number
  pCharListCount->lDbIdx = 0;              // DB Idx
  pCharListCount->bCharCount = bCharCount;
	pCharListCount->bEnabledMG = 0;          // UNUSED (0.78-???) MG status
  memcpy(pCharListCount->szId, pMsg->szId, MAX_IDSTRING);
//  g_sConsole.writeText("recvd: '%s', sent: '%s'", pMsg->szId, pCharListCount->szId); // DEBUG
	g_sConsole.writeText("Sending info about %d chars on account %ld", pCharListCount->bCharCount, pCharListCount->lDbIdx);

	IOCPSock_send1(iIdx, pBuffer, wPacketSize);
	HeapFree(GetProcessHeap(), 0, pBuffer);
}

void protocol_createCharacter(LPSDHP_CREATECHAR pMsg, short iIdx) {
	SDHP_CREATECHARRESULT sResponse;
	ZeroMemory(&sResponse, sizeof(SDHP_CREATECHARRESULT));
	// Header
	sResponse.h.bTypeC = PMHC_BYTE;
	sResponse.h.bSize = sizeof(SDHP_CREATECHARRESULT);
	sResponse.h.bHeadCode = 0x04;
	// Message specific
  db_createChar(pMsg, &sResponse);

	IOCPSock_send1(iIdx, (BYTE *)&sResponse, sizeof(SDHP_CREATECHARRESULT));
}

void protocol_deleteCharacter(LPSDHP_CHARDELETE pMsg, short iIdx) {
	SDHP_CHARDELETERESULT sResponse;
	ZeroMemory(&sResponse, sizeof(SDHP_CHARDELETERESULT));
	// Header
	sResponse.h.bTypeC = PMHC_BYTE;
	sResponse.h.bSize = sizeof(SDHP_CHARDELETERESULT);
	sResponse.h.bHeadCode = 0x05;
	// Message specific
	memcpy(sResponse.szAccName, pMsg->szAccName, MAX_IDSTRING);
	sResponse.iNumber = pMsg->iNumber;
	sResponse.bResult = db_deleteChar(pMsg);

	IOCPSock_send1(iIdx, (BYTE *)&sResponse, sizeof(SDHP_CHARDELETERESULT));
}

void protocol_charInfo(LPSDHP_DBCHARINFOREQUEST pMsg, short iIdx) {
  SDHP_DBCHARINFORESULT sResponse;
  ZeroMemory(&sResponse, sizeof(SDHP_DBCHARINFORESULT));
  // Header
  sResponse.h.bTypeC = PMHC_WORD;
  sResponse.h.bSizeH = HIBYTE(sizeof(SDHP_DBCHARINFORESULT));
  sResponse.h.bSizeL = LOBYTE(sizeof(SDHP_DBCHARINFORESULT));
  sResponse.h.bHeadCode = 0x06;
  // Message specific
  sResponse.iNumber = pMsg->Number;
  memcpy(sResponse.szAccName, pMsg->szAccName, MAX_IDSTRING);
  memcpy(sResponse.szCharName, pMsg->szCharName, MAX_IDSTRING);
  sResponse.bResult = db_getCharacterInfo(pMsg, &sResponse);

  IOCPSock_send1(iIdx, (BYTE *)&sResponse, sizeof(SDHP_DBCHARINFORESULT));
}

void protocol_charSave(LPSDHP_DBCHARINFOSAVE pMsg, short iIdx) {
	char szStmt[1000+2*MAX_DBINVENTORY+2*MAX_DBMAGIC+1];
	char szCharName[10] = {0};
	PCHAR pFields[18] = {
		"`lvl`", "`class`", "`lvlPoints`",
		"`exp`", "`zen`", "`strength`",
		"`agility`", "`vitality`", "`energy`",
		"`lifeMax`", "`manaMax`", "`mapIdx`",
		"`mapX`", "`mapY`", "`mapDir`",
		"`pkKills`", "`pkLvl`", "`pkTime`"
	};
	DWORD pValues[18] = {
		pMsg->wLvl, pMsg->bClass, pMsg->wLvlPoints,
		pMsg->dwExp, pMsg->dwZen, pMsg->wStrength,
		pMsg->wDexterity, pMsg->wVitality, pMsg->wEnergy,
		pMsg->wLifeMax, pMsg->wManaMax, pMsg->bMapIdx,
		pMsg->bMapX, pMsg->bMapY, pMsg->bMapDir,
		pMsg->dwPkKills, pMsg->dwPkLvl, pMsg->dwPkTime
	};
	BYTE i;
	tItem *pInventory;
	// char name
	memcpy(szCharName, pMsg->szName, MAX_IDSTRING);
	// statement
	g_sConsole.writeText("Statement create start");
	strcpy(szStmt, "UPDATE `character` SET ");
	for(i = 0; i != 18; ++i) {
		sprintf(szStmt+strlen(szStmt), "%s = %lu, ", pFields[i], pValues[i]);
	}
	sprintf(szStmt+strlen(szStmt), "inventory = 0x");
	pInventory = (tItem *)pMsg->pInventory;
	for(i = 0; i != MAX_DBINVENTORY/ITEMCODE_SIZE; ++i) {
    item_writeCode(&pInventory[i], szStmt+strlen(szStmt));
	}
	strcat(szStmt, ", skills = 0x");
	for(i = 0; i != MAX_DBMAGIC; ++i) {
		sprintf(szStmt+strlen(szStmt), "%02X", pMsg->pSkills[i]);
	}
	sprintf(szStmt+strlen(szStmt), " WHERE `name` = '%s'", szCharName);
	// query
	g_sConsole.writeText("Query exec start");
//	g_sConsole.writeText(szStmt);
  if(mysql_query(g_pMySQL->pConnHandle, szStmt)) {
		kn_mysql_error(g_pMySQL);
		g_sConsole.writeError("[PROTOCOL][charSave]SQL fail for %s", szCharName);
	}
}
