/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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

#define MAX_REPLYSIZE		32768 /* arbitrary 32 KB limit for http reply buffering */

#define A_READ			1
#define A_MODIFY		2
#define A_INSERT		4
#define A_DELETE		8
#define A_ADMIN			16

#define MENU_ADMIN		1
#define MENU_BOOKMARKS		2
#define MENU_CALENDAR		3
#define MENU_CALLS		4
#define MENU_CONTACTS		5
#define MENU_EMAIL		6
#define MENU_FILES		7
#define MENU_FINANCE		8
#define MENU_FORUMS		9
#define MENU_INVOICES		10
#define MENU_MAIN		11
#define MENU_MESSAGES		12
#define MENU_NOTES		13
#define MENU_PROFILE		14
#define MENU_PROJECTS		15
#define MENU_SEARCHES		16
#define MENU_TASKS		17
#define MENU_WEBLOG		18
#define MENU_XMLRPC		19

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
	short int popssl;
	short int smtpport;
	short int smtpssl;
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
} EMAIL;
typedef struct {
	struct timeval runtime;
	/* USER DATA */
	char       username[64];
	char       token[64];
	char       domainname[64];
	short int  uid;
	short int  gid;
	short int  did;
	short int  maxlist;
	short int  timezone;
	char       language[4];
	char       theme[40];
	EMAIL      *wm;
	/* OUTGOING DATA */
	short int out_status;
	short int out_headdone;
	short int out_bodydone;
	short int out_flushed;
	int       out_bytecount;
	int       out_ContentLength;
	/* BUFFERS */
	short int lastbuf;
	char      smallbuf[4][4096];
	char      largebuf[MAX_FIELD_SIZE];
	char      replybuf[MAX_REPLYSIZE];
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
	nes_state *N;
	CONNDATA *dat;
} CONN;
typedef struct {
	nes_state *N;
	CONN *conn;
	pthread_t ListenThreadSTD;
	pthread_t ListenThreadSSL;
	TCP_SOCKET ListenSocketSTD;
	TCP_SOCKET ListenSocketSSL;
} HTTP_PROC;

typedef	int  (*HTMOD_INIT)(_PROC *);
typedef	int  (*HTMOD_MAIN)(CONN *);
typedef	int  (*HTMOD_EXIT)(void);
/*
typedef	void (*HTMOD_HTML_HEADER)(CONN *, char *);
typedef	void (*HTMOD_HTML_FOOTER)(CONN *);
typedef	void (*HTMOD_HTML_LOGIN)(CONN *);
typedef	void (*HTMOD_HTML_LOGOUT)(CONN *);
typedef	void (*HTMOD_HTML_MOTD)(CONN *);
typedef	void (*HTMOD_HTML_FRAMESET)(CONN *);
typedef	void (*HTMOD_HTML_MENUFRAME)(CONN *);
typedef	void (*HTMOD_HTML_RELOADFRAME)(CONN *);
typedef	void (*HTMOD_HTML_TOPMENU)(CONN *, int);
*/
typedef	int  (*HTMOD_EMAIL_SYNC)(CONN *, int);
typedef	void (*HTMOD_NOTES_SUBLIST)(CONN *, char *, int, int);
typedef	void (*HTMOD_TASKS_LIST)(CONN *, int, int);
