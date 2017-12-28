#ifndef GUARD_LOG_H
#define GUARD_LOG_H

#include <windows.h>
#include <stdio.h>
#include "utils.h"

// structs

typedef struct _tLog {
	// functions
	void (*init)(
		IN PCHAR szDir,
		IN PCHAR szSuff
	);
	void (*close)(void);
	void (*write)(
		IN PCHAR szFmt,
		IN ...
	);

	// settable fields

	// fields
	FILE *hLog;
} tLog;

// functions

// externs
extern tLog g_sLog;

#endif // GUARD LOG_H
