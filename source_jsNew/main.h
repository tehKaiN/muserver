#ifndef GUARD_MAIN_H
#define GUARD_MAIN_H

// ------ Sys include ------ //
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <mysql.h>

// ------ Common include ------ //

#include "../source_common/utils.h"
#include "../source_common/_buildver.h"
#include "../source_common/_defines.h"
#include "../source_common/console.h"
#include "../source_common/log.h"
#include "../source_common/queue.h"
#include "../source_common/iocp_sock.h"
#include "../source_common/mysql.h"
#include "../source_common/protocolCommon.h"

// ------ Custom define  ------ //

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif // _WIN32_WINNT

// ------ Custom include ------ //
#include "db.h"
#include "config.h"
#include "user.h"
#include "gs.h"
#include "useLog.h"
#include "protocol.h"
#include "udp.h"
#include "serverState.h"
#include "gs.h"

// ------ Main externs ------ //
extern tQueue g_sRecvQueue;

#endif // GUARD_MAIN_H
