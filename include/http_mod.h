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

#ifdef WIN32
#include "config-nt.h"
#else
#include "config.h"
#endif
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define _MT 1
//#pragma comment(lib, "libcmt.lib")
#include <winsock2.h>
#include <windows.h>
#include <process.h>
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
#ifdef HAVE_LIBSSL
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include "defines.h"
#include "typedefs.h"
#include "http_records.h"
#include "http_typedefs.h"

/*
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
*/








typedef	int   (*MAIN_AUTH_SETCOOKIE)(CONN *);
typedef	int   (*MAIN_AUTH_PRIV)(CONN *, char *);
typedef	char *(*MAIN_AUTH_SETPASS)(CONN *, char *);
typedef int   (*MAIN_CONFIG_READ)(CONFIG *);
typedef int   (*MAIN_CONFIG_WRITE)(CONFIG *);
typedef	char *(*MAIN_DECODE_BASE64)(char *, int, char *);
typedef	char *(*MAIN_GETBUFFER)(CONN *);
typedef	char *(*MAIN_GET_MIME_TYPE)(char *);
typedef	char *(*MAIN_GETGETENV)(CONN *, char *);
typedef	char *(*MAIN_GETMIMEENV)(CONN *, char *, unsigned int *);
typedef	char *(*MAIN_GETPOSTENV)(CONN *, char *);
typedef	char *(*MAIN_GETXMLENV)(CONN *, char *);
typedef	char *(*MAIN_GETXMLPARAM)(CONN *, int, char *);
typedef	char *(*MAIN_GETXMLSTRUCT)(CONN *, char *, char *);
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
typedef	int   (*MAIN_DB_LOG_ACTIVITY)(CONN *, int, char *, int, char *, const char *, ...);
typedef	void  (*MAIN_LOG_ACCESS)(char *, const char *, ...);
typedef	void  (*MAIN_LOG_ERROR)(char *, char *, int, int, const char *, ...);
typedef	void  (*MAIN_MD5_INIT)(struct MD5Context *);
typedef	void  (*MAIN_MD5_UPDATE)(struct MD5Context *, unsigned char const *, unsigned);
typedef	void  (*MAIN_MD5_FINAL)(unsigned char*, struct MD5Context *);
typedef	char *(*MAIN_MD5_CRYPT)(char *, char *, char *);
typedef	void  (*MAIN_SQL_FREERESULT)(int);
typedef	int   (*MAIN_SQL_UPDATE)(char *);
typedef	int   (*MAIN_SQL_UPDATEF)(const char *, ...);
typedef	int   (*MAIN_SQL_QUERY)(char *);
typedef	int   (*MAIN_SQL_QUERYF)(char *, ...);
typedef	char *(*MAIN_SQL_GETNAME)(int, int);
typedef	char *(*MAIN_SQL_GETVALUE)(int, int, int);
typedef	char *(*MAIN_SQL_GETVALUEBYNAME)(int, int, char *);
typedef	int   (*MAIN_SQL_NUMFIELDS)(int);
typedef	int   (*MAIN_SQL_NUMTUPLES)(int);
//typedef	int   (*MAIN_TCP_BIND)(char *, unsigned short);
//typedef	int   (*MAIN_TCP_ACCEPT)(int, struct sockaddr *);
typedef	int   (*MAIN_TCP_FGETS)(char *, int, TCP_SOCKET *);
typedef	int   (*MAIN_TCP_FPRINTF)(TCP_SOCKET *, const char *, ...);
typedef	int   (*MAIN_TCP_RECV)(TCP_SOCKET *, char *, int, int);
typedef	int   (*MAIN_TCP_SEND)(TCP_SOCKET *, const char *, int, int);
typedef	int   (*MAIN_TCP_CLOSE)(TCP_SOCKET *);
typedef	time_t(*MAIN_TIME_SQL2UNIX)(char *);
typedef	char *(*MAIN_TIME_UNIX2SQL)(char *, int, time_t);
typedef	char *(*MAIN_TIME_SQL2DATETEXT)(CONN *, char *);
typedef	char *(*MAIN_TIME_SQL2TIMETEXT)(CONN *, char *);
typedef	int   (*MAIN_TIME_TZOFFSET)(CONN *, time_t);
typedef	int   (*MAIN_TIME_TZOFFSET2)(CONN *, time_t, int);
typedef	int   (*MAIN_TIME_TZOFFSETCON)(CONN *, time_t, int);
typedef	char *(*MAIN_TIME_UNIX2TEXT)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2LOTIMETEXT)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2TIMETEXT)(CONN *, time_t);
typedef	char *(*MAIN_TIME_UNIX2DATETEXT)(CONN *, time_t);
typedef	time_t(*MAIN_TIME_WMGETDATE)(char *);
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
typedef	char *(*MAIN_ENCODEURL)(CONN *, unsigned char *);
typedef	void  (*MAIN_STRIPRN)(char *);
typedef	void  (*MAIN_FLUSHBUFFER)(CONN *);
typedef	void  (*MAIN_FIXSLASHES)(char *);
typedef	char *(*MAIN_STR2SQL)(char *, int, char *);
typedef	int   (*MAIN_FILESEND)(CONN *, char *);
typedef	int   (*MAIN_CLOSECONNECT)(CONN *, int);
typedef	int   (*MAIN_HEX2INT)(char *);
typedef	char *(*MAIN_P_STRCASESTR)(char *, char *);
typedef	int   (*MAIN_MODULE_EXISTS)(CONN *, char *);
typedef	void *(*MAIN_MODULE_CALL)(CONN *, char *);
#ifdef WIN32
typedef	int   (*MAIN_GETTIMEOFDAY)(struct timeval *, struct timezone *);
typedef	DIR  *(*MAIN_OPENDIR)(char *);
typedef	struct direct *(*MAIN_READDIR)(DIR *);
typedef	void  (*MAIN_CLOSEDIR)(DIR *);
#endif

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

#ifdef SRVMOD_MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN MAIN_AUTH_SETCOOKIE		auth_setcookie;
EXTERN MAIN_AUTH_PRIV			auth_priv;
EXTERN MAIN_AUTH_SETPASS		auth_setpass;
EXTERN MAIN_CONFIG_READ			config_read;
EXTERN MAIN_CONFIG_WRITE		config_write;
EXTERN MAIN_DECODE_BASE64		decode_base64;
EXTERN MAIN_GETBUFFER			getbuffer;
EXTERN MAIN_GET_MIME_TYPE		get_mime_type;
EXTERN MAIN_GETGETENV			getgetenv;
EXTERN MAIN_GETMIMEENV			getmimeenv;
EXTERN MAIN_GETPOSTENV			getpostenv;
EXTERN MAIN_GETXMLENV			getxmlenv;
EXTERN MAIN_GETXMLPARAM			getxmlparam;
EXTERN MAIN_GETXMLSTRUCT		getxmlstruct;
EXTERN MAIN_HTPAGE_HEADER		htpage_header;
EXTERN MAIN_HTPAGE_FOOTER		htpage_footer;
EXTERN MAIN_HTPAGE_TOPMENU		htpage_topmenu;
EXTERN MAIN_HTSELECT_CONTACT		htselect_contact;
EXTERN MAIN_HTSELECT_DAY		htselect_day;
EXTERN MAIN_HTSELECT_GROUP		htselect_group;
EXTERN MAIN_HTSELECT_EVENTSTATUS	htselect_eventstatus;
EXTERN MAIN_HTSELECT_HOUR		htselect_hour;
EXTERN MAIN_HTSELECT_NUMBER		htselect_number;
EXTERN MAIN_HTSELECT_MINUTES		htselect_minutes;
EXTERN MAIN_HTSELECT_MONTH		htselect_month;
EXTERN MAIN_HTSELECT_PRIORITY		htselect_priority;
EXTERN MAIN_HTSELECT_QHOURS		htselect_qhours;
EXTERN MAIN_HTSELECT_QMINUTES		htselect_qminutes;
EXTERN MAIN_HTSELECT_REMINDER		htselect_reminder;
EXTERN MAIN_HTSELECT_TIME		htselect_time;
EXTERN MAIN_HTSELECT_TIMEZONE		htselect_timezone;
EXTERN MAIN_HTSELECT_USER		htselect_user;
EXTERN MAIN_HTSELECT_YEAR		htselect_year;
EXTERN MAIN_HTSELECT_ZONE		htselect_zone;
EXTERN MAIN_HTVIEW_CALLACTION		htview_callaction;
EXTERN MAIN_HTVIEW_CONTACT		htview_contact;
EXTERN MAIN_HTVIEW_EVENTCLOSINGSTATUS	htview_eventclosingstatus;
EXTERN MAIN_HTVIEW_EVENTSTATUS		htview_eventstatus;
EXTERN MAIN_HTVIEW_EVENTTYPE		htview_eventtype;
EXTERN MAIN_HTVIEW_HOLIDAY		htview_holiday;
EXTERN MAIN_HTVIEW_REMINDER		htview_reminder;
EXTERN MAIN_HTVIEW_USER			htview_user;
EXTERN MAIN_HTSCRIPT_SHOWPAGE		htscript_showpage;
EXTERN MAIN_DB_LOG_ACTIVITY		db_log_activity;
EXTERN MAIN_LOG_ACCESS			log_access;
EXTERN MAIN_LOG_ERROR			log_error;
EXTERN MAIN_MD5_INIT			md5_init;
EXTERN MAIN_MD5_UPDATE			md5_update;
EXTERN MAIN_MD5_FINAL			md5_final;
EXTERN MAIN_MD5_CRYPT			md5_crypt;
EXTERN MAIN_SQL_FREERESULT		sql_freeresult;
EXTERN MAIN_SQL_UPDATE			sql_update;
EXTERN MAIN_SQL_UPDATEF			sql_updatef;
EXTERN MAIN_SQL_QUERY			sql_query;
EXTERN MAIN_SQL_QUERYF			sql_queryf;
EXTERN MAIN_SQL_GETNAME			sql_getname;
EXTERN MAIN_SQL_GETVALUE		sql_getvalue;
EXTERN MAIN_SQL_GETVALUEBYNAME		sql_getvaluebyname;
EXTERN MAIN_SQL_NUMFIELDS		sql_numfields;
EXTERN MAIN_SQL_NUMTUPLES		sql_numtuples;
//EXTERN MAIN_TCP_BIND			tcp_bind;
//EXTERN MAIN_TCP_ACCEPT			tcp_accept;
EXTERN MAIN_TCP_FGETS			tcp_fgets;
EXTERN MAIN_TCP_FPRINTF			tcp_fprintf;
EXTERN MAIN_TCP_RECV			tcp_recv;
EXTERN MAIN_TCP_SEND			tcp_send;
EXTERN MAIN_TCP_CLOSE			tcp_close;
EXTERN MAIN_TIME_SQL2UNIX		time_sql2unix;
EXTERN MAIN_TIME_UNIX2SQL		time_unix2sql;
EXTERN MAIN_TIME_SQL2DATETEXT		time_sql2datetext;
EXTERN MAIN_TIME_SQL2TIMETEXT		time_sql2timetext;
EXTERN MAIN_TIME_TZOFFSET		time_tzoffset;
EXTERN MAIN_TIME_TZOFFSET2		time_tzoffset2;
EXTERN MAIN_TIME_TZOFFSETCON		time_tzoffsetcon;
EXTERN MAIN_TIME_UNIX2TEXT		time_unix2text;
EXTERN MAIN_TIME_UNIX2LOTIMETEXT	time_unix2lotimetext;
EXTERN MAIN_TIME_UNIX2TIMETEXT		time_unix2timetext;
EXTERN MAIN_TIME_UNIX2DATETEXT		time_unix2datetext;
EXTERN MAIN_TIME_WMGETDATE		time_wmgetdate;
EXTERN MAIN_PRINTS			prints;
EXTERN MAIN_PRINTHEX			printhex;
EXTERN MAIN_PRINTHT			printht;
EXTERN MAIN_PRINTLINE			printline;
EXTERN MAIN_PRINTLINE2			printline2;
EXTERN MAIN_STR2HTML			str2html;
EXTERN MAIN_STRNCATF			strncatf;
EXTERN MAIN_SEND_ERROR			send_error;
EXTERN MAIN_SEND_HEADER			send_header;
EXTERN MAIN_DECODEURL			decodeurl;
EXTERN MAIN_ENCODEURL			encodeurl;
EXTERN MAIN_STRIPRN			striprn;
EXTERN MAIN_FLUSHBUFFER			flushbuffer;
EXTERN MAIN_FIXSLASHES			fixslashes;
EXTERN MAIN_STR2SQL			str2sql;
EXTERN MAIN_FILESEND			filesend;
EXTERN MAIN_CLOSECONNECT		closeconnect;
EXTERN MAIN_HEX2INT			hex2int;
EXTERN MAIN_P_STRCASESTR		p_strcasestr;
EXTERN MAIN_MODULE_EXISTS		module_exists;
EXTERN MAIN_MODULE_CALL			module_call;
#ifdef WIN32
EXTERN MAIN_GETTIMEOFDAY		gettimeofday;
EXTERN MAIN_OPENDIR			opendir;
EXTERN MAIN_READDIR			readdir;
EXTERN MAIN_CLOSEDIR			closedir;
#endif

EXTERN CONFIG    *config;
EXTERN FUNCTION  *functions;
EXTERN _PROC     *proc;
EXTERN CONN      *conn;
EXTERN HTTP_PROC *http_proc;

#ifdef SRVMOD_MAIN
void __log_error(char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	time_t t=time(NULL);

	snprintf(file, sizeof(file)-1, "%s/%s-error.log", config->server_dir_var_log, logsrc);
	fp=fopen(file, "a");
	if (fp!=NULL) {
		va_start(ap, format);
		vsnprintf(logbuffer, sizeof(logbuffer)-1, format, ap);
		va_end(ap);
		strftime(timebuffer, sizeof(timebuffer), "%b %d %H:%M:%S", gmtime(&t));
		fprintf(fp, "%s - [%d] %s %d %s\n", timebuffer, loglevel, srcfile, line, logbuffer);
		fclose(fp);
	}
}

int _get_func(void **fn, char *fn_name)
{
	int i;

	for (i=0;;i++) {
		if ((functions[i].fn_name==NULL)||(functions[i].fn_ptr==NULL)) break;
		if (strcmp(functions[i].fn_name, fn_name)==0) {
			*fn=functions[i].fn_ptr;
			return 0;
		}
	}
	__log_error("httpmod", __FILE__, __LINE__, 0, "ERROR: Failed to find function %s", fn_name);
	return -1;
}

int mod_import()
{
	log_error=NULL;
	if (_get_func((void *)&log_error,			"log_error"			)!=0) return -1;
	if (_get_func((void *)&log_access,			"log_access"			)!=0) return -1;
	if (_get_func((void *)&db_log_activity,			"db_log_activity"		)!=0) return -1;
	if (_get_func((void *)&auth_setcookie,			"auth_setcookie"		)!=0) return -1;
	if (_get_func((void *)&auth_priv,			"auth_priv"			)!=0) return -1;
	if (_get_func((void *)&auth_setpass,			"auth_setpass"			)!=0) return -1;
	if (_get_func((void *)&config_read,			"config_read"			)!=0) return -1;
	if (_get_func((void *)&config_write,			"config_write"			)!=0) return -1;
	if (_get_func((void *)&decode_base64,			"decode_base64"			)!=0) return -1;
	if (_get_func((void *)&getbuffer,			"getbuffer"			)!=0) return -1;
	if (_get_func((void *)&get_mime_type,			"get_mime_type"			)!=0) return -1;
	if (_get_func((void *)&getgetenv,			"getgetenv"			)!=0) return -1;
	if (_get_func((void *)&getmimeenv,			"getmimeenv"			)!=0) return -1;
	if (_get_func((void *)&getpostenv,			"getpostenv"			)!=0) return -1;
	if (_get_func((void *)&getxmlenv,			"getxmlenv"			)!=0) return -1;
	if (_get_func((void *)&getxmlparam,			"getxmlparam"			)!=0) return -1;
	if (_get_func((void *)&getxmlstruct,			"getxmlstruct"			)!=0) return -1;
	if (_get_func((void *)&htpage_header,			"htpage_header"			)!=0) return -1;
	if (_get_func((void *)&htpage_footer,			"htpage_footer"			)!=0) return -1;
	if (_get_func((void *)&htpage_topmenu,			"htpage_topmenu"		)!=0) return -1;
	if (_get_func((void *)&htselect_contact,		"htselect_contact"		)!=0) return -1;
	if (_get_func((void *)&htselect_day,			"htselect_day"			)!=0) return -1;
	if (_get_func((void *)&htselect_group,			"htselect_group"		)!=0) return -1;
	if (_get_func((void *)&htselect_eventstatus,		"htselect_eventstatus"		)!=0) return -1;
	if (_get_func((void *)&htselect_hour,			"htselect_hour"			)!=0) return -1;
	if (_get_func((void *)&htselect_number,			"htselect_number"		)!=0) return -1;
	if (_get_func((void *)&htselect_minutes,		"htselect_minutes"		)!=0) return -1;
	if (_get_func((void *)&htselect_month,			"htselect_month"		)!=0) return -1;
	if (_get_func((void *)&htselect_priority,		"htselect_priority"		)!=0) return -1;
	if (_get_func((void *)&htselect_qhours,			"htselect_qhours"		)!=0) return -1;
	if (_get_func((void *)&htselect_qminutes,		"htselect_qminutes"		)!=0) return -1;
	if (_get_func((void *)&htselect_reminder,		"htselect_reminder"		)!=0) return -1;
	if (_get_func((void *)&htselect_time,			"htselect_time"			)!=0) return -1;
	if (_get_func((void *)&htselect_timezone,		"htselect_timezone"		)!=0) return -1;
	if (_get_func((void *)&htselect_user,			"htselect_user"			)!=0) return -1;
	if (_get_func((void *)&htselect_year,			"htselect_year"			)!=0) return -1;
	if (_get_func((void *)&htselect_zone,			"htselect_zone"			)!=0) return -1;
	if (_get_func((void *)&htview_callaction,		"htview_callaction"		)!=0) return -1;
	if (_get_func((void *)&htview_contact,			"htview_contact"		)!=0) return -1;
	if (_get_func((void *)&htview_eventclosingstatus,	"htview_eventclosingstatus"	)!=0) return -1;
	if (_get_func((void *)&htview_eventstatus,		"htview_eventstatus"		)!=0) return -1;
	if (_get_func((void *)&htview_eventtype,		"htview_eventtype"		)!=0) return -1;
	if (_get_func((void *)&htview_holiday,			"htview_holiday"		)!=0) return -1;
	if (_get_func((void *)&htview_reminder,			"htview_reminder"		)!=0) return -1;
	if (_get_func((void *)&htview_user,			"htview_user"			)!=0) return -1;
	if (_get_func((void *)&htscript_showpage,		"htscript_showpage"		)!=0) return -1;
	if (_get_func((void *)&md5_init,			"md5_init"			)!=0) return -1;
	if (_get_func((void *)&md5_update,			"md5_update"			)!=0) return -1;
	if (_get_func((void *)&md5_final,			"md5_final"			)!=0) return -1;
	if (_get_func((void *)&md5_crypt,			"md5_crypt"			)!=0) return -1;
	if (_get_func((void *)&sql_freeresult,			"sql_freeresult"		)!=0) return -1;
	if (_get_func((void *)&sql_update,			"sql_update"			)!=0) return -1;
	if (_get_func((void *)&sql_updatef,			"sql_updatef"			)!=0) return -1;
	if (_get_func((void *)&sql_query,			"sql_query"			)!=0) return -1;
	if (_get_func((void *)&sql_queryf,			"sql_queryf"			)!=0) return -1;
	if (_get_func((void *)&sql_getname,			"sql_getname"			)!=0) return -1;
	if (_get_func((void *)&sql_getvalue,			"sql_getvalue"			)!=0) return -1;
	if (_get_func((void *)&sql_getvaluebyname,		"sql_getvaluebyname"		)!=0) return -1;
	if (_get_func((void *)&sql_numfields,			"sql_numfields"			)!=0) return -1;
	if (_get_func((void *)&sql_numtuples,			"sql_numtuples"			)!=0) return -1;
//	if (_get_func((void *)&tcp_bind,			"tcp_bind"			)!=0) return -1;
//	if (_get_func((void *)&tcp_accept,			"tcp_accept"			)!=0) return -1;
	if (_get_func((void *)&tcp_fgets,			"tcp_fgets"			)!=0) return -1;
	if (_get_func((void *)&tcp_fprintf,			"tcp_fprintf"			)!=0) return -1;
	if (_get_func((void *)&tcp_recv,			"tcp_recv"			)!=0) return -1;
	if (_get_func((void *)&tcp_send,			"tcp_send"			)!=0) return -1;
	if (_get_func((void *)&tcp_close,			"tcp_close"			)!=0) return -1;
	if (_get_func((void *)&time_sql2unix,			"time_sql2unix"			)!=0) return -1;
	if (_get_func((void *)&time_unix2sql,			"time_unix2sql"			)!=0) return -1;
	if (_get_func((void *)&time_sql2datetext,		"time_sql2datetext"		)!=0) return -1;
	if (_get_func((void *)&time_sql2timetext,		"time_sql2timetext"		)!=0) return -1;
	if (_get_func((void *)&time_tzoffset,			"time_tzoffset"			)!=0) return -1;
	if (_get_func((void *)&time_tzoffset2,			"time_tzoffset2"		)!=0) return -1;
	if (_get_func((void *)&time_tzoffsetcon,		"time_tzoffsetcon"		)!=0) return -1;
	if (_get_func((void *)&time_unix2text,			"time_unix2text"		)!=0) return -1;
	if (_get_func((void *)&time_unix2lotimetext,		"time_unix2lotimetext"		)!=0) return -1;
	if (_get_func((void *)&time_unix2timetext,		"time_unix2timetext"		)!=0) return -1;
	if (_get_func((void *)&time_unix2datetext,		"time_unix2datetext"		)!=0) return -1;
	if (_get_func((void *)&time_wmgetdate,			"time_wmgetdate"		)!=0) return -1;
	if (_get_func((void *)&prints,				"prints"			)!=0) return -1;
	if (_get_func((void *)&printhex,			"printhex"			)!=0) return -1;
	if (_get_func((void *)&printht,				"printht"			)!=0) return -1;
	if (_get_func((void *)&printline,			"printline"			)!=0) return -1;
	if (_get_func((void *)&printline2,			"printline2"			)!=0) return -1;
	if (_get_func((void *)&str2html,			"str2html"			)!=0) return -1;
	if (_get_func((void *)&strncatf,			"strncatf"			)!=0) return -1;
	if (_get_func((void *)&send_error,			"send_error"			)!=0) return -1;
	if (_get_func((void *)&send_header,			"send_header"			)!=0) return -1;
	if (_get_func((void *)&decodeurl,			"decodeurl"			)!=0) return -1;
	if (_get_func((void *)&encodeurl,			"encodeurl"			)!=0) return -1;
	if (_get_func((void *)&striprn,				"striprn"			)!=0) return -1;
	if (_get_func((void *)&flushbuffer,			"flushbuffer"			)!=0) return -1;
	if (_get_func((void *)&fixslashes,			"fixslashes"			)!=0) return -1;
	if (_get_func((void *)&str2sql,				"str2sql"			)!=0) return -1;
	if (_get_func((void *)&filesend,			"filesend"			)!=0) return -1;
	if (_get_func((void *)&closeconnect,			"closeconnect"			)!=0) return -1;
	if (_get_func((void *)&hex2int,				"hex2int"			)!=0) return -1;
	if (_get_func((void *)&p_strcasestr,			"p_strcasestr"			)!=0) return -1;
	if (_get_func((void *)&module_exists,			"module_exists"			)!=0) return -1;
	if (_get_func((void *)&module_call,			"module_call"			)!=0) return -1;
#ifdef WIN32
	if (_get_func((void *)&gettimeofday,			"gettimeofday"			)!=0) return -1;
	if (_get_func((void *)&opendir,				"opendir"			)!=0) return -1;
	if (_get_func((void *)&readdir,				"readdir"			)!=0) return -1;
	if (_get_func((void *)&closedir,			"closedir"			)!=0) return -1;
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	return 0;
}

int mod_export_main(MODULE_MENU *newmod)
{
	int i;

	for (i=0;i<50;i++) {
		if (strlen(http_proc->mod_menuitems[i].mod_name)>0) continue;
		if (strlen(http_proc->mod_menuitems[i].mod_menuname)>0) continue;
		if (http_proc->mod_menuitems[i].fn_ptr!=NULL) continue;
		http_proc->mod_menuitems[i].mod_submenu=newmod->mod_submenu;
		snprintf(http_proc->mod_menuitems[i].mod_name, sizeof(http_proc->mod_menuitems[i].mod_name)-1, "%s", newmod->mod_name);
		snprintf(http_proc->mod_menuitems[i].mod_menuname, sizeof(http_proc->mod_menuitems[i].mod_menuname)-1, "%s", newmod->mod_menuname);
		snprintf(http_proc->mod_menuitems[i].mod_menuperm, sizeof(http_proc->mod_menuitems[i].mod_menuperm)-1, "%s", newmod->mod_menuperm);
		snprintf(http_proc->mod_menuitems[i].mod_menuuri, sizeof(http_proc->mod_menuitems[i].mod_menuuri)-1, "%s", newmod->mod_menuuri);
		snprintf(http_proc->mod_menuitems[i].fn_name, sizeof(http_proc->mod_menuitems[i].fn_name)-1, "%s", newmod->fn_name);
		snprintf(http_proc->mod_menuitems[i].fn_uri, sizeof(http_proc->mod_menuitems[i].fn_uri)-1, "%s", newmod->fn_uri);
		http_proc->mod_menuitems[i].fn_ptr=newmod->fn_ptr;
		return 0;
	}
	__log_error("httpmod", __FILE__, __LINE__, 0, "ERROR: Failed to export function %s", newmod->fn_name);
	return -1;
}

int mod_export_function(char *mod_name, char *fn_name, void *fn_ptr)
{
	int i;

	for (i=0;i<50;i++) {
		if (strlen(http_proc->mod_functions[i].mod_name)>0) continue;
		if (strlen(http_proc->mod_functions[i].fn_name)>0) continue;
		if (http_proc->mod_functions[i].fn_ptr!=NULL) continue;
		snprintf(http_proc->mod_functions[i].mod_name, sizeof(http_proc->mod_functions[i].mod_name)-1, "%s", mod_name);
		snprintf(http_proc->mod_functions[i].fn_name, sizeof(http_proc->mod_functions[i].fn_name)-1, "%s", fn_name);
		http_proc->mod_functions[i].fn_ptr=fn_ptr;
		return 0;
	}
	__log_error("httpmod", __FILE__, __LINE__, 0, "ERROR: Failed to export function %s", fn_name);
	return -1;
}

#ifdef WIN32
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpReserved)
{
	switch (dwFunction) {
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
#else
/*
void _init()
{
	return;
} 
*/
#endif
#endif /* SRVMOD_MAIN */
