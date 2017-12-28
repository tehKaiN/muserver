#ifndef GUARD_QUEUE_H
#define GUARD_QUEUE_H

#include <windows.h>

// specific defines

// max queue nodes, WZ orginal: 50000
// #define MAX_QUEUE_NODES (g_sConfig.dwMaxUsers*10)
// spin count for queue thread, 4000 reccomended by MS for HeapAlloc etc.
#define SPIN_COUNT 4000

// structs

typedef struct _tQueueNode {
	struct _tQueueNode *pPrev; // served before current - wz notation: up
	struct _tQueueNode *pNext; // served  after current - wz notation: down
	BYTE *pObject;
	DWORD dwSize;
	BOOL bSending;
	int iOfs;
	BYTE bHeadCode;
	int iIdx;
	//PER_IO_CONTEXT_L IoCtxt;
} tQueueNode;

typedef struct _tQueue {
	BOOL bEmpty;
	DWORD dwCount;
	DWORD dwMaxNodes;
	tQueueNode *pLast;  //  last to be served - wz notation: tail
	tQueueNode *pFirst; // first to be served - wz notation: head
	tQueueNode *pCurr;  // current node
	CRITICAL_SECTION sCriticalSection;
} tQueue;

// externs
void queue_init(
	IN tQueue *pQueue,
	IN DWORD dwMaxNodes
);
void queue_close(
	IN tQueue *pQueue
);
DWORD queue_getCount(
	IN tQueue *pQueue
);

tQueueNode *queue_push(
	IN tQueue *pQueue,
	IN BYTE *pObject,
	IN DWORD dwSize,
	IN BYTE bHeadCode,
	IN long lIdx
);
BOOL queue_pushNode(
	IN tQueue *pQueue,
	IN tQueueNode *pNode
);
BOOL queue_pop(
	IN tQueue *pQueue,
	OUT BYTE *pObject,
	OUT DWORD *pSize,
	OUT BYTE *pHeadCode,
	OUT long *pIdx
);
void queue_deleteAllNodes(
	IN tQueue *pQueue
);

#endif // GUARD_QUEUE_H
