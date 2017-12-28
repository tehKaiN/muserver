#ifndef GUARD_PROTOCOL_H
#define GUARD_PROTOCOL_H

// specific defines
#define MAX_BUXCCODE 3
#define MAX_ILLEGALCREDCHARS 4

// specific includes
#include "../source_common/protocol/jg_structs.h"
#include "../source_common/protocol/common_structs.h"

// structs

// functions

// externs

extern void protocol_process(
	IN BYTE bProtoNum,
	IN BYTE *pData,
	IN long lSize,
	IN short iIdx
);
extern void protocol_serverLogin(
	IN LPSDHP_SERVERINFO pMsg,
	IN short iIdx
);
extern void protocol_userAuth(
	IN LPSDHP_IDPASS pMsg,
	IN short iIdx
);
extern void protocol_internal_warnConnectedUsr(
	IN BYTE bGsIdx,
	IN PCHAR szId
);
extern void protocol_internal_billCheck(
	IN PCHAR szId,
	IN long lNumber,
	IN short iIdx
);
extern void protocol_joinFail(
	IN LPSDHP_JOINFAIL pMsg,
	IN short iIdx
);
extern void protocol_disconnectUser(
	IN LPSDHP_USERCLOSE_ID pMsg,
	IN short iIdx
);

#endif // GUARD_PROTOCOL_H
