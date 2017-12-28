#include "main.h"

// g_sUdp-specific functions

BOOL sUdp_init(void) {
	WSADATA wsd;

	if(WSAStartup(MAKEWORD(2,2), &wsd) != 0){
		g_sConsole.writeError("[UDP][init]WSAStartup fail: %d", WSAGetLastError());
		return 0;
	}
	g_sUdp.dwLength  = DEFAULT_BUFFER_LENGTH;
	g_sUdp.lRecvOffs = 0;
	g_sUdp.hThread = NULL;
	g_sConsole.writeSuccess("[UDP]init");
	return 1;
}

BOOL sUdp_close(void) {
	if(g_sUdp.hThread) {
		TerminateThread(g_sUdp.hThread, 0);
		if(g_sUdp.hThread) {
			WaitForSingleObject( g_sUdp.hThread, INFINITE );
			CloseHandle(g_sUdp.hThread);
			g_sUdp.hThread = 0;
		}
		HeapFree(GetProcessHeap(), 0, g_sUdp.pRecvBuff);
		return 1;
	}
	return 0;
}

BOOL sUdp_createSocket(void) {
	g_sUdp.hSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 0);
	if( g_sUdp.hSocket == INVALID_SOCKET ) {
		return 0;
	}
	return 1;
}

BOOL sUdp_sendSet(PCHAR szIP, WORD wPort) {

	g_sUdp.wPort = wPort;

	g_sUdp.sSockAddr.sin_family = AF_INET;
	g_sUdp.sSockAddr.sin_port = htons(wPort);
	g_sUdp.sSockAddr.sin_addr.s_addr = inet_addr(szIP);

	if(g_sUdp.sSockAddr.sin_addr.s_addr == INADDR_NONE) {
		struct hostent *pHost;
		pHost = gethostbyname(szIP);
		if(pHost) {
			CopyMemory(&g_sUdp.sSockAddr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
		} else {
			return 0;
		}
	}
	g_sConsole.writeSuccess("[UDP]Sending set to %s:%u", szIP, wPort);
	return 1;
}

BOOL sUdp_recvSet(WORD wPort) {
	g_sUdp.wPort = wPort;

	g_sUdp.sSockAddr.sin_port = htons(wPort);
	g_sUdp.sSockAddr.sin_family = AF_INET;
	g_sUdp.sSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if( bind(g_sUdp.hSocket, (SOCKADDR *)&g_sUdp.sSockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR ) {
		return 0;
	}

	g_sUdp.pRecvBuff = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_sUdp.dwLength);
	if(!g_sUdp.pRecvBuff) {
		g_sConsole.writeError("[UDP][recvSet]Can't alloc memory for pRecvBuff!");
		return 0;
	}

	g_sConsole.writeSuccess("[UDP]Recv set at port %u",  wPort);
	return 1;
}

BOOL sUdp_sendData(BYTE *pSendData, DWORD dwSendDataLen) {
	ZeroMemory(&(g_sUdp.sPerIoSendData.sOverlapped), sizeof(OVERLAPPED));
	memcpy(g_sUdp.sPerIoSendData.pBuffer, pSendData, dwSendDataLen);

	g_sUdp.sPerIoSendData.iOfs = dwSendDataLen;
	g_sUdp.sPerIoSendData.sDataBuff.buf = g_sUdp.sPerIoSendData.pBuffer;
	g_sUdp.sPerIoSendData.sDataBuff.len = dwSendDataLen;

	DWORD dwRet = WSASendTo(
		g_sUdp.hSocket, &(g_sUdp.sPerIoSendData.sDataBuff),
		1, &dwSendDataLen,
		0, (SOCKADDR*)&g_sUdp.sSockAddr, sizeof(SOCKADDR_IN),
		&(g_sUdp.sPerIoSendData.sOverlapped), NULL
	);
	if( dwRet == SOCKET_ERROR ) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			g_sConsole.writeError("[UDP][sendData]WSASend() failed with error %d", WSAGetLastError());
			return 0;
		}
	}
	return 1;
}

BOOL sUdp_muProtocolParse(BYTE *pRecvData, int iRecvDataLen, PCHAR szIP) {
	int iOffs = 0;
	WORD wSize = 0;
	BYTE bHeadCode; // compiler throws warning because UdpProtocolCore is not used

	while(1) {
		switch(pRecvData[iOffs]) {
			case PMHC_BYTE: { // 0xC1
				wSize = pRecvData[iOffs+1];
				bHeadCode = pRecvData[iOffs+2];
			} break;
			case PMHC_WORD: { // 0xC2
				wSize = pRecvData[iOffs+1];
				wSize <<= 8;
				wSize |= pRecvData[iOffs+2];
				bHeadCode = pRecvData[iOffs+3];
			} break;
			default: {
				g_sConsole.writeError("[UDP][muProtocolParse]Unknown packet type: %02X", pRecvData[iOffs]);
				g_sUdp.lRecvOffs = 0;
				return 0;
			}
		}
		if(wSize == 0)	{
			g_sConsole.writeError("[UDP][muProtocolParse]wSize == 0");
			return 0;
		} else if(wSize <= iRecvDataLen) {
			g_sUdp.protocolCore(bHeadCode, (pRecvData+iOffs), wSize, szIP);
			iOffs += wSize;
			g_sUdp.lRecvOffs -= wSize;
			if(g_sUdp.lRecvOffs <= 0) {
				break;
			}
		} else {
			if(iOffs > 0) {
				if(g_sUdp.lRecvOffs < 1) {
					return 0;
				} else {
					memcpy(pRecvData, (pRecvData+iOffs), g_sUdp.lRecvOffs);
					return 1;
				}
			}
			break;
		}
	}
	return 1;

}

BOOL sUdp_run(void) {
	g_sUdp.hThread = CreateThread(
		NULL, 0,
		(LPTHREAD_START_ROUTINE)g_sUdp.recvThreadProc,
		(void*)&g_sUdp, 0, &g_sUdp.dwThreadID
	);
	if(g_sUdp.hThread == NULL) {
		g_sConsole.writeError("[UDP][run]CreateThread fail");
		return 0;
	}
	return 1;

}

BOOL sUdp_recvThreadProc(void) {
	int iRet;
	SOCKADDR_IN	sender;
	DWORD dwSenderSize;
	dwSenderSize = sizeof(sender);
	while(1) {
		iRet = recvfrom(
			g_sUdp.hSocket, (PCHAR) g_sUdp.pRecvBuff + g_sUdp.lRecvOffs,
			DEFAULT_BUFFER_LENGTH-g_sUdp.lRecvOffs, 0,
			(SOCKADDR *)&sender, (int*)&dwSenderSize
		);

		if (iRet == SOCKET_ERROR) {
			g_sConsole.writeError("[UDP][recvThreadProc]recvfrom() failed: %d", WSAGetLastError());
		}
		else if (iRet) {
			g_sConsole.writeNotice("[UDP][recvThreadProc]JS actually receives sth from CS!");
			g_sUdp.dwLength = iRet;
			g_sUdp.lRecvOffs += iRet;
			g_sUdp.muProtocolParse(g_sUdp.pRecvBuff, g_sUdp.dwLength, inet_ntoa(sender.sin_addr));
		}
	}
}

void sUdp_protocolCore(BYTE bProtoNum, BYTE *pRecv, long lLen, PCHAR szIP) {
	// placeholder, not sure if it's really used for anything
	g_sConsole.writeNotice("[UDP][protocolCore]CALL! code: %u, length: %d, IP: %s", bProtoNum, lLen, szIP);
}

// g_sUdp definition
tUdp g_sUdp = {
	sUdp_init,
	sUdp_close,
	sUdp_createSocket,
	sUdp_sendSet,
	sUdp_recvSet,
	sUdp_sendData,
	sUdp_muProtocolParse,
	sUdp_run,
	sUdp_recvThreadProc,
	sUdp_protocolCore
};
