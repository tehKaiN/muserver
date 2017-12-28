#include "item.h"

tItem item_makeEmpty() {
	tItem sItem;
	ZeroMemory(&sItem, sizeof(tItem));

	sItem.sHead.bItemCode = 0xFF;

	return sItem;
}

void item_makeEmptyInventory(tItem *pItems) {
	BYTE i;
	for(i = 0; i != MAX_DBINVENTORY/10; ++i) {
		pItems[i] = item_makeEmpty();
	}
}

void item_makeEmptyPreviewInventory(tItemHead *pItems) {
	BYTE i;
	for(i = 0; i != 12; ++i) {
		pItems[i].bItemCode = 0xFF;
		pItems[i].bSLOL = 0x00;
	}
}

tItem item_make(BYTE bGroup, BYTE bIdx) {
	tItem sItem;
	ZeroMemory(&sItem, sizeof(tItem));

	sItem.sHead.sItemCode.bGroup = bGroup;
	sItem.sHead.sItemCode.bIdx = bIdx;

	return sItem;
}

void item_writeCode(tItem *pItem, PCHAR szBfr) {
  BYTE i;
  BYTE *pRawItem;
  pRawItem = (BYTE *)pItem;
  for(i = 0; i != ITEMCODE_SIZE; ++i) {
		sprintf(szBfr, "%02X", pRawItem[i]);
		szBfr += 2;
  }
}

tItem pDftItems[4][12];

void item_initDftItems(void) {
	memcpy(pDftItems, (tItem[4][12]) {
		{ // DW
		item_makeEmpty(), item_makeEmpty(),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty()
		},
		{ // DK
		item_make(1, 0), item_makeEmpty(),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty()
		},
		{ // Elf
		item_make(4, 0), item_make(4, 15),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty()
		},
		{ // MG
		item_make(0, 1), item_make(6, 0),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(), item_makeEmpty(),
		item_makeEmpty(), item_makeEmpty(), item_makeEmpty()
		}
	}, 4*12*sizeof(tItem));
}
