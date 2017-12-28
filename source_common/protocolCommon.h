#ifndef GUARD_PROTOCOLCOMMON_H
#define GUARD_PROTOCOLCOMMON_H

#include <windows.h>
#include "console.h"

BOOL _protocol_checkSize(
	IN WORD wProtoCode,
	IN DWORD dwStructSize,
	IN BYTE *pData,
	IN DWORD dwPacketSize
);

#define protocol_checkSize(wProtoCode, strct, pData, dwPacketSize) _protocol_checkSize(wProtoCode, sizeof(strct), pData, dwPacketSize)

#endif // GUARD_PROTOCOLCOMMON_H
