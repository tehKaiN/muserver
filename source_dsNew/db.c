#include "main.h"

void db_create(void) {
  g_pMySQL = kn_mysql_init(g_sConfig.szSqlHost, g_sConfig.szSqlUser, g_sConfig.szSqlPass, g_sConfig.szSqlDB);
}

void db_destroy(void) {
	kn_mysql_close(g_pMySQL);
}

BYTE db_getCharacterPreview(char *szId, SDHP_CHARLIST *pChars) {
	char szWhere[100];
	BYTE bResult;

	sprintf(szWhere, "`acc_name` = '%s'", szId);
	bResult = kn_mysql_select(
		g_pMySQL,
		"`acc_name`, `name`, `lvl`, `class`, `gmCode`, `inventory`, `order`",
		"`character`",
		szWhere,
		"`order` ASC",
		0, 5
	);
	if(!bResult)
		return 0;

	BYTE i;
	WORD j;
	BYTE pInventoryBfr[MAX_DBINVENTORY];
	char szNameTmp[MAX_IDSTRING+1];
	for(i = 0; kn_mysql_fetch(g_pMySQL); ++i) {
		ZeroMemory(szNameTmp, MAX_IDSTRING+1);
		ZeroMemory(pChars[i].szName, MAX_IDSTRING);
    kn_mysql_getStr(g_pMySQL, "name", szNameTmp, MAX_IDSTRING+1);
    memcpy(pChars[i].szName, szNameTmp, MAX_IDSTRING);

		//kn_mysql_getInt(g_pMySQL, "cOrder", pChars[i].bListIdx);
		pChars[i].bListIdx = i;
    pChars[i].wLvl = kn_mysql_getUInt(g_pMySQL, "lvl");
    pChars[i].bClass = kn_mysql_getUInt(g_pMySQL, "class");
    pChars[i].bCtlCode = kn_mysql_getUInt(g_pMySQL, "gmCode");
    kn_mysql_getBinary(g_pMySQL, "inventory", pInventoryBfr, MAX_DBINVENTORY);
    for(j = 0; j != 12; ++j)
			memcpy(pChars[i].pInventoryBytes+(j*2), pInventoryBfr+(j*ITEMCODE_SIZE), 2);
	}

	g_sConsole.writeText("%u chars found for account '%s'", i, szId);
	return i;
}

void db_createChar(LPSDHP_CREATECHAR pRequest, LPSDHP_CREATECHARRESULT pResult) {
	char szBfr[4096];
	char szCharName[MAX_IDSTRING+1] = {0};
	char szAccName[MAX_IDSTRING+1] = {0};
	BYTE bResult = 1;
	BYTE bRet;
	BYTE bSlotIdx;

	memcpy(szCharName, pRequest->szCharName, MAX_IDSTRING);
	memcpy(szAccName, pRequest->szAccName, MAX_IDSTRING);

	// check if name is already used
	sprintf(szBfr, "`name` = '%s'", szCharName);
	bRet = kn_mysql_select(g_pMySQL, "`name`", "`character`", szBfr, NULL, 0, 1);
	if(!bRet) {
		bResult = 0; // TODO(#5): proper result code - char already exists
		g_sConsole.writeNotice("[DB][createChar]Char %s already exists", szCharName);
	}
	if(bResult == 1){
		BYTE pSlots[5] = {0,0,0,0,0};
		// fetch slots taken by existing chars of given acc
		// NOTE(#1): GS is stupid and ignores filling pRequest->lDbIdx
		sprintf(szBfr, "`acc_name` = '%s'", szAccName);
		bRet = kn_mysql_select(g_pMySQL, "`order`", "`character`", szBfr, NULL, 0, 5);
		if(!bRet)
			bSlotIdx = 0;
		else {
			while(kn_mysql_fetch(g_pMySQL)) {
				bSlotIdx = kn_mysql_getUInt(g_pMySQL, "order");
				pSlots[bSlotIdx] = 1;
			}
			// determine free slot
			bSlotIdx = 0;
			while(pSlots[bSlotIdx] && bSlotIdx != 5)
				++bSlotIdx;
			if(bSlotIdx == 5) {
				bResult = 0; // TODO(#5): proper result code - no free slots
				g_sConsole.writeNotice("[DB][createChar]No free slots in account %s", szAccName);
			}
		}
	}
	if(bResult == 1) {
		// insert new char
		BYTE i;
		g_sConsole.writeText("insert: %s", szAccName);
		sprintf(szBfr, "'%s', '%s', %u, %u, 0x", szAccName, szCharName, pRequest->bClass, bSlotIdx);
		// Equipment - class specific
		i = 0;
		while(i != 12) {
			item_writeCode(&pDftItems[pRequest->bClass / 16][i], &szBfr[strlen(szBfr)]);
			++i;
		}
		// Inventory - always same
		while(i != MAX_DBINVENTORY/ITEMCODE_SIZE) {
			strcat(szBfr, ITEMCODE_EMPTY);
			++i;
		}
		// NOTE (KaiN#9): LDate & MDate: current timestamp
		kn_mysql_insert(g_pMySQL, "`character`", "`acc_name`, `name`, `class`, `order`, `inventory`", szBfr);
		if(!bRet) {
			bResult = 0; // TODO(#1): proper result code - other error
			g_sConsole.writeNotice("[DB][createChar]INSERT fail");
		}
		else
			g_sConsole.writeText("Character '%s'@'%s' created", szCharName, szAccName);
	}
	// fill up response packet
	pResult->bClass = pRequest->bClass;
	pResult->bListIdx = (bResult==1? bSlotIdx: 0);
	pResult->bResult = bResult;
	pResult->iNumber = pRequest->iNumber;
	item_makeEmptyPreviewInventory(pResult->pPreviewItems);
	memcpy(pResult->szAccName, pRequest->szAccName, MAX_IDSTRING);
	memcpy(pResult->szCharName, pRequest->szCharName, MAX_IDSTRING);

}

// TODO(#3): proper result codes
BYTE db_deleteChar(LPSDHP_CHARDELETE pMsg) {
	char szWhere[60];
	char szCharName[MAX_IDSTRING+1] = {0};
	char szAccName[MAX_IDSTRING+1] = {0};
	// prep WHERE
	memcpy(szCharName, pMsg->szCharName, MAX_IDSTRING);
	memcpy(szAccName, pMsg->szAccName, MAX_IDSTRING);
	sprintf(szWhere, "`name` = '%s' AND `acc_name` = '%s'", szCharName, szAccName);
	// check if char exists
	if(!kn_mysql_select(g_pMySQL, "`idx`", "`character`", szWhere, NULL, 0, 1)) {
		return 0x00;
	}
	if(!g_pMySQL->dwNumRows) {
		g_sConsole.writeNotice("[DB][deleteChar]Char %s@%s doesn't exist", szCharName, szAccName);
		return 0x00;
	}
	if(pMsg->bInGuild) {
		g_sConsole.writeNotice("[DB][deleteChar]Guild unimplemented!");
		// TODO(#5): guild
		// config: disband guild | assign to assistant | don't allow deletion
	}
	// delete char
	kn_mysql_delete(g_pMySQL, "`character`", szWhere);
	// TODO(#9): Affected rows - return 0 if 0
	g_sConsole.writeText("Deleted char: %s@%s", szCharName, szAccName);
	return 0x01;
}

#define CHARINFO_WHERE_FMT "`name` = '%s' AND `acc_name` = '%s'"

BYTE db_getCharacterInfo(LPSDHP_DBCHARINFOREQUEST pMsg, LPSDHP_DBCHARINFORESULT pResponse) {
	BOOL bRet;
	char szAccName[MAX_IDSTRING+1] = {0};
	char szCharName[MAX_IDSTRING+1] = {0};
	char szWhere[strlen(CHARINFO_WHERE_FMT) + 2*MAX_IDSTRING + 1];

	// WHERE preparation
	memcpy(szAccName,  pMsg->szAccName, MAX_IDSTRING);
	memcpy(szCharName, pMsg->szCharName, MAX_IDSTRING);
	sprintf(szWhere, CHARINFO_WHERE_FMT, szCharName, szAccName);
	// Query
	bRet = kn_mysql_select(g_pMySQL, "`lvl`, `class`, `exp`, `lvlPoints`, `zen`, `strength`, `agility`, `vitality`, `energy`, `lifeMax`, `manaMax`, `inventory`, `skills`, `mapIdx`, `mapX`, `mapY`, `mapDir`, `pkKills`, `pkLvl`, `pkTime`, `gmCode`", "`character`", szWhere, NULL, 0, 1);
	if(!bRet || !g_pMySQL->dwNumRows) {
		g_sConsole.writeError("[DB][getCharacterInfo]Can't find char: %s@%s", szCharName, szAccName);
		return 0;
	}
	kn_mysql_fetch(g_pMySQL);
	// Class, lvl, exp
  pResponse->bClass      = kn_mysql_getUInt(g_pMySQL, "class");
  pResponse->bGmCode      = kn_mysql_getUInt(g_pMySQL, "gmCode");
	pResponse->wLvl        = kn_mysql_getUInt(g_pMySQL, "lvl");
  pResponse->dwLvlPoints = kn_mysql_getUInt(g_pMySQL, "lvlPoints");
  pResponse->dwExp       = kn_mysql_getUInt(g_pMySQL, "exp");
  pResponse->dwZen       = kn_mysql_getUInt(g_pMySQL, "zen");
	// Stats
  pResponse->wStrength  = kn_mysql_getUInt(g_pMySQL, "strength");
  pResponse->wDexterity = kn_mysql_getUInt(g_pMySQL, "agility");
  pResponse->wVitality  = kn_mysql_getUInt(g_pMySQL, "vitality");
  pResponse->wEnergy    = kn_mysql_getUInt(g_pMySQL, "energy");
	// HP & Mana
  pResponse->wLifeMax = kn_mysql_getUInt(g_pMySQL, "lifeMax");
  pResponse->wManaMax = kn_mysql_getUInt(g_pMySQL, "manaMax");
	// Binary fields
  kn_mysql_getBinary(g_pMySQL, "inventory", pResponse->pInventory, MAX_DBINVENTORY);
  kn_mysql_getBinary(g_pMySQL, "skills", pResponse->pMagicList, MAX_DBMAGIC);
	// Map pos
  pResponse->bMapIdx = kn_mysql_getUInt(g_pMySQL, "mapIdx");
  pResponse->bMapX   = kn_mysql_getUInt(g_pMySQL, "mapX");
  pResponse->bMapY   = kn_mysql_getUInt(g_pMySQL, "mapY");
  pResponse->bMapDir = kn_mysql_getUInt(g_pMySQL, "mapDir");
	// PK
  pResponse->dwPkKills = kn_mysql_getUInt(g_pMySQL, "pkKills");
  pResponse->dwPkLvl   = kn_mysql_getUInt(g_pMySQL, "pkLvl");
  pResponse->dwPkTime  = kn_mysql_getUInt(g_pMySQL, "pkTime");
  return 1;
};

tMySQL *g_pMySQL;
