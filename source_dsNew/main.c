#include "main.h"

//tQueue g_sRecvQueue;

void close(void) {
//	g_sServerState.close();
//	g_sUdp.close();
	IOCPSock_destroy();
//	g_sJsUserBase.close();
//	g_sUseLog.close();
	db_destroy();
	WSACleanup();
	g_sLog.close();
	g_sConsole.close();
	g_sConfig.close();
}

BOOL init(LPSTR szCmdLine) {
	g_sLog.init("dsLog", "");
	g_sConsole.init("DataServer", close);
	g_sConsole.writeSuccess("DS init, ver.%u.%u.%u", BUILD_YEAR, BUILD_MONTH, BUILD_DAY);

	g_sConfig.init(szCmdLine);
//	g_sUseLog.init();
	item_initDftItems();
	db_create();
//	g_sJsUserBase.init();

	IOCPSock_create(
		g_sConfig.dwListenPort, g_sConfig.dwMaxServers*g_sConfig.dwPacketQueueLength, g_sConfig.dwWorkerThreadCount,
    g_sConfig.dwMaxServers, sizeof(tNetClient),
    0, 0, protocol_process
	);

	g_sConsole.writeSuccess("DataServer started");

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
