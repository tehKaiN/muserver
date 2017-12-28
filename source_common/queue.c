#include "queue.h"

void queue_deleteAllNodes(tQueue *pQueue) {
  EnterCriticalSection(&pQueue->sCriticalSection);
	tQueueNode *pCurr;
	tQueueNode *pNext;
	pCurr = pQueue->pFirst;
	while(pCurr) {
		pNext = pCurr->pNext;
		HeapFree(GetProcessHeap(), 0, pCurr->pObject);
		HeapFree(GetProcessHeap(), 0, pCurr);
		pCurr = pNext;
	}
	pQueue->pCurr = pQueue->pFirst = pQueue->pLast = NULL;
  LeaveCriticalSection(&pQueue->sCriticalSection);
}

void queue_init(tQueue *pQueue, DWORD dwMaxNodes) {
	pQueue->dwMaxNodes = dwMaxNodes;
	pQueue->pFirst = pQueue->pLast = NULL;
	pQueue->dwCount = 0;

	BOOL bSuccess =	InitializeCriticalSectionAndSpinCount(&pQueue->sCriticalSection, SPIN_COUNT);
	if(!bSuccess)
	{
		if( GetLastError() == STATUS_NO_MEMORY )
			InitializeCriticalSection(&pQueue->sCriticalSection);
	}
}

void queue_close(tQueue *pQueue) {
	queue_deleteAllNodes(pQueue);
	DeleteCriticalSection(&pQueue->sCriticalSection);
}

DWORD queue_getCount(tQueue *pQueue) {
	EnterCriticalSection(&pQueue->sCriticalSection);
	DWORD dwNum = pQueue->dwCount;
	LeaveCriticalSection(&pQueue->sCriticalSection);
	return dwNum;
}

tQueueNode *queue_push(tQueue *pQueue, BYTE *pObject, DWORD dwSize, BYTE bHeadCode, long lIdx) {
	if( dwSize < 1 || dwSize > 65536 ) {
		return 0;
	}

	BOOL bRet = 0;

	EnterCriticalSection(&pQueue->sCriticalSection);
	tQueueNode* pNode = (tQueueNode*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(tQueueNode));

	if(pNode) {
		BYTE* pMsg = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
		if(pMsg) {
			memcpy(pMsg, pObject, dwSize);

			pNode->pObject = pMsg;
			pNode->dwSize = dwSize;
			pNode->bHeadCode = bHeadCode;
			pNode->iIdx = lIdx;
			pNode->bSending = 0;
			pNode->iOfs = 0;
			if( queue_pushNode(pQueue, pNode)) {
				bRet = 1;
			}
		}	else {
			HeapFree(GetProcessHeap(), 0, pMsg);
			HeapFree(GetProcessHeap(), 0, pNode);
		}
	} else {
		HeapFree(GetProcessHeap(), 0, pNode);
	}
	LeaveCriticalSection(&pQueue->sCriticalSection);
	if(bRet) {
		return pNode;
	}
	return 0;
}

BOOL queue_pushNode(tQueue *pQueue, tQueueNode *pNode) {
	EnterCriticalSection(&pQueue->sCriticalSection);
	BOOL bRet = 1;
	if( pQueue->dwCount+1 < pQueue->dwMaxNodes) {
		// crosslink two nodes.
		pNode->pPrev = pQueue->pLast;
		pNode->pNext = NULL;

		// check headnode.
		if(pQueue->pFirst == NULL)
			pQueue->pFirst = pNode;
		else
			pQueue->pLast->pNext = pNode;

		pQueue->pLast = pNode;

		++pQueue->dwCount;
	} else {
		bRet = 0;
	}
	LeaveCriticalSection(&pQueue->sCriticalSection);
	return bRet;
}

BOOL queue_pop(tQueue *pQueue, BYTE *pObject, DWORD *pSize, BYTE *pHeadCode, long *pIdx) {
	tQueueNode*	pNode = NULL;

	EnterCriticalSection(&pQueue->sCriticalSection);
	pNode = pQueue->pFirst;
	if(pNode) {
		// remove from list - set new head
		if(pNode->pNext) {
			pQueue->pFirst = pNode->pNext;
			pQueue->pFirst->pPrev = NULL;
		} else {
			pQueue->pFirst = pQueue->pLast = NULL;
		}
		pQueue->dwCount--;

		// pass values to params
		memcpy(pObject, pNode->pObject, pNode->dwSize);
		*pSize = pNode->dwSize;
		*pHeadCode = pNode->bHeadCode;
		*pIdx = pNode->iIdx;

		// free old head
		HeapFree(GetProcessHeap(), 0, pNode->pObject);
		HeapFree(GetProcessHeap(), 0, pNode);
		LeaveCriticalSection(&pQueue->sCriticalSection);
		return 1;
	}

	LeaveCriticalSection(&pQueue->sCriticalSection);
	return 0;
}
