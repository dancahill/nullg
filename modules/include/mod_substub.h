/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include <sys/stat.h>
#include <sys/types.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include <netdb.h>
#include <sys/time.h>
#endif
#include "language-en.h"
#include "defines.h"
#include "typedefs.h"

typedef	int   (*MAIN_AUTH_PRIV)(CONNECTION *, int);
typedef	char *(*MAIN_AUTH_SETPASS)(CONNECTION *, char *);
typedef	int   (*MAIN_DB_READ)(CONNECTION *, short int, short int, int, void *);
typedef	int   (*MAIN_DB_WRITE)(CONNECTION *, short int, int, void *);
typedef	int   (*MAIN_DB_LOG_ACTIVITY)(CONNECTION *, int, char *, int, char *, const char *, ...);
typedef	void  (*MAIN_LOGACCESS)(CONNECTION *, int, const char *, ...);
typedef	void  (*MAIN_LOGERROR)(CONNECTION *, char *, int, const char *, ...);
typedef	void  (*MAIN_HTPAGE_HEADER)(CONNECTION *, char *);
typedef	void  (*MAIN_HTPAGE_FOOTER)(CONNECTION *);
typedef	void  (*MAIN_HTPAGE_TOPMENU)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_CONTACT)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_DAY)(CONNECTION *, char *);
typedef	void  (*MAIN_HTSELECT_EVENTSTATUS)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_GROUP)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_HOUR)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_MINUTES)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_MONTH)(CONNECTION *, char *);
typedef	void  (*MAIN_HTSELECT_NUMBER)(CONNECTION *, int, int, int);
typedef	void  (*MAIN_HTSELECT_PRIORITY)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_QHOURS)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_QMINUTES)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_REMINDER)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_TIME)(CONNECTION *, time_t);
typedef	void  (*MAIN_HTSELECT_TIMEZONE)(CONNECTION *, short int);
typedef	void  (*MAIN_HTSELECT_USER)(CONNECTION *, int);
typedef	void  (*MAIN_HTSELECT_YEAR)(CONNECTION *, int, char *);
typedef	void  (*MAIN_HTSELECT_ZONE)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_CALLACTION)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_CONTACT)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_EVENTCLOSINGSTATUS)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_EVENTSTATUS)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_EVENTTYPE)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_HOLIDAY)(char *);
typedef	char *(*MAIN_HTVIEW_REMINDER)(CONNECTION *, int);
typedef	char *(*MAIN_HTVIEW_USER)(CONNECTION *, int);
typedef	void  (*MAIN_SQL_FREERESULT)(int);
typedef	char *(*MAIN_SQL_GETNAME)(int, int);
typedef	char *(*MAIN_SQL_GETVALUE)(int, int, int);
typedef	char *(*MAIN_SQL_GETVALUEBYNAME)(int, int, char *);
typedef	int   (*MAIN_SQL_NUMFIELDS)(int);
typedef	int   (*MAIN_SQL_NUMTUPLES)(int);
typedef	int   (*MAIN_SQL_QUERY)(CONNECTION *, char *);
typedef	int   (*MAIN_SQL_QUERYF)(CONNECTION *, char *, ...);
typedef	int   (*MAIN_SQL_UPDATE)(CONNECTION *, char *);
typedef	int   (*MAIN_SQL_UPDATEF)(CONNECTION *, const char *, ...);
typedef	char *(*MAIN_TIME_SQL2DATETEXT)(CONNECTION *, char *);
typedef	char *(*MAIN_TIME_SQL2TIMETEXT)(CONNECTION *, char *);
typedef	time_t(*MAIN_TIME_SQL2UNIX)(char *);
typedef	int   (*MAIN_TIME_TZOFFSET)(CONNECTION *, time_t);
typedef	int   (*MAIN_TIME_TZOFFSET2)(CONNECTION *, time_t, int);
typedef	char *(*MAIN_TIME_UNIX2SQL)(CONNECTION *, time_t);
typedef	char *(*MAIN_TIME_UNIX2TEXT)(CONNECTION *, time_t);
typedef	char *(*MAIN_TIME_UNIX2LOTIMETEXT)(CONNECTION *, time_t);
typedef	char *(*MAIN_TIME_UNIX2TIMETEXT)(CONNECTION *, time_t);
typedef	char *(*MAIN_TIME_UNIX2DATETEXT)(CONNECTION *, time_t);
typedef	time_t(*MAIN_TIME_WMGETDATE)(char *);
typedef	char *(*MAIN_GETBUFFER)(CONNECTION *);
typedef	char *(*MAIN_GETGETENV)(CONNECTION *, char *);
typedef	char *(*MAIN_GETMIMEENV)(CONNECTION *, char *, unsigned int *);
typedef	char *(*MAIN_GETPOSTENV)(CONNECTION *, char *);
typedef	int   (*MAIN_PRINTS)(CONNECTION *, const char *, ...);
typedef	int   (*MAIN_PRINTHEX)(CONNECTION *, const char *, ...);
typedef	int   (*MAIN_PRINTHT)(CONNECTION *, const char *, ...);
typedef	void  (*MAIN_PRINTLINE)(CONNECTION *, short int, char *);
typedef	void  (*MAIN_PRINTLINE2)(CONNECTION *, int, char *);
typedef	char *(*MAIN_STR2HTML)(CONNECTION *, char *);
typedef	char *(*MAIN_STRNCATF)(char *, int, const char *, ...);
typedef	void  (*MAIN_SEND_ERROR)(CONNECTION *, int, char *, char *);
typedef	void  (*MAIN_SEND_HEADER)(CONNECTION *, int, int, char *, char *, char *, int, time_t);
typedef	void  (*MAIN_DECODEURL)(unsigned char *);
typedef	void  (*MAIN_FIXSLASHES)(char *);
typedef	void  (*MAIN_STRIPRN)(char *);
typedef	void  (*MAIN_FLUSHBUFFER)(CONNECTION *);
typedef	char *(*MAIN_STR2SQL)(CONNECTION *, char *);
typedef	char *(*MAIN_STR2SQLBUF)(CONNECTION *, char *, char *, int);
typedef	char *(*MAIN_GET_MIME_TYPE)(char *);
typedef	int   (*MAIN_FILESEND)(CONNECTION *, char *);
typedef	int   (*MAIN_CLOSECONNECT)(CONNECTION *, int);
typedef	int   (*MAIN_HEX2INT)(char *);
typedef	char *(*MAIN_P_STRCASESTR)(char *, char *);
typedef	char *(*MAIN_SRV_RESTART)(void);
typedef	void *(*MAIN_MODULE_CALL)(CONNECTION *, char *);
#ifdef WIN32
typedef	int   (*MAIN_GETTIMEOFDAY)(struct timeval *, struct timezone *);
typedef	DIR  *(*MAIN_OPENDIR)(char *);
typedef	struct	direct *(*MAIN_READDIR)(DIR *);
typedef	void  (*MAIN_CLOSEDIR)(DIR *);
#endif
//typedef	void  (*MAIN_NOTESSUBLIST)(void);

/* auth.c functions */
MAIN_AUTH_PRIV				auth_priv;
MAIN_AUTH_SETPASS			auth_setpass;
/* connio.c functions */
MAIN_PRINTS				prints;
MAIN_PRINTHEX				printhex;
MAIN_PRINTHT				printht;
MAIN_PRINTLINE				printline;
MAIN_PRINTLINE2				printline2;
/* dbio.c */
MAIN_DB_READ				db_read;
MAIN_DB_WRITE				db_write;
MAIN_DB_LOG_ACTIVITY			db_log_activity;
/* format.c */
MAIN_TIME_SQL2DATETEXT			time_sql2datetext;
MAIN_TIME_SQL2TIMETEXT			time_sql2timetext;
MAIN_TIME_SQL2UNIX			time_sql2unix;
MAIN_TIME_TZOFFSET			time_tzoffset;
MAIN_TIME_TZOFFSET2			time_tzoffset2;
MAIN_TIME_UNIX2SQL			time_unix2sql;
MAIN_TIME_UNIX2TEXT			time_unix2text;
MAIN_TIME_UNIX2LOTIMETEXT		time_unix2lotimetext;
MAIN_TIME_UNIX2TIMETEXT			time_unix2timetext;
MAIN_TIME_UNIX2DATETEXT			time_unix2datetext;
MAIN_TIME_WMGETDATE			time_wmgetdate;
/* html.c */
MAIN_HTPAGE_HEADER			htpage_header;
MAIN_HTPAGE_FOOTER			htpage_footer;
MAIN_HTPAGE_TOPMENU			htpage_topmenu;
MAIN_HTSELECT_CONTACT			htselect_contact;
MAIN_HTSELECT_DAY			htselect_day;
MAIN_HTSELECT_GROUP			htselect_group;
MAIN_HTSELECT_EVENTSTATUS		htselect_eventstatus;
MAIN_HTSELECT_HOUR			htselect_hour;
MAIN_HTSELECT_NUMBER			htselect_number;
MAIN_HTSELECT_MINUTES			htselect_minutes;
MAIN_HTSELECT_MONTH			htselect_month;
MAIN_HTSELECT_PRIORITY			htselect_priority;
MAIN_HTSELECT_QHOURS			htselect_qhours;
MAIN_HTSELECT_QMINUTES			htselect_qminutes;
MAIN_HTSELECT_REMINDER			htselect_reminder;
MAIN_HTSELECT_TIME			htselect_time;
MAIN_HTSELECT_TIMEZONE			htselect_timezone;
MAIN_HTSELECT_USER			htselect_user;
MAIN_HTSELECT_YEAR			htselect_year;
MAIN_HTSELECT_ZONE			htselect_zone;
MAIN_HTVIEW_CALLACTION			htview_callaction;
MAIN_HTVIEW_CONTACT			htview_contact;
MAIN_HTVIEW_EVENTCLOSINGSTATUS		htview_eventclosingstatus;
MAIN_HTVIEW_EVENTSTATUS			htview_eventstatus;
MAIN_HTVIEW_EVENTTYPE			htview_eventtype;
MAIN_HTVIEW_HOLIDAY			htview_holiday;
MAIN_HTVIEW_REMINDER			htview_reminder;
MAIN_HTVIEW_USER			htview_user;
/* http.c functions */
MAIN_GET_MIME_TYPE			get_mime_type;
MAIN_GETGETENV				getgetenv;
MAIN_GETMIMEENV				getmimeenv;
MAIN_GETPOSTENV				getpostenv;
/* logging.c functions */
MAIN_LOGACCESS				logaccess;
MAIN_LOGERROR				logerror;
/* sql.c functions */
MAIN_SQL_FREERESULT			sql_freeresult;
MAIN_SQL_GETNAME			sql_getname;
MAIN_SQL_GETVALUE			sql_getvalue;
MAIN_SQL_GETVALUEBYNAME			sql_getvaluebyname;
MAIN_SQL_NUMFIELDS			sql_numfields;
MAIN_SQL_NUMTUPLES			sql_numtuples;
MAIN_SQL_QUERY				sql_query;
MAIN_SQL_QUERYF				sql_queryf;
MAIN_SQL_UPDATE				sql_update;
MAIN_SQL_UPDATEF			sql_updatef;

MAIN_DECODEURL				decodeurl;
MAIN_GETBUFFER				getbuffer;
MAIN_STR2HTML				str2html;
MAIN_STRNCATF				strncatf;
MAIN_SEND_ERROR				send_error;
MAIN_SEND_HEADER			send_header;
MAIN_FIXSLASHES				fixslashes;
MAIN_STRIPRN				striprn;
MAIN_FLUSHBUFFER			flushbuffer;
MAIN_STR2SQL				str2sql;
MAIN_STR2SQLBUF				str2sqlbuf;
MAIN_FILESEND				filesend;
MAIN_HEX2INT				hex2int;
MAIN_CLOSECONNECT			closeconnect;
MAIN_P_STRCASESTR			p_strcasestr;
MAIN_SRV_RESTART			srv_restart;
MAIN_MODULE_CALL			module_call;
#ifdef WIN32
MAIN_GETTIMEOFDAY			gettimeofday;
MAIN_OPENDIR				opendir;
MAIN_READDIR				readdir;
MAIN_CLOSEDIR				closedir;
#endif

//MAIN_NOTESSUBLIST			notessublist;

CONFIG      *config;
FUNCTION    *functions;
MODULE_FUNC *mod_functions;
MODULE_MENU *mod_menuitems;
