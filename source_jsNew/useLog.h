#ifndef GUARD_USELOG_H
#define GUARD_USELOG_H

/*
 * Struct for handling connection log - connect and disconnect event for given player
 */

// structs

typedef struct _tUseLog {
  void (*init)(void);
  void (*close)(void);
  void (*refreshDate)(void);

  void (*connect)(PCHAR szServerName, PCHAR szId, PCHAR szIP);
  void (*disconnect)(PCHAR szServerName, PCHAR szId, PCHAR szIP, DWORD dwPlayTime);

  BOOL bEnabled;
  FILE *hLog;
  char szDateBfr[11]; // YYYY-MM-DD\0
} tUseLog;

// functions

// externs
extern tUseLog g_sUseLog;

#endif // GUARD_USELOG_H
