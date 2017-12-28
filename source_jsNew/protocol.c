#include "main.h"

void protocol_process(BYTE bProtoNum, BYTE *pData, long lSize, short iIdx) {
	g_sConsole.writeText("Processing protocol: %u", bProtoNum);
	switch(bProtoNum) {
		case 0x00 :
				protocol_checkSize(bProtoNum, SDHP_SERVERINFO, pData, lSize);
				protocol_serverLogin((LPSDHP_SERVERINFO)pData, iIdx);
			break;
		case 0x01 :
				protocol_checkSize(bProtoNum, SDHP_IDPASS, pData, lSize);
				protocol_userAuth((LPSDHP_IDPASS)pData, iIdx );
			break;
		case 0x02 :
				protocol_checkSize(bProtoNum, SDHP_JOINFAIL, pData, lSize);
				protocol_joinFail((LPSDHP_JOINFAIL)pData, iIdx);
			break;
		case 0x05 :
				protocol_checkSize(bProtoNum, SDHP_USERCLOSE_ID, pData, lSize);
				protocol_disconnectUser((LPSDHP_USERCLOSE_ID)pData, iIdx);
			break;
		default:
			g_sConsole.writeError("[PROTOCOL][process]Unhandled protocol: %u", bProtoNum);
			g_sConsole.writeBinary(pData, lSize);
		}
}


/** \brief Attempts to set GS(/JS/MS) info in JS at given idx
 *
 * \param pMsg LPSDHP_SERVERINFO
 * \param iIdx short g_sServerBase.pServers[] index
 * \return void
 * On return sends SDHP_RESULT packet with dwResult set to:
 * 0x01 - success
 * 0x00 - fail
 */
void protocol_serverLogin(LPSDHP_SERVERINFO pMsg, short iIdx) {
	SDHP_RESULT pResult = {{
			PMHC_BYTE, // bTypeC
			sizeof(pResult), // bSize
			0x00 // bHeadCode
		},
		0x01, // dwResult
		0x00 // dwItemCount
	};
	if(iIdx>>8) {
		g_sConsole.writeError("[PROTOCOL][getJoinInfo]iIdx > 255 (%d)", iIdx);
		// g_sServerBase functions expect BYTE arg, protocol uses short
	}
	if(!gs_setInfo(iIdx, pMsg->wPort, pMsg->bType, pMsg->szGsName)) {
		pResult.bResult = 0x00;
		g_sConsole.writeError("[PROTOCOL][getJoinInfo]g_sServerBase.setInfo fail");
	}
	// wsjServer.DataSend(aIndex, (char*)&pResult, pResult.h.size);
	IOCPSock_send1(iIdx, (BYTE*)&pResult, pResult.h.bSize);
}

/** \brief Informs about failed attempt to join GS by user
 *
 * \param pMsg LPSDHP_JOINFAIL
 * \param iIdx short index on pJsUserBase.pUsers[]
 * \return void
 * calls user deletion from pJsUserBase.pUsers[]
 */
void protocol_joinFail(LPSDHP_JOINFAIL pMsg, short iIdx) {
	g_sJsUserBase.joinFail(pMsg->lUserIdx, pMsg->szId, pMsg->lDBIdx);
}

/** \brief Informs JS that user have been disconnected
 *
 * \param pMsg LPSDHP_USERCLOSE_ID
 * \param iIdx short unused
 * \return void
 * Updates g_sJsUserBase by closing given user
 */
void protocol_disconnectUser(LPSDHP_USERCLOSE_ID pMsg, short iIdx) {
	char szAccountID[MAX_IDSTRING+1];

	szAccountID[MAX_IDSTRING] = '\0';
	memcpy(szAccountID, pMsg->szId, MAX_IDSTRING);

	g_sJsUserBase.delById(szAccountID);
}

/** \brief Query remaining premium play time - internal use by g_sProtocol.auth()
 *
 * \param szId PCHAR account name
 * \param lNumber long
 * \param iIdx short index on pJsUserBase.pUsers[]
 * \return void
 * Implementation is dummy - didn't needed premium stuff
 */
void protocol_internal_billCheck(PCHAR szId, long lNumber, short iIdx) {
	SDHP_BILLSEARCH_RESULT sMsg = {{
			PMHC_BYTE,                      // bTypeC
			sizeof(SDHP_BILLSEARCH_RESULT), // bSize
			0x06                            // bHeadCode
		},
		"",                               // id
		lNumber,                          // Number
		CERTIFYTYPE_ACCOUNT,              // cCertifyType
		BILLTYPE_NOCHARGE,                // PayCode
		{0,0,0,0,0,0,0,0,0,0,0,0},        // EndsDays
		0                                 // EndTime
	};
	memcpy(sMsg.szId, szId, MAX_IDSTRING);
	IOCPSock_send1(iIdx, (BYTE*)&sMsg, sMsg.h.bSize);
}

/** \brief Attepts to log in user by given login/pass/IP
 *
 * \param pMsg LPSDHP_IDPASS
 * \param iIdx short unused
 * \return void
 * On return sends SDHP_IDPASSRESULT with one of results:
 * 0x00 - user not found
 * 0x01 - OK
 * 0x02 - illegal chars
 * 0x03 - already logged in
 * 0x04 - JS full
 * 0x05 - blockCode > '0'
 * 0x09 - bill-related
 * 0x0A - bill-related (JUNGACK)
 * 0x0B - bill-related
 * 0x0C - IP-related (JUNGACK)
 * 0x0D - IP-related
 * 0x0E - blockCode 'A'
 * 0x0F - blockCode 'B'
 * 0xD2 - IP check - Trying Conect In Other Country
 */
void protocol_userAuth(LPSDHP_IDPASS pMsg, short iIdx) {
	static BYTE	pBuxCode[MAX_BUXCCODE] = {0xFC, 0xCF, 0xAB}; // for deXORing login/pass
	static char pIllegalCredChars[MAX_ILLEGALCREDCHARS] = {' ', '\'', '"', '`'};
	BYTE i;

	// Basic credentials
	char szId[MAX_IDSTRING+1];
	char szPass[MAX_IDSTRING+1];
	char szIP[17];
	ZeroMemory(szId, MAX_IDSTRING+1);
	ZeroMemory(szPass, MAX_IDSTRING+1);
	ZeroMemory(szIP, 17);
	memcpy(szId, pMsg->szId, MAX_IDSTRING);
	memcpy(szPass, pMsg->szPass, MAX_IDSTRING);
	memcpy(szIP, pMsg->szIP, 16);

	char szJoominNumber[10]="";
	long lUserIdx = -1;
	DWORD dwDBNumber;

	// deXOR credentials
	for(i = MAX_IDSTRING; i--;) {
		szId[i] ^= pBuxCode[i%MAX_BUXCCODE];
		szPass[i] ^= pBuxCode[i%MAX_BUXCCODE];
	}
	szId[MAX_IDSTRING] = '\0';
	szPass[MAX_IDSTRING] = '\0';


	BYTE bResult=0x01;
	// Illegal chars check
	for(i = MAX_ILLEGALCREDCHARS; bResult == 0x01 && i--;) {
		if(strchr(szId, pIllegalCredChars[i]) || strchr(szPass, pIllegalCredChars[i])) {
			bResult = 0x02;
		}
	}
	if(bResult == 0x01) {
		// Check if account already connected
		if(g_sJsUserBase.searchById(szId) != -1) {
			bResult = 0x03;
		}
	}
	char cBlock = '0';
	if(bResult == 0x01) {
		// Check if account is in DB
		int nRet = g_sDB.isUser(szId, szPass, szJoominNumber, &cBlock, &dwDBNumber);
		if(!nRet) {
			bResult = 0;
		}
		else if(nRet == 0x02) {
			if(g_sConfig.dwCreateAcc) {
				// Account doesn't exist - create one
				if(g_sDB.createAcc(szId, szPass)) {
					bResult = 0x09;
				}
				else {
					bResult = 0x0A;
				}
			}
		}
	}
	if(bResult == 0x01) {
		// Check if account is blocked
		// Simplified - no bill types
		if(cBlock != '0') {
			bResult = 0x05;
		}
	}
	if(bResult == 0x01) {
		// Try adding to g_sUserBase.pUsers[]
		g_sConsole.writeText("Try adding to userBase");
		lUserIdx = g_sJsUserBase.add(szId, szPass, dwDBNumber, iIdx, szIP, pMsg->iNumber);
		if(lUserIdx < 0) {
			bResult = 0x04;
		}
		g_sConsole.writeText("OK");
	}
	// Send packet
	SDHP_IDPASSRESULT	spResult = {{
			PMHC_BYTE,                 // bTypeC
			sizeof(SDHP_IDPASSRESULT), // bSize
			0x01                       // bHeadCode
		},
		bResult,       // bResult
		pMsg->iNumber, // iNumber
		"",            // szId
		lUserIdx,      // iUserNumber
		dwDBNumber,      // iDBIdx
		""             // pSecurityNum
	};
	memcpy(spResult.pSecurityNum, szJoominNumber, 13);
	memcpy(spResult.szId, szId, MAX_IDSTRING);
	IOCPSock_send1(iIdx, (BYTE*)&spResult, spResult.h.bSize);

	if( bResult == 0x01 ) {
		g_sConsole.writeText("User %s@%s logged in", szId, szIP);
		protocol_internal_billCheck(szId, pMsg->iNumber, iIdx);
	}
}

/** \brief Informs GS that someone tries to connect to already connected acc
 *
 * \param bGsIdx BYTE GS index
 * \param szId PCHAR user Id
 * \return void
 * Sends MSG to GS so it can warn online user about login attempt
 */
void protocol_internal_warnConnectedUsr(BYTE bGsIdx, PCHAR szId) {
	SDHP_OTHERJOINMSG sMsg;

	sMsg.h.bTypeC = PMHC_BYTE;
	sMsg.h.bHeadCode = 0x08;
	sMsg.h.bSize = sizeof(sMsg);
	memcpy(sMsg.szId, szId, MAX_IDSTRING);
	IOCPSock_send1(bGsIdx, (BYTE*)&sMsg, sMsg.h.bSize);
}
