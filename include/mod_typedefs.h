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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define _MT 1
#pragma comment(lib, "libcmt.lib")
#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include <netdb.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/time.h>
#endif
#include "defines.h"
#include "http_typedefs.h"

/* typedefs */
typedef struct {
	int bookmarkid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int folderid;
	char bookmarkname[51];
	char bookmarkurl[256];
} REC_BOOKMARK;
typedef struct {
	int folderid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int parentid;
	char foldername[51];
} REC_BOOKMARKFOLDER;
typedef struct {
	int callid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int assignedby;
	int assignedto;
	char callname[51];
	time_t callstart;
	time_t callfinish;
	int contactid;
	int action;
	int status;
	char details[1025];
} REC_CALL;
typedef struct {
	int contactid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char loginip[21];
	time_t logintime;
	char logintoken[51];
	char username[51];
	char password[51];
	int enabled;
	short int geozone;
	short int timezone;
	char surname[51];
	char givenname[51];
	char salutation[51];
	char contacttype[51];
	char referredby[51];
	char altcontact[51];
	char prefbilling[51];
	char email[51];
	char homenumber[26];
	char worknumber[26];
	char faxnumber[26];
	char mobilenumber[26];
	char jobtitle[51];
	char organization[51];
	char homeaddress[51];
	char homelocality[51];
	char homeregion[51];
	char homecountry[51];
	char homepostalcode[11];
	char workaddress[51];
	char worklocality[51];
	char workregion[51];
	char workcountry[51];
	char workpostalcode[11];
} REC_CONTACT;
typedef struct {
	int eventid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int assignedby;
	int assignedto;
	char eventname[51];
	int eventtype;
	int contactid;
	int priority;
	int reminder;
	time_t eventstart;
	time_t eventfinish;
	int busy;
	int status;
	int closingstatus;
	char details[1025];
} REC_EVENT;
typedef struct {
	int fileid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char filename[256];
	char filepath[256];
	char filetype[11];
	time_t uldate;
	time_t lastdldate;
	int numdownloads;
	char description[1025];
} REC_FILE;
typedef struct {
	int forumgroupid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char title[51];
	char description[1025];
} REC_FORUMGROUP;
typedef struct {
	int groupid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char groupname[51];
	char availability[169];
	char motd[1025];
	char members[1025];
} REC_GROUP;
typedef struct {
	int mailaccountid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char accountname[51];
	char realname[51];
	char organization[51];
	char address[51];
	char replyto[51];
	char hosttype[12];
	char pophost[51];
	int popport;
	char smtphost[51];
	int smtpport;
	char popusername[51];
	char poppassword[51];
	char smtpauth[10];
	int lastcount;
	int notify;
	int remove;
	time_t lastcheck;
	char signature[1025];
} REC_MAILACCT;
typedef struct {
	int noteid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char tablename[51];
	int tableindex;
	char notetitle[51];
	char notetext[8193];
} REC_NOTE;
typedef struct {
	int orderid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int contactid;
	int userid;
	time_t orderdate;
	char ordertype[51];
	char paymentmethod[51];
	float paymentdue;
	float paymentreceived;
	int status;
	char details[1025];
} REC_ORDER;
typedef struct {
	int orderitemid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int orderid;
	int productid;
	float quantity;
	float discount;
	float unitprice;
	float internalcost;
	float tax1;
	float tax2;
} REC_ORDERITEM;
typedef struct {
	int productid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char productname[51];
	char category[51];
	float discount;
	float unitprice;
	float internalcost;
	float tax1;
	float tax2;
	char details[1025];
} REC_PRODUCT;
typedef struct {
	int queryid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char queryname[51];
	char query[1025];
} REC_QUERY;
typedef struct {
	int taskid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	int assignedby;
	int assignedto;
	char taskname[51];
	time_t duedate;
	int priority;
	int reminder;
	int status;
	char details[1025];
} REC_TASK;
typedef struct {
	int userid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char loginip[21];
	time_t logintime;
	char logintoken[51];
	char username[51];
	char password[51];
	int groupid;
	int enabled;
	int authadmin;
	int authbookmarks;
	int authcalendar;
	int authcalls;
	int authcontacts;
	int authfiles;
	int authforums;
	int authmessages;
	int authorders;
	int authprofile;
	int authquery;
	int authwebmail;
	int prefdaystart;
	int prefdaylength;
	int prefmailcurrent;
	int prefmaildefault;
	int prefmaxlist;
	int prefmenustyle;
	short int preftimezone;
	short int prefgeozone;
	char availability[169];
	char surname[51];
	char givenname[51];
	char jobtitle[51];
	char division[51];
	char supervisor[51];
	char address[51];
	char locality[51];
	char region[51];
	char country[51];
	char postalcode[11];
	char homenumber[26];
	char worknumber[26];
	char faxnumber[26];
	char cellnumber[26];
	char pagernumber[26];
	char email[51];
	char birthdate[12];
	char hiredate[12];
	char sin[16];
	char isactive[11];
} REC_USER;
typedef struct {
	int zoneid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char zonename[51];
} REC_ZONE;

typedef	int   (*MAIN_AUTH_SETCOOKIE)(CONN *);
typedef	int   (*MAIN_AUTH_PRIV)(CONN *, char *);
typedef	char *(*MAIN_AUTH_SETPASS)(CONN *, char *);
typedef int   (*MAIN_CONFIG_READ)(CONFIG *);
typedef int   (*MAIN_CONFIG_WRITE)(CONFIG *);
typedef	int   (*MAIN_DB_LOG_ACTIVITY)(CONN *, int, char *, int, char *, const char *, ...);
typedef	void  (*MAIN_LOGACCESS)(CONN *, const char *, ...);
typedef	void  (*MAIN_LOGERROR)(CONN *, char *, int, int, const char *, ...);
typedef	void  (*MAIN_HTPAGE_HEADER)(CONN *, char *);
typedef	void  (*MAIN_HTPAGE_FOOTER)(CONN *);
typedef	void  (*MAIN_HTPAGE_TOPMENU)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_CONTACT)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_DAY)(CONN *, char *);
typedef	void  (*MAIN_HTSELECT_EVENTSTATUS)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_GROUP)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_HOUR)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_MINUTES)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_MONTH)(CONN *, char *);
typedef	void  (*MAIN_HTSELECT_NUMBER)(CONN *, int, int, int, int);
typedef	void  (*MAIN_HTSELECT_PRIORITY)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_QHOURS)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_QMINUTES)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_REMINDER)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_TIME)(CONN *, time_t);
typedef	void  (*MAIN_HTSELECT_TIMEZONE)(CONN *, short int);
typedef	void  (*MAIN_HTSELECT_USER)(CONN *, int);
typedef	void  (*MAIN_HTSELECT_YEAR)(CONN *, int, char *);
typedef	void  (*MAIN_HTSELECT_ZONE)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_CALLACTION)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_CONTACT)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_EVENTCLOSINGSTATUS)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_EVENTSTATUS)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_EVENTTYPE)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_HOLIDAY)(char *);
typedef	char *(*MAIN_HTVIEW_REMINDER)(CONN *, int);
typedef	char *(*MAIN_HTVIEW_USER)(CONN *, int);
typedef void  (*MAIN_HTSCRIPT_SHOWPAGE)(CONN *, short int);
typedef	void  (*MAIN_MD5_INIT)(struct MD5Context *);
typedef	void  (*MAIN_MD5_UPDATE)(struct MD5Context *, unsigned char const *, unsigned);
typedef	void  (*MAIN_MD5_FINAL)(unsigned char*, struct MD5Context *);
typedef	char *(*MAIN_MD5_CRYPT)(CONN *, char *, char *);
typedef	void  (*MAIN_SQL_FREERESULT)(int);
typedef	char *(*MAIN_SQL_GETNAME)(int, int);
typedef	char *(*MAIN_SQL_GETVALUE)(int, int, int);
typedef	char *(*MAIN_SQL_GETVALUEBYNAME)(int, int, char *);
typedef	int   (*MAIN_SQL_NUMFIELDS)(int);
typedef	int   (*MAIN_SQL_NUMTUPLES)(int);
typedef	int   (*MAIN_SQL_QUERY)(CONN *, char *);
typedef	int   (*MAIN_SQL_QUERYF)(CONN *, char *, ...);
typedef	int   (*MAIN_SQL_UPDATE)(CONN *, char *);
typedef	int   (*MAIN_SQL_UPDATEF)(CONN *, const char *, ...);
typedef	char *(*MAIN_TIME_SQL2DATETEXT)(CONN *, char *);
typedef	char *(*MAIN_TIME_SQL2TIMETEXT)(CONN *, char *);
typedef	time_t(*MAIN_TIME_SQL2UNIX)(char *);
typedef	int   (*MAIN_TIME_TZOFFSET)(CONN *, time_t);
typedef	int   (*MAIN_TIME_TZOFFSET2)(CONN *, time_t, int);
typedef	int   (*MAIN_TIME_TZOFFSETCON)(CONN *, time_t, int);
typedef	char *(*MAIN_TIME_UNIX2SQL)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2TEXT)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2LOTIMETEXT)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2TIMETEXT)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2DATETEXT)(CONN *, time_t);
typedef	time_t(*MAIN_TIME_WMGETDATE)(char *);
typedef	char *(*MAIN_GETBUFFER)(CONN *);
typedef	char *(*MAIN_GETGETENV)(CONN *, char *);
typedef	char *(*MAIN_GETMIMEENV)(CONN *, char *, unsigned int *);
typedef	char *(*MAIN_GETPOSTENV)(CONN *, char *);
typedef	char *(*MAIN_GETXMLENV)(CONN *, char *);
typedef	char *(*MAIN_GETXMLPARAM)(CONN *, int, char *);
typedef	char *(*MAIN_GETXMLSTRUCT)(CONN *, char *, char *);
typedef	int   (*MAIN_PRINTS)(CONN *, const char *, ...);
typedef	int   (*MAIN_RAW_PRINTS)(CONN *, const char *, ...);
typedef	int   (*MAIN_PRINTHEX)(CONN *, const char *, ...);
typedef	int   (*MAIN_PRINTHT)(CONN *, const char *, ...);
typedef	void  (*MAIN_PRINTLINE)(CONN *, short int, char *);
typedef	void  (*MAIN_PRINTLINE2)(CONN *, int, char *);
typedef	char *(*MAIN_STR2HTML)(CONN *, char *);
typedef	char *(*MAIN_STRNCATF)(char *, int, const char *, ...);
typedef	void  (*MAIN_SEND_ERROR)(CONN *, int, char *, char *);
typedef	void  (*MAIN_SEND_HEADER)(CONN *, int, int, char *, char *, char *, int, time_t);
typedef	void  (*MAIN_DECODEURL)(unsigned char *);
typedef	char *(*MAIN_ENCODEURL)(CONN *, unsigned char *);
typedef	char *(*MAIN_DECODE_B64S)(CONN *, char *);
typedef	void  (*MAIN_FIXSLASHES)(char *);
typedef	void  (*MAIN_STRIPRN)(char *);
typedef	void  (*MAIN_FLUSHBUFFER)(CONN *);
typedef	char *(*MAIN_STR2SQL)(CONN *, char *);
typedef	char *(*MAIN_STR2SQLBUF)(CONN *, char *, char *, int);
typedef	char *(*MAIN_GET_MIME_TYPE)(char *);
typedef	int   (*MAIN_FILESEND)(CONN *, char *);
typedef	int   (*MAIN_TCP_SEND)(CONN *, int, const char *, int, int);
typedef	int   (*MAIN_CLOSECONNECT)(CONN *, int);
typedef	int   (*MAIN_HEX2INT)(char *);
typedef	char *(*MAIN_P_STRCASESTR)(char *, char *);
typedef	char *(*MAIN_SRV_RESTART)(void);
typedef	int   (*MAIN_MODULE_EXISTS)(CONN *, char *);
typedef	void *(*MAIN_MODULE_CALL)(CONN *, char *);
#ifdef WIN32
typedef	int   (*MAIN_GETTIMEOFDAY)(struct timeval *, struct timezone *);
typedef	DIR  *(*MAIN_OPENDIR)(char *);
typedef	struct	direct *(*MAIN_READDIR)(DIR *);
typedef	void  (*MAIN_CLOSEDIR)(DIR *);
#endif
//typedef	void  (*MAIN_NOTESSUBLIST)(void);

typedef	int   (*MOD_CONTACTS_READ)(CONN *, short int, int, REC_CONTACT *);
typedef	int   (*MOD_CONTACTS_WRITE)(CONN *, int, REC_CONTACT *);
