#ifndef GUARD_ITEM_H
#define GUARD_ITEM_H

#include "_defines.h"
#include <stdio.h>
#include <windows.h>

#include "console.h"

// defines
#define ITEMCODE_EMPTY "FF000000000000000000"

// structs

typedef struct _tItemCode{
	BYTE bGroup:4;
	BYTE bIdx:4;
} tItemCode;

typedef struct _tSLOL{
	BYTE bSkill:1;
	BYTE bLvl:4;
	BYTE unk6:1;
	BYTE unk7:1;
	BYTE unk8:1;
} tSLOL;

typedef struct _tPreviewItem{
	// first byte
	union {
		tItemCode sItemCode;
		BYTE bItemCode;
	};
	// second byte
	union {
		tSLOL sSLOL;
		BYTE bSLOL;
	};
} tItemHead;

typedef struct _tItem{
	tItemHead sHead;
  BYTE unk[8];
} tItem;

// externs

extern tItem pDftItems[4][12];

extern tItem item_makeEmpty(void);

extern void item_makeEmptyInventory(
	OUT tItem *pItems
);

extern tItem item_make(
	IN BYTE bGroup,
	IN BYTE bIdx
);

extern void item_makeEmptyPreviewInventory(
	OUT tItemHead *pItems
);

extern void item_writeCode(
	IN tItem *pItem,
	OUT PCHAR szBfr
);

extern void item_initDftItems(void);

#endif // GUARD_ITEM_H
