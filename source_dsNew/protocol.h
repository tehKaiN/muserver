#ifndef GUARD_PROTOCOL_H
#define GUARD_PROTOCOL_H

#include "../source_common/protocol/common_structs.h"
#include "../source_common/protocol/dg_structs.h"

// structs

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

extern void protocol_getCharacterList(
	IN LPSDHP_GETCHARLIST pMsg,
	IN short iIdx
);

extern void protocol_createCharacter(
	IN LPSDHP_CREATECHAR pMsg,
	IN short iIdx
);

extern void protocol_deleteCharacter(
	IN LPSDHP_CHARDELETE pMsg,
	IN short iIdx
);

extern void protocol_charInfo(
	IN LPSDHP_DBCHARINFOREQUEST pMsg,
	IN short iIdx
);

extern void protocol_charSave(
	IN LPSDHP_DBCHARINFOSAVE pMsg,
	IN short iIdx
);


#endif // GUARD_PROTOCOL_H
