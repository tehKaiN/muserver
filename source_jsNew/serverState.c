#include "main.h"

// tServerState-specific functions

void sServerState_init(void) {
	g_sServerState.bActive = 1;
	DWORD	ThreadID;

	g_sServerState.hThread = CreateThread(
		NULL, 0,
		(LPTHREAD_START_ROUTINE)g_sServerState.threadProc, &g_sServerState,
		0, &ThreadID
	);

	if (g_sServerState.hThread == NULL) {
		g_sServerState.bActive = 0;
		g_sConsole.writeError("[SERVERSTATE][init]CreateThread() failed with error %d", GetLastError());
	}
}

void sServerState_close(void) {
	if(g_sServerState.bActive) {
		g_sServerState.bActive = 0;
		WaitForSingleObject(g_sServerState.hThread, INFINITE);
  }
}

void sServerState_threadProc(void) {
	PMSG_JOINSERVER_STAT msg;

	msg.h.bSize = sizeof( msg );
	msg.h.bTypeC = PMHC_BYTE;
	msg.h.bHeadCode = 0x02;

	while(g_sServerState.bActive) {
//		msg.iQueueCount = fnQueue.getCount(&g_sRecvQueue);
		g_sUdp.sendData((LPBYTE)&msg, msg.h.bSize);
		Sleep(1000);
	}
}

// tServerState declaration

tServerState g_sServerState = {
	sServerState_init,
	sServerState_close,
	sServerState_threadProc
};
