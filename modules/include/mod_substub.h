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
#include "mod_typedefs.h"

typedef	int   (*MAIN_AUTH_SETCOOKIE)(CONN *);
typedef	int   (*MAIN_AUTH_PRIV)(CONN *, char *);
typedef	char *(*MAIN_AUTH_SETPASS)(CONN *, char *);
typedef int   (*MAIN_CONFIG_READ)(CONFIG *);
typedef int   (*MAIN_CONFIG_WRITE)(CONFIG *);
typedef	int   (*MAIN_DB_LOG_ACTIVITY)(CONN *, int, char *, int, char *, const char *, ...);
typedef	void  (*MAIN_LOGACCESS)(CONN *, int, const char *, ...);
typedef	void  (*MAIN_LOGERROR)(CONN *, char *, int, const char *, ...);
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
typedef	void  (*MAIN_HTSELECT_NUMBER)(CONN *, int, int, int);
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
typedef	int   (*MAIN_PRINTHEX)(CONN *, const char *, ...);
typedef	int   (*MAIN_PRINTHT)(CONN *, const char *, ...);
typedef	void  (*MAIN_PRINTLINE)(CONN *, short int, char *);
typedef	void  (*MAIN_PRINTLINE2)(CONN *, int, char *);
typedef	char *(*MAIN_STR2HTML)(CONN *, char *);
typedef	char *(*MAIN_STRNCATF)(char *, int, const char *, ...);
typedef	void  (*MAIN_SEND_ERROR)(CONN *, int, char *, char *);
typedef	void  (*MAIN_SEND_HEADER)(CONN *, int, int, char *, char *, char *, int, time_t);
typedef	void  (*MAIN_DECODEURL)(unsigned char *);
typedef	char *(*MAIN_DECODE_B64S)(CONN *, char *);
typedef	void  (*MAIN_FIXSLASHES)(char *);
typedef	void  (*MAIN_STRIPRN)(char *);
typedef	void  (*MAIN_FLUSHBUFFER)(CONN *);
typedef	char *(*MAIN_STR2SQL)(CONN *, char *);
typedef	char *(*MAIN_STR2SQLBUF)(CONN *, char *, char *, int);
typedef	char *(*MAIN_GET_MIME_TYPE)(char *);
typedef	int   (*MAIN_FILESEND)(CONN *, char *);
typedef	int   (*MAIN_TCP_SEND)(int, const char *, int, int);
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

/* auth.c functions */
MAIN_AUTH_PRIV				auth_priv;
MAIN_AUTH_SETCOOKIE			auth_setcookie;
MAIN_AUTH_SETPASS			auth_setpass;
/* config.c functions */
MAIN_CONFIG_READ			config_read;
MAIN_CONFIG_WRITE			config_write;
/* connio.c functions */
MAIN_PRINTS				prints;
MAIN_PRINTHEX				printhex;
MAIN_PRINTHT				printht;
MAIN_PRINTLINE				printline;
MAIN_PRINTLINE2				printline2;
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
MAIN_GETXMLENV				getxmlenv;
MAIN_GETXMLPARAM			getxmlparam;
MAIN_GETXMLSTRUCT			getxmlstruct;
/* log.c functions */
MAIN_DB_LOG_ACTIVITY			db_log_activity;
MAIN_LOGACCESS				logaccess;
MAIN_LOGERROR				logerror;
/* md5.c functions */
MAIN_MD5_INIT				md5_init;
MAIN_MD5_UPDATE				md5_update;
MAIN_MD5_FINAL				md5_final;
MAIN_MD5_CRYPT				md5_crypt;
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
MAIN_DECODE_B64S			decode_b64s;
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
MAIN_TCP_SEND				tcp_send;
MAIN_HEX2INT				hex2int;
MAIN_CLOSECONNECT			closeconnect;
MAIN_P_STRCASESTR			p_strcasestr;
MAIN_SRV_RESTART			srv_restart;
MAIN_MODULE_EXISTS			module_exists;
MAIN_MODULE_CALL			module_call;
#ifdef WIN32
MAIN_GETTIMEOFDAY			gettimeofday;
MAIN_OPENDIR				opendir;
MAIN_READDIR				readdir;
MAIN_CLOSEDIR				closedir;
#endif

CONFIG   *config;
FUNCTION *functions;
_PROC    *proc;
