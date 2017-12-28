#ifndef GUARD_SERVERSTATE_H
#define GUARD_SERVERSTATE_H

/*
 * Struct for handling reporting JS load to CS
 */

// structs

typedef struct
{
	PBMSG_HEAD	h;
	int			iQueueCount;
} PMSG_JOINSERVER_STAT;

typedef struct _tServerState {
  // functions
  void (*init)(void);
  void (*close)(void);
  void (*threadProc)(void);
  // fields
  BOOL bActive;
  HANDLE hThread;
}	tServerState;

// functions

// externs
extern tServerState g_sServerState;

#endif // GUARD_SERVERSTATE_H
