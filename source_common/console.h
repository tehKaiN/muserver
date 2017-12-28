#ifndef GUARD_CONSOLE_H
#define GUARD_CONSOLE_H

#include <windows.h>

// structs

typedef struct _tConsole {
  // functions
  void (*init)(
		IN PCHAR szTitle,
		IN void (*closeProc)(void)
	);
  void (*close)(void);
  BOOL (*handler)(
		IN DWORD CEvent
	);
  void (*writeBinary)(
		IN BYTE *pBinBfr,
		IN DWORD dwLength
  );
  void (*writeText)(
		IN PCHAR szText,
		IN ...
	);
  void (*writeSuccess)(
		IN PCHAR szText,
		IN ...
	);
  void (*writeError)(
		IN PCHAR szText,
		IN ...
	);
  void (*writeNotice)(
		IN PCHAR szText,
		IN ...
	);

	// fields
	void (*closeProc)(void);
} tConsole;

// functions

// externs
extern tConsole g_sConsole;

#endif // GUARD_CONSOLE_H
