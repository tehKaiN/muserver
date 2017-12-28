#include "main.h"

//tQueue g_sRecvQueue;

void close(void) {
	g_sServerState.close();
	g_sUdp.close();
	IOCPSock_destroy();
	g_sJsUserBase.close();
	g_sUseLog.close();
	g_sDB.close();
	WSACleanup();
	g_sLog.close();
	g_sConsole.close();
	g_sConfig.close();
}

BOOL init(LPSTR szCmdLine) {
	g_sLog.init("jsLog", "");
	g_sConsole.init("JoinServer", close);
	g_sConsole.writeSuccess("JS init, ver.%u.%u.%u", BUILD_YEAR, BUILD_MONTH, BUILD_DAY);

	g_sConfig.init(szCmdLine);
	g_sUseLog.init();
	g_sDB.init();
	g_sJsUserBase.init();

	g_sUdp.init();
	g_sUdp.createSocket();
	g_sUdp.sendSet(g_sConfig.szCsIp, g_sConfig.wCsPort);
	g_sServerState.init();

	IOCPSock_create(
		g_sConfig.wJsPort, g_sConfig.dwMaxUsers*10, g_sConfig.dwWorkerThreadCount,
    g_sConfig.dwMaxServers, sizeof(tGS),
    0, gs_onServerDel, protocol_process
	);

	g_sConsole.writeSuccess("JoinServer started");

	return 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int nCmdShow) {
	if(!init(szCmdLine)) {
		close();
		return EXIT_FAILURE;
	}
	MSG msg;
	while(GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	close();
	return EXIT_SUCCESS;
}
