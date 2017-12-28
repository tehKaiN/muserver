#ifndef GUARD_PROTOCOL_HEADS_H
#define GUARD_PROTOCOL_HEADS_H

#define PMHC_BYTE				0xC1 // size is BYTE
#define PMHC_WORD				0xC2 // size is WORD

// Message header - with BYTE as size
typedef struct {
	BYTE bTypeC;    // 0xC1
	BYTE bSize;     // size of message
	BYTE bHeadCode; // protocol type
} PBMSG_HEAD, *LPPBMSG_HEAD;

// Message header - with WORD as size
typedef struct {
	BYTE bTypeC;    // 0xC2
	BYTE bSizeH;    // higher part of size WORD
	BYTE bSizeL;    // lower part of zie WORD
	BYTE bHeadCode;	// protocol type
} PWMSG_HEAD, *LPPWMSG_HEAD;

// Message header - with BYTE as size - extended code
typedef struct {
	BYTE bTypeC;    // 0xC1?
	BYTE bSize;     // size of message
	BYTE bHeadCode; // protocol type
	BYTE bSubCode;  // protocol subtype
} PBMSG_HEAD2, * LPPBMSG_HEAD2;

typedef struct {
	BYTE bTypeC;    // 0xC2?
	BYTE bSizeH;    // higher part of size WORD
	BYTE bSizeL;    // lower part of zie WORD
	BYTE bHeadCode; // protocol type
	BYTE bSubCode;  // protocol subtype
} PWMSG_HEAD2, * LPPWMSG_HEAD2;

// ------ Result packet ------ //
typedef struct {
	PBMSG_HEAD h;
	BYTE bResult;      // Result code
	DWORD dwItemCount; //
} SDHP_RESULT, *LPSDHP_RESULT;

#endif // GUARD_PROTOCOL_HEADS_H
