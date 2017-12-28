#ifndef GUARD_PROTOCOL_DG_STRUCTS
#define GUARD_PROTOCOL_DG_STRUCTS

// packet 0x01: GS->DS: get character list request
typedef struct _SDHP_GETCHARLIST {
	PBMSG_HEAD h;
	char szId[MAX_IDSTRING]; // Account ID
	short iNumber;            // GS internal idx of account
} SDHP_GETCHARLIST, *LPSDHP_GETCHARLIST;

// packet 0x01: DS->GS: send character count and special char creation status
typedef struct _SDHP_CHARLISTCOUNT {
	PWMSG_HEAD	h;
	short iNumber;              // GS internal idx of account
	BYTE bCharCount;           // Character count
	long lDbIdx;              // Account DB idx
	BYTE bEnabledMG;           // 1: MG unlocked
	char szId[MAX_IDSTRING+1]; // Account ID
} SDHP_CHARLISTCOUNT, *LPSDHP_CHARLISTCOUNT;

typedef enum {CTLCODE_NORMAL = 0, CTLCODE_YELLOW = 1, CTLCODE_BLUE = 2, CTLCODE_WHITE = 4} tCtlCode;

// packet 0x01: DS->GS: sent for each char immediately after SDHP_CHARLISTCOUNT
typedef struct _SDHP_CHARLIST {
	BYTE bListIdx;                  // Char idx on preview list (0..4)
	char szName[MAX_IDSTRING];      // Name
	WORD wLvl;                      // Level
	BYTE bClass;                    // Class: 16 - DK, ?? - DW, ?? - Elf, ?? - MG
	BYTE bCtlCode;                  //
	union {
		tItemHead pPreviewItems[12];
		BYTE pInventoryBytes[24];     // 2 bytes for each item of inventory
	};
} SDHP_CHARLIST, * LPSDHP_CHARLIST;

// packet 0x04: GS->DS: character creation request
typedef struct
{
	PBMSG_HEAD h;
	long lUserNumber;
	long lDbIdx; // Unused by GS
	short iNumber;
	char szAccName[MAX_IDSTRING];
	char szCharName[MAX_IDSTRING];
	BYTE bClass;
} SDHP_CREATECHAR, *LPSDHP_CREATECHAR;

// packet 0x04: DS->GS: character creation result
typedef struct
{
	PBMSG_HEAD h;
	BYTE bResult;
	short iNumber;
	char szAccName[MAX_IDSTRING];
	char szCharName[MAX_IDSTRING];
	BYTE bListIdx;
	BYTE bClass;
	union {
		tItemHead pPreviewItems[12];
		BYTE pInventoryBytes[24];     // 2 bytes for each item of inventory
	};
} SDHP_CREATECHARRESULT, *LPSDHP_CREATECHARRESULT;

// packet 0x05: GS->DS: character delete request
typedef struct {
	PBMSG_HEAD h;
	short iNumber;
	char szAccName[MAX_IDSTRING];
	char szCharName[MAX_IDSTRING];
	BYTE bInGuild; // TODO(#5): check values for GM, member
	char szGuildName[MAX_GUILDNAMESTRING];
} SDHP_CHARDELETE, *LPSDHP_CHARDELETE;

// packet 0x05: DS->GS: character delete result
typedef struct {
	PBMSG_HEAD h;
	BYTE bResult;
	short iNumber;
	char szAccName[MAX_IDSTRING];
} SDHP_CHARDELETERESULT, *LPSDHP_CHARDELETERESULT;

/**
 * packet 0x06: GS->DS: character data request
 * slightly modified from original:
 *   - accName & charName length shortened from (MAX_IDSTRING+1)
 *   - charName is not encrypted by GS
 */
typedef struct {
	PBMSG_HEAD h;
	char szAccName[MAX_IDSTRING];
	char szCharName[MAX_IDSTRING];
	short Number;
} SDHP_DBCHARINFOREQUEST, *LPSDHP_DBCHARINFOREQUEST;

/**
 * packet 0x06: DS->GS: character data query result
 * slightly modified:
 *   - szCharName shortened to MAX_IDSTRING
 *   - changed Level, Str, Dex, Vit, Energy, MaxLife, MaxMana type to WORD
 *   - changed LevelUpPoint, Exp, Money, PkCount, PkLvl, PkTime type to DWORD
 *   - dropped nextExp field - can be calculeted by GS
 *   - dropped Life and Mana field - always set to max
 */
typedef struct {
	PWMSG_HEAD h;

	BYTE bResult;
	short iNumber;

	char szAccName[MAX_IDSTRING];
	char szCharName[MAX_IDSTRING];
	BYTE bClass;
	BYTE bGmCode;
	WORD wLvl;
	DWORD dwLvlPoints;
	DWORD dwExp;
	DWORD dwZen;
	WORD wStrength;
	WORD wDexterity;
	WORD wVitality;
	WORD wEnergy;
	WORD wLifeMax;
	WORD wManaMax;

	BYTE pInventory[MAX_DBINVENTORY];
	BYTE pMagicList[MAX_DBMAGIC];

	BYTE bMapIdx;
	BYTE bMapX;
	BYTE bMapY;
	BYTE bMapDir;

	DWORD dwPkKills;
	DWORD dwPkLvl;
	DWORD dwPkTime;
} SDHP_DBCHARINFORESULT, *LPSDHP_DBCHARINFORESULT;

/**
 * Packet 0x07: GS->DS: Character data save request, slightly changed:
 *   - Name size shortened to MAX_IDSTRING
 *   - Class, LvlPoints, Money, Str, Dex, Vit, Energy, MaxLife, MaxMana field type changed to WORD
 *   - Exp, Money, PkKills, PkLvl, PkTime field type changed to DWORD
 *   - Removed NextExp field
 */

typedef struct {
	PWMSG_HEAD	h;
	char  szName[MAX_IDSTRING];

	WORD wLvl;
	BYTE bClass;
	WORD wLvlPoints;
	DWORD dwExp;
	DWORD dwZen;
	WORD wStrength;
	WORD wDexterity;
	WORD wVitality;
	WORD wEnergy;
	WORD wLifeMax;
	WORD wManaMax;

	BYTE pInventory[MAX_DBINVENTORY];
	BYTE pSkills[MAX_DBMAGIC];

	BYTE bMapIdx;
	BYTE bMapX;
	BYTE bMapY;
	BYTE bMapDir;

	DWORD dwPkKills;
	DWORD dwPkLvl;
	DWORD dwPkTime;
} SDHP_DBCHARINFOSAVE, *LPSDHP_DBCHARINFOSAVE;


#endif // GUARD_PROTOCOL_DG_STRUCTS
