#ifndef GUARD_USER_H
#define GUARD_USER_H

/*
 * Struct for handling connected players - state, play time, ip, db idx, etc.
 * Used for checking if player is already connected on login and for useLog
 */

// structs

typedef struct _tJsUser {
	BOOL bConnected;           // connect state
	char szId[MAX_IDSTRING+1]; // login
	char szIP[17];             // ip address (###.###.###.###\0)
	long iDBIdx;               // idx in database
	BYTE bGsIdx;               // idx of connected gs
	int iGaIdx;                // idx in GS?
	DWORD dwPlayTime;
} tJsUser;

typedef struct _tJsUserBase{
	// functions
  void (*init)(void);
  void (*close)(void);

  long (*add)(PCHAR szId, PCHAR szPass, long iDBNumber, BYTE bGameServerIdx, PCHAR szIP, int iGaIdx);
  BOOL (*delById)(PCHAR szId);
  BOOL (*delByIdx)(long iIdx, long iDBNumber);
  long (*searchById)(PCHAR szId);
  long (*searchByIdRet)(PCHAR szId);
  BOOL (*joinFail)(long iIdx, PCHAR szId, long iDBNumber);
  BOOL (*accountBlock)(long iIdx, PCHAR szId, long iDBNumber, char cBlockCode);
  void (*proc)(void);

  // fields
  tJsUser **pUsers;
  DWORD dwNextUserIdx;
  DWORD dwTotalCount;
} tJsUserBase;

// functions

// externs
extern tJsUserBase g_sJsUserBase;

#endif // GUARD_USER_H
