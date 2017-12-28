#ifndef GUARD_UDP_H
#define GUARD_UDP_H

/*
 * Struct for communication with ConnectServer
 */

// defines
#define DATA_BUFSIZE 2048
#define DEFAULT_BUFFER_LENGTH   4096

// structs

typedef struct
{
	OVERLAPPED sOverlapped;
	WSABUF sDataBuff;
	char pBuffer[DATA_BUFSIZE];
	DWORD dwByteCount;
	int iOfs;
} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

typedef struct _tUdp {

	// functions

	BOOL (*init)(void);
	BOOL (*close)(void);
	BOOL (*createSocket)(void);
	BOOL (*sendSet)(PCHAR szIP, WORD wPort);
	BOOL (*recvSet)(WORD wPort);
	BOOL (*sendData)(BYTE *pSendData, DWORD dwSendDataLen);
	BOOL (*muProtocolParse)(BYTE *pRecvData, int iRecvDataLen, PCHAR szIP);
	BOOL (*run)(void);
	BOOL (*recvThreadProc)(void);
	void (*protocolCore)(BYTE bProtoNum, BYTE *pRecv, long lLen, PCHAR szIP);

	// fields
	WORD wPort;
	HANDLE hThread;
	DWORD dwThreadID;

	SOCKET hSocket;
	BYTE *pRecvBuff;
	BYTE *pSendBuff;
	DWORD dwLength;
	long lRecvOffs;
	DWORD dwSendLength;

	PER_IO_OPERATION_DATA	sPerIoSendData;
	SOCKADDR_IN sSockAddr;

	// WZQueue - unused
//	WZQueue RecvQueue;
//	WZQueue	SendQueue;


} tUdp;

// functions

// externs
extern tUdp g_sUdp;

#endif // GUARD_UDP_H
