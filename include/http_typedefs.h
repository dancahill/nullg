/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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

#define MAX_AUTH_FIELDS		16
#define MAX_PREF_FIELDS		16
#define MAX_REPLYSIZE		16384 /* arbitrary 16 KB limit for reply buffering */

#define A_READ			1
#define A_MODIFY		2
#define A_INSERT		4
#define A_DELETE		8
#define A_ADMIN			16

#define MENU_ADMIN		1
#define MENU_ACCOUNTING		2
#define MENU_BOOKMARKS		3
#define MENU_CALENDAR		4
#define MENU_CALLS		5
#define MENU_CONTACTS		6
#define MENU_FILES		7
#define MENU_FORUMS		8
#define MENU_MAIN		9
#define MENU_MESSAGES		10
#define MENU_NOTES		11
#define MENU_ORDERS		12
#define MENU_PROFILE		13
#define MENU_SEARCHES		14
#define MENU_TASKS		15
#define MENU_WEBMAIL		16
#define MENU_XMLRPC		17

typedef struct {
	int lastref;
	int printed;
} _btree;
typedef struct {
	int id;
	int depth;
	int numchildren;
} _ptree;
typedef struct {
	int userid;
	int time;
} u_avail;

typedef struct {
	TCP_SOCKET socket;
	short int popport;
	short int smtpport;
	short int connected;
	short int remove;
	short int showdebug;
	short int imapidx;
	short int codecstate;
	short int numfiles;
	int       imapmread;
	int       imapmsize;
	char      files[20][100];
	char      username[64];
	char      password[64];
	char      realname[64];
	char      organization[64];
	char      address[64];
	char      replyto[64];
	char      servertype[64];
	char      popserver[64];
	char      smtpserver[64];
	char      smtpauth[10];
	char      signature[1025];
} WEBMAIL;
typedef struct {
	char name[17];
	short int val;
} AUTH;
typedef struct {
	char name[17];
	short int val;
} PREF;
typedef struct {
	// USER DATA
	char       user_username[64];
	char       user_token[64];
	char       user_domainname[64];
	short int  user_uid;
	short int  user_gid;
	short int  user_did;
	AUTH       auth[MAX_AUTH_FIELDS];
	PREF       pref[MAX_PREF_FIELDS];
	short int  user_daystart;
	short int  user_daylength;
	short int  user_mailcurrent;
	short int  user_maildefault;
	short int  user_menustyle;
	short int  user_maxlist;
	short int  user_timezone;
	char       user_language[4];
	char       user_theme[40];
	WEBMAIL   *wm;
	// INCOMING DATA
	short int cgi_lite;
	char in_Connection[16];
	int  in_ContentLength;
	char in_ContentType[128];
	char in_Cookie[1024];
	char in_Host[64];
	char in_IfModifiedSince[64];
	char in_PathInfo[128];
	char in_Protocol[16];
	char in_QueryString[1024];
	char in_Referer[128];
	char in_RemoteAddr[16];
	int  in_RemotePort;
	char in_RequestMethod[8];
	char in_RequestURI[1024];
	char in_ScriptName[128];
	char in_UserAgent[128];
	char in_CGIPathInfo[128];
	char in_CGIScriptName[128];
	// OUTGOING DATA
	short int out_status;
	short int out_headdone;
	short int out_bodydone;
	int out_bytecount;
	short int out_flushed;
	char out_CacheControl[40];
	char out_Connection[20];
	char out_ContentDisposition[64];
	int  out_ContentLength;
	char out_ContentType[128];
	char out_Date[40];
	char out_Expires[40];
	char out_LastModified[64];
	char out_Location[128];
	char out_Pragma[40];
	char out_Protocol[20];
	char out_Server[128];
	char out_SetCookieUser[128];
	char out_SetCookiePass[128];
	unsigned char out_ReplyData[MAX_REPLYSIZE];
	// BUFFERS
	short int lastbuf;
	char      smallbuf[4][4096];
	char      largebuf[16384];
} CONNDATA;
typedef struct {
	pthread_t handle;
#ifdef WIN32
	unsigned int id;
#else
	pthread_t id;
#endif
	short int state;
	TCP_SOCKET socket;
	CONNDATA *dat;
	char *PostData;
} CONN;

//typedef struct {
//	char *fn_name;
//	void *fn_ptr;
//} FUNCTION;
typedef struct {
	char mod_name[40];
	short int mod_submenu;
	char mod_menuname[40];
	char mod_menuuri[40];
	char mod_menuperm[40];
	char fn_name[40];
	char fn_uri[40];
	void *fn_ptr;
} MODULE_MENU;
typedef struct {
	char mod_name[40];
	char fn_name[40];
	void *fn_ptr;
} MODULE_FUNC;

typedef struct {
	int ListenSocket;
	int ListenSocketSSL;
	pthread_t ListenThread;
	pthread_t ListenThreadSSL;
	unsigned short RunAsCGI;
	MODULE_MENU mod_menuitems[MAX_MOD_MENUITEMS+1];
	MODULE_FUNC mod_functions[MAX_MOD_FUNCTIONS+1];
} HTTP_PROC;

typedef	int  (*HTMOD_INIT)(_PROC *, HTTP_PROC *, FUNCTION *);
typedef	void (*HTMOD_MAIN)(CONN *);

typedef	void (*HTMOD_HTML_HEADER)(CONN *, char *);
typedef	void (*HTMOD_HTML_FOOTER)(CONN *);
typedef	void (*HTMOD_HTML_LOGIN)(CONN *);
typedef	void (*HTMOD_HTML_LOGOUT)(CONN *);
typedef	void (*HTMOD_HTML_MOTD)(CONN *);
typedef	void (*HTMOD_HTML_FRAMESET)(CONN *);
typedef	void (*HTMOD_HTML_MENUFRAME)(CONN *);
typedef	void (*HTMOD_HTML_RELOADFRAME)(CONN *);
typedef	void (*HTMOD_HTML_TOPMENU)(CONN *, int);

typedef	int  (*HTMOD_MAIL_SYNC)(CONN *, int);
typedef	void (*HTMOD_NOTES_SUBLIST)(CONN *, char *, int, int);
typedef	void (*HTMOD_TASKS_LIST)(CONN *, int, int);
