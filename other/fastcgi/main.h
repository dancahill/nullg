/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* #includes */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#pragma comment(lib, "ws2_32.lib")
	#include <winsock2.h>
	#include <windows.h>
	#include <io.h>
#ifdef BCC
	#define _setmode setmode
#endif
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#define closesocket close
#endif

/* arbitrary 32 MB limit for POST request sizes */
#define MAX_POSTSIZE	33554432

typedef struct {
	short int state;
	short int socket;
	struct sockaddr_in ClientAddr;
	time_t ctime; // Creation time
	time_t atime; // Last Access time
	char *PostData;
	// INCOMING DATA
//	char in_Connection[16];
	int  in_ContentLength;
//	char in_ContentType[128];
	char in_Cookie[1024];
	char in_Host[64];
//	char in_IfModifiedSince[64];
	char in_PathInfo[128];
//	char in_Protocol[16];
	char in_QueryString[1024];
//	char in_Referer[128];
	char in_RemoteAddr[16];
//	int  in_RemotePort;
	char in_RequestMethod[8];
	char in_RequestURI[1024];
	char in_ScriptName[128];
	char in_UserAgent[128];
//	char in_CGIPathInfo[128];
//	char in_CGIScriptName[128];
	// OUTGOING DATA
//	short int out_status;
//	short int out_headdone;
//	short int out_bodydone;
//	short int out_flushed;
//	char out_CacheControl[40];
//	char out_Connection[20];
//	char out_ContentDisposition[64];
//	int  out_ContentLength;
//	char out_ContentType[128];
//	char out_Date[40];
//	char out_Expires[40];
//	char out_LastModified[64];
//	char out_Location[128];
//	char out_Pragma[40];
//	char out_Protocol[20];
//	char out_Server[128];
//	char out_SetCookieUser[128];
//	char out_SetCookiePass[128];
} CONN;
