#ifndef GUARD_PROTOCOL_JG_STRUCTS_H
#define GUARD_PROTOCOL_JG_STRUCTS_H

#include "heads.h"

// 0x01 GS->JS: Auth request
typedef struct {
	PBMSG_HEAD h;

	char szId[MAX_IDSTRING];   // Account ID
	char szPass[MAX_IDSTRING]; // Password
	short iNumber;             //
	char szIP[17];             // IP
} SDHP_IDPASS, *LPSDHP_IDPASS;

// 0x01 JS->GS: Auth result
typedef struct
{
	PBMSG_HEAD h;
	BYTE bResult;

	short iNumber;           //
	char szId[MAX_IDSTRING]; // Account ID
	short iUserNumber;       //
	short iDBIdx;            // User index in DB
	char pSecurityNum[13];	 // Security number
} SDHP_IDPASSRESULT, *LPSDHP_IDPASSRESULT;

// 0x02 GS->JS: Notice about failed join attempt
typedef struct {
	PBMSG_HEAD h;

	short iNumber;             //
	char szId[MAX_IDSTRING+1]; // account ID
	long lUserIdx;             //
	long lDBIdx;               // user DB index
} SDHP_JOINFAIL, * LPSDHP_JOINFAIL;

// 0x05:
typedef struct {
	PBMSG_HEAD h;
	char szId[MAX_IDSTRING]; // account ID
} SDHP_USERCLOSE_ID, *LPSDHP_USERCLOSE_ID;

// 0x06 JS->GS: User bill information - not needed
typedef struct {
	PBMSG_HEAD h;

	char szId[MAX_IDSTRING]; // Account ID
	short iNumber;           //
	BYTE bCertifyType;       // Membership type
	BYTE bPayCode;           //
	char pEndsDays[12];      //
	long lEndTime;           //
} SDHP_BILLSEARCH_RESULT, * LPSDHP_SDHP_BILLSEARCH_RESULT;

// 0x08 JS->GS: Warn about account already connected
typedef struct {
	PBMSG_HEAD h;
	char szId[MAX_IDSTRING]; // Account ID
} SDHP_OTHERJOINMSG, *LPSDHP_OTHERJOINMSG;

#endif // GUARD_PROTOCOL_JG_STRUCTS_H
