#ifndef GUARD_MAIN_H
#define GUARD_MAIN_H

// ------ Sys include ------ //
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <sql.h>
#include <sqlext.h>

// ------ Common include ------ //
#include "../source_common/_buildver.h"
#include "../source_common/_defines.h"
#include "../source_common/item.h"
#include "../source_common/console.h"
#include "../source_common/iocp_sock.h"
#include "../source_common/log.h"
#include "../source_common/netclient.h"
#include "../source_common/queue.h"
#include "../source_common/mysql.h"
#include "../source_common/utils.h"
#include "../source_common/protocolCommon.h"

// ------ Custom define  ------ //


// ------ Custom include ------ //

#include "config.h"
#include "gs.h"
#include "protocol.h"
#include "db.h"

// ------ Main externs ------ //

extern BOOL init(
	IN LPSTR szCmdLine
);
extern void close(void);

#endif // GUARD_MAIN_H
