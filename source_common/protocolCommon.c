#include "protocolCommon.h"

BOOL _protocol_checkSize(WORD wProtoCode, DWORD dwStructSize, BYTE *pData, DWORD dwPacketSize) {
	if(dwStructSize != dwPacketSize) {
    g_sConsole.writeError("Packet %02X size doesn't match: %u, struct size: %u", wProtoCode, dwPacketSize, dwStructSize);
    g_sConsole.writeBinary(pData, dwPacketSize);
    return 0;
	}
  return 1;
}
