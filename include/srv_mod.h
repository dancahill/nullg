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
#pragma comment(lib, "libcmt.lib")
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
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
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

typedef int   (*MAIN_CONFIG_READ)(CONFIG *);
typedef int   (*MAIN_CONFIG_WRITE)(CONFIG *);
typedef	char *(*MAIN_DECODE_BASE64)(char *, int, char *);
typedef	char *(*MAIN_DOMAIN_GETNAME)(char *, int, int);
typedef	int   (*MAIN_DOMAIN_GETID)(char *);
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
typedef	int   (*MAIN_TCP_BIND)(char *, unsigned short);
typedef	int   (*MAIN_TCP_ACCEPT)(int, struct sockaddr *);
typedef	int   (*MAIN_TCP_FGETS)(char *, int, TCP_SOCKET *);
typedef	int   (*MAIN_TCP_FPRINTF)(TCP_SOCKET *, const char *, ...);
typedef	int   (*MAIN_TCP_RECV)(TCP_SOCKET *, char *, int, int);
typedef	int   (*MAIN_TCP_SEND)(TCP_SOCKET *, const char *, int, int);
typedef	int   (*MAIN_TCP_CLOSE)(TCP_SOCKET *, short int);
typedef	time_t(*MAIN_TIME_SQL2UNIX)(char *);
typedef	char *(*MAIN_TIME_UNIX2SQL)(char *, int, time_t);
typedef	char *(*MAIN_STRNCATF)(char *, int, const char *, ...);
typedef	void  (*MAIN_STRIPRN)(char *);
typedef	void  (*MAIN_FIXSLASHES)(char *);
typedef	char *(*MAIN_STR2SQL)(char *, int, char *);
typedef	char *(*MAIN_P_STRCASESTR)(char *, char *);
#ifdef HAVE_LIBSSL
typedef	int   (*MAIN_SSL_ACCEPT)(TCP_SOCKET *);
typedef	int   (*MAIN_SSL_CLOSE)(TCP_SOCKET *);
#endif
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
#define EXTERN extern
#else
#define EXTERN
#endif

EXTERN MAIN_CONFIG_READ			config_read;
EXTERN MAIN_CONFIG_WRITE		config_write;
EXTERN MAIN_DECODE_BASE64		decode_base64;
EXTERN MAIN_DOMAIN_GETNAME		domain_getname;
EXTERN MAIN_DOMAIN_GETID		domain_getid;
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
EXTERN MAIN_TCP_BIND			tcp_bind;
EXTERN MAIN_TCP_ACCEPT			tcp_accept;
EXTERN MAIN_TCP_FGETS			tcp_fgets;
EXTERN MAIN_TCP_FPRINTF			tcp_fprintf;
EXTERN MAIN_TCP_RECV			tcp_recv;
EXTERN MAIN_TCP_SEND			tcp_send;
EXTERN MAIN_TCP_CLOSE			tcp_close;
EXTERN MAIN_TIME_SQL2UNIX		time_sql2unix;
EXTERN MAIN_TIME_UNIX2SQL		time_unix2sql;
EXTERN MAIN_STRNCATF			strncatf;
EXTERN MAIN_STRIPRN			striprn;
EXTERN MAIN_FIXSLASHES			fixslashes;
EXTERN MAIN_STR2SQL			str2sql;
EXTERN MAIN_P_STRCASESTR		p_strcasestr;
#ifdef HAVE_LIBSSL
EXTERN MAIN_SSL_ACCEPT			ssl_accept;
EXTERN MAIN_SSL_CLOSE			ssl_close;
#endif
#ifdef WIN32
EXTERN MAIN_GETTIMEOFDAY		gettimeofday;
EXTERN MAIN_OPENDIR			opendir;
EXTERN MAIN_READDIR			readdir;
EXTERN MAIN_CLOSEDIR			closedir;
#endif

EXTERN CONFIG   *config;
EXTERN FUNCTION *functions;
EXTERN _PROC    *proc;

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
	__log_error("srvmod", __FILE__, __LINE__, 0, "ERROR: Failed to find function %s", fn_name);
	return -1;
}

int mod_import()
{
	log_error=NULL;
	if (_get_func((void *)&log_error,		"log_error"		)!=0) return -1;
	if (_get_func((void *)&log_access,		"log_access"		)!=0) return -1;
	if (_get_func((void *)&config_read,		"config_read"		)!=0) return -1;
	if (_get_func((void *)&config_write,		"config_write"		)!=0) return -1;
	if (_get_func((void *)&decode_base64,		"decode_base64"		)!=0) return -1;
	if (_get_func((void *)&domain_getname,		"domain_getname"	)!=0) return -1;
	if (_get_func((void *)&domain_getid,		"domain_getid"		)!=0) return -1;
	if (_get_func((void *)&md5_init,		"md5_init"		)!=0) return -1;
	if (_get_func((void *)&md5_update,		"md5_update"		)!=0) return -1;
	if (_get_func((void *)&md5_final,		"md5_final"		)!=0) return -1;
	if (_get_func((void *)&md5_crypt,		"md5_crypt"		)!=0) return -1;
	if (_get_func((void *)&sql_freeresult,		"sql_freeresult"	)!=0) return -1;
	if (_get_func((void *)&sql_update,		"sql_update"		)!=0) return -1;
	if (_get_func((void *)&sql_updatef,		"sql_updatef"		)!=0) return -1;
	if (_get_func((void *)&sql_query,		"sql_query"		)!=0) return -1;
	if (_get_func((void *)&sql_queryf,		"sql_queryf"		)!=0) return -1;
	if (_get_func((void *)&sql_getname,		"sql_getname"		)!=0) return -1;
	if (_get_func((void *)&sql_getvalue,		"sql_getvalue"		)!=0) return -1;
	if (_get_func((void *)&sql_getvaluebyname,	"sql_getvaluebyname"	)!=0) return -1;
	if (_get_func((void *)&sql_numfields,		"sql_numfields"		)!=0) return -1;
	if (_get_func((void *)&sql_numtuples,		"sql_numtuples"		)!=0) return -1;
	if (_get_func((void *)&tcp_bind,		"tcp_bind"		)!=0) return -1;
	if (_get_func((void *)&tcp_accept,		"tcp_accept"		)!=0) return -1;
	if (_get_func((void *)&tcp_fgets,		"tcp_fgets"		)!=0) return -1;
	if (_get_func((void *)&tcp_fprintf,		"tcp_fprintf"		)!=0) return -1;
	if (_get_func((void *)&tcp_recv,		"tcp_recv"		)!=0) return -1;
	if (_get_func((void *)&tcp_send,		"tcp_send"		)!=0) return -1;
	if (_get_func((void *)&tcp_close,		"tcp_close"		)!=0) return -1;
	if (_get_func((void *)&time_sql2unix,		"time_sql2unix"		)!=0) return -1;
	if (_get_func((void *)&time_unix2sql,		"time_unix2sql"		)!=0) return -1;
	if (_get_func((void *)&strncatf,		"strncatf"		)!=0) return -1;
	if (_get_func((void *)&striprn,			"striprn"		)!=0) return -1;
	if (_get_func((void *)&fixslashes,		"fixslashes"		)!=0) return -1;
	if (_get_func((void *)&str2sql,			"str2sql"		)!=0) return -1;
	if (_get_func((void *)&p_strcasestr,		"p_strcasestr"		)!=0) return -1;
#ifdef HAVE_LIBSSL
	if (_get_func((void *)&ssl_accept,		"ssl_accept"		)!=0) return -1;
	if (_get_func((void *)&ssl_close,		"ssl_close"		)!=0) return -1;
#endif
#ifdef WIN32
	if (_get_func((void *)&gettimeofday,		"gettimeofday"		)!=0) return -1;
	if (_get_func((void *)&opendir,			"opendir"		)!=0) return -1;
	if (_get_func((void *)&readdir,			"readdir"		)!=0) return -1;
	if (_get_func((void *)&closedir,		"closedir"		)!=0) return -1;
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	return 0;
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
