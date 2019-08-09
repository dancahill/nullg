/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#include "nullg/config-nt.h"
#define _USE_32BIT_TIME_T
#define WIN32_LEAN_AND_MEAN
#define _MT 1
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <process.h>
#include <direct.h>
#include <sys/stat.h>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
#include "nullg/config.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifdef HAVE_PTHREAD_MIT_PTHREAD_H
#include <pthread/mit/pthread.h>
#else
#include <pthread.h>
#endif
#endif

#include "nullg/ssl.h"
#include "nullg/defines.h"
#define NSP_NOFUNCTIONS 1
#include "nsp/nsp.h"
#undef NSP_NOFUNCTIONS
#define NULLG_NOFUNCTIONS 1
#include "nullg.h"
#undef NULLG_NOFUNCTIONS
#include "nullg/typedefs.h"

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

typedef int        (*MAIN_CONFIG_READ)(nsp_state *, char *, void *);
typedef int        (*MAIN_CONFIG_WRITE)(nsp_state *);
typedef	char      *(*MAIN_DECODE_BASE64)(char *, int, char *);
typedef int        (*MAIN_DIR_LIST)(nsp_state *, obj_t **, const char *);
typedef	char      *(*MAIN_DOMAIN_GETNAME)(char *, int, int);
typedef	int        (*MAIN_DOMAIN_GETID)(char *);
typedef char      *(*MAIN_LANGUAGE_GETS)(nsp_state *, char *, char *, char *);
typedef	void      *(*MAIN_LIB_OPEN)(const char *);
typedef	void      *(*MAIN_LIB_SYM)(void *, const char *);
typedef	char      *(*MAIN_LIB_ERROR)(void);
typedef	int        (*MAIN_LIB_CLOSE)(void *);
typedef	void       (*MAIN_LOG_ACCESS)(nsp_state *, char *, const char *, ...);
typedef	void       (*MAIN_LOG_ERROR)(nsp_state *, char *, char *, int, int, const char *, ...);
typedef	void       (*MAIN_MD5_INIT)(struct MD5Context *);
typedef	void       (*MAIN_MD5_UPDATE)(struct MD5Context *, unsigned char const *, unsigned);
typedef	void       (*MAIN_MD5_FINAL)(unsigned char*, struct MD5Context *);
typedef	char      *(*MAIN_MD5_CRYPT)(char *, char *, char *);

typedef nsp_state *(*MAIN_NSP_NEWSTATE)  (void);
typedef nsp_state *(*MAIN_NSP_ENDSTATE)  (nsp_state *);
typedef obj_t     *(*MAIN_NSP_EXEC)      (nsp_state *, const char *);
typedef int        (*MAIN_NSP_EXECFILE)  (nsp_state *, char *);

typedef void       (*MAIN_NSP_SETVALTYPE)(nsp_state *, obj_t *, unsigned short);
typedef void       (*MAIN_NSP_LINKVAL)   (nsp_state *, obj_t *, obj_t *);
typedef void       (*MAIN_NSP_UNLINKVAL) (nsp_state *, obj_t *);
typedef void       (*MAIN_NSP_FREETABLE) (nsp_state *, obj_t *);
typedef obj_t     *(*MAIN_NSP_GETOBJ)    (nsp_state *, obj_t *, char *);
typedef obj_t     *(*MAIN_NSP_GETIOBJ)   (nsp_state *, obj_t *, unsigned long);
typedef obj_t     *(*MAIN_NSP_SETOBJ)    (nsp_state *, obj_t *, char *, unsigned short, NSP_CFUNC, num_t, char *, long);
typedef void       (*MAIN_NSP_STRCAT)    (nsp_state *, obj_t *, char *, long);
typedef void       (*MAIN_NSP_STRMUL)    (nsp_state *, obj_t *, unsigned long);
typedef short      (*MAIN_NSP_TOBOOL)    (nsp_state *, obj_t *);
typedef num_t      (*MAIN_NSP_TONUM)     (nsp_state *, obj_t *);
typedef char      *(*MAIN_NSP_TOSTR)     (nsp_state *, obj_t *);

typedef obj_t     *(*MAIN_NSP_EVAL)      (nsp_state *, char *);
typedef obj_t     *(*MAIN_NSP_EVALF)     (nsp_state *, const char *, ...);

typedef	void       (*MAIN_SQL_FREERESULT)(nsp_state *, obj_t **);
typedef	int        (*MAIN_SQL_UPDATE)(nsp_state *, obj_t **, char *);
typedef	int        (*MAIN_SQL_UPDATEF)(nsp_state *, obj_t **, const char *, ...);
typedef	int        (*MAIN_SQL_QUERY)(nsp_state *, obj_t **, char *);
typedef	int        (*MAIN_SQL_QUERYF)(nsp_state *, obj_t **, char *, ...);
typedef	int        (*MAIN_SQL_GETSEQUENCE)(nsp_state *, char *);
typedef	char      *(*MAIN_SQL_GETNAME)(nsp_state *, obj_t **, int);
typedef	char      *(*MAIN_SQL_GETVALUE)(nsp_state *, obj_t **, int, int);
typedef	char      *(*MAIN_SQL_GETVALUEBYNAME)(nsp_state *, obj_t **, int, char *);
typedef	int        (*MAIN_SQL_NUMFIELDS)(nsp_state *, obj_t **);
typedef	int        (*MAIN_SQL_NUMTUPLES)(nsp_state *, obj_t **);
typedef	int        (*MAIN_SYS_SYSTEM)(const char *, ...);
typedef	int        (*MAIN_SSL_CONNECT)(TCP_SOCKET *);
typedef	int        (*MAIN_TCP_BIND)(char *, unsigned short);
typedef int        (*MAIN_TCP_ACCEPT)(int, TCP_SOCKET *);
typedef	int        (*MAIN_TCP_CONNECT)(TCP_SOCKET *, char *, unsigned short, short int);
typedef	int        (*MAIN_TCP_FGETS)(char *, int, TCP_SOCKET *);
typedef	int        (*MAIN_TCP_FPRINTF)(TCP_SOCKET *, const char *, ...);
typedef	int        (*MAIN_TCP_RECV)(TCP_SOCKET *, char *, int, int);
typedef	int        (*MAIN_TCP_SEND)(TCP_SOCKET *, const char *, int, int);
typedef	int        (*MAIN_TCP_CLOSE)(TCP_SOCKET *, short int);
typedef	time_t     (*MAIN_TIME_SQL2UNIX)(char *);
typedef	char      *(*MAIN_TIME_UNIX2SQL)(char *, int, time_t);
typedef	char      *(*MAIN_STRNCATF)(char *, int, const char *, ...);
typedef	void       (*MAIN_STRIPRN)(char *);
typedef	void       (*MAIN_FIXSLASHES)(char *);
typedef	char      *(*MAIN_STR2SQL)(char *, int, char *);
typedef	char      *(*MAIN_P_STRCASESTR)(char *, char *);
typedef	int        (*MAIN_SSL_ACCEPT)(TCP_SOCKET *);
typedef	int        (*MAIN_SSL_CLOSE)(TCP_SOCKET *);
typedef void       (*MAIN_ADDLISTENER)(char *, TCP_SOCKET *, void *, void *, int);
#ifdef WIN32
typedef	int        (*MAIN_GETTIMEOFDAY)(struct timeval *, struct timezone *);
#endif

EXTERN MAIN_CONFIG_READ			config_read;
EXTERN MAIN_CONFIG_WRITE		config_write;
EXTERN MAIN_DECODE_BASE64		decode_base64;
EXTERN MAIN_DIR_LIST			dir_list;
EXTERN MAIN_DOMAIN_GETNAME		domain_getname;
EXTERN MAIN_DOMAIN_GETID		domain_getid;
EXTERN MAIN_LANGUAGE_GETS		language_gets;
EXTERN MAIN_LIB_OPEN			lib_open;
EXTERN MAIN_LIB_SYM			lib_sym;
EXTERN MAIN_LIB_ERROR			lib_error;
EXTERN MAIN_LIB_CLOSE			lib_close;
EXTERN MAIN_LOG_ACCESS			log_access;
EXTERN MAIN_LOG_ERROR			log_error;
EXTERN MAIN_MD5_INIT			md5_init;
EXTERN MAIN_MD5_UPDATE			md5_update;
EXTERN MAIN_MD5_FINAL			md5_final;
EXTERN MAIN_MD5_CRYPT			md5_crypt;

EXTERN MAIN_NSP_NEWSTATE		nsp_newstate;
EXTERN MAIN_NSP_ENDSTATE		nsp_endstate;
EXTERN MAIN_NSP_EXEC			nsp_exec;
EXTERN MAIN_NSP_EXECFILE		nsp_execfile;
EXTERN MAIN_NSP_LINKVAL			nsp_linkval;
EXTERN MAIN_NSP_UNLINKVAL		nsp_unlinkval;
EXTERN MAIN_NSP_FREETABLE		nsp_freetable;
EXTERN MAIN_NSP_GETOBJ			nsp_getobj;
EXTERN MAIN_NSP_GETIOBJ			nsp_getiobj;
EXTERN MAIN_NSP_SETOBJ			nsp_setobj;
EXTERN MAIN_NSP_STRCAT			nsp_strcat;
EXTERN MAIN_NSP_TONUM			nsp_tonum;
EXTERN MAIN_NSP_TOSTR			nsp_tostr;
EXTERN MAIN_NSP_EVAL			nsp_eval;
EXTERN MAIN_NSP_EVALF			nsp_evalf;

EXTERN MAIN_SQL_FREERESULT		sql_freeresult;
EXTERN MAIN_SQL_UPDATE			sql_update;
EXTERN MAIN_SQL_UPDATEF			sql_updatef;
EXTERN MAIN_SQL_QUERY			sql_query;
EXTERN MAIN_SQL_QUERYF			sql_queryf;
EXTERN MAIN_SQL_GETSEQUENCE		sql_getsequence;
EXTERN MAIN_SQL_GETNAME			sql_getname;
EXTERN MAIN_SQL_GETVALUE		sql_getvalue;
EXTERN MAIN_SQL_GETVALUEBYNAME		sql_getvaluebyname;
EXTERN MAIN_SQL_NUMFIELDS		sql_numfields;
EXTERN MAIN_SQL_NUMTUPLES		sql_numtuples;
EXTERN MAIN_SYS_SYSTEM			sys_system;

EXTERN MAIN_SSL_CONNECT			ssl_connect;

EXTERN MAIN_TCP_BIND			tcp_bind;
EXTERN MAIN_TCP_ACCEPT			tcp_accept;
EXTERN MAIN_TCP_CONNECT			tcp_connect;
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
EXTERN MAIN_SSL_ACCEPT			ssl_accept;
EXTERN MAIN_SSL_CLOSE			ssl_close;
EXTERN MAIN_ADDLISTENER			addlistener;
#ifdef WIN32
EXTERN MAIN_GETTIMEOFDAY		gettimeofday;
#endif

EXTERN _PROC *proc;

#ifdef SRVMOD_MAIN
void __log_error(char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...)
{
	char logbuffer[2048];
	char timebuffer[100];
	char file[200];
	va_list ap;
	FILE *fp;
	time_t t=time(NULL);

/*	snprintf(file, sizeof(file)-1, "%s/%s-error.log", config->dir_var_log, logsrc); */
	snprintf(file, sizeof(file)-1, "%s/%s-error.log", "", logsrc);
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

static obj_t *_getobj(nsp_state *N, obj_t *tobj, char *oname)
{
	obj_t *cobj;

	if ((tobj==NULL)||(tobj->val->type!=NT_TABLE)) return NULL;
	for (cobj=tobj->val->d.table.f;cobj;cobj=cobj->next) {
		if (strcmp(cobj->name, oname)==0) return cobj;
	}
	return NULL;
}

static int _get_func(void **fn, char *fn_name)
{
	obj_t *tobj, *cobj;

	tobj=_getobj(proc->N, &proc->N->g, "_exports");
	if (tobj==NULL||tobj->val->type!=NT_TABLE) return -1;
	cobj=_getobj(proc->N, tobj, "core");
	if (cobj==NULL||cobj->val->type!=NT_TABLE) return -1;
	cobj=_getobj(proc->N, cobj, fn_name);
	if ((cobj==NULL)||(cobj->val->type!=NT_CFUNC)) {
		*fn=NULL;
	} else {
		*fn=cobj->val->d.cfunc;
	}
	if (*fn!=NULL) return 0;
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
	if (_get_func((void *)&dir_list,		"dir_list"		)!=0) return -1;
	if (_get_func((void *)&domain_getname,		"domain_getname"	)!=0) return -1;
	if (_get_func((void *)&domain_getid,		"domain_getid"		)!=0) return -1;
	if (_get_func((void *)&language_gets,		"language_gets"		)!=0) return -1;
	if (_get_func((void *)&lib_open,		"lib_open"		)!=0) return -1;
	if (_get_func((void *)&lib_sym,			"lib_sym"		)!=0) return -1;
	if (_get_func((void *)&lib_error,		"lib_error"		)!=0) return -1;
	if (_get_func((void *)&lib_close,		"lib_close"		)!=0) return -1;
	if (_get_func((void *)&md5_init,		"md5_init"		)!=0) return -1;
	if (_get_func((void *)&md5_update,		"md5_update"		)!=0) return -1;
	if (_get_func((void *)&md5_final,		"md5_final"		)!=0) return -1;
	if (_get_func((void *)&md5_crypt,		"md5_crypt"		)!=0) return -1;

	if (_get_func((void *)&nsp_newstate,		"nsp_newstate"		)!=0) return -1;
	if (_get_func((void *)&nsp_endstate,		"nsp_endstate"		)!=0) return -1;
	if (_get_func((void *)&nsp_exec,		"nsp_exec"		)!=0) return -1;
	if (_get_func((void *)&nsp_execfile,		"nsp_execfile"		)!=0) return -1;
	if (_get_func((void *)&nsp_linkval,		"nsp_linkval"		)!=0) return -1;
	if (_get_func((void *)&nsp_unlinkval,		"nsp_unlinkval"		)!=0) return -1;
	if (_get_func((void *)&nsp_freetable,		"nsp_freetable"		)!=0) return -1;
	if (_get_func((void *)&nsp_getobj,		"nsp_getobj"		)!=0) return -1;
	if (_get_func((void *)&nsp_getiobj,		"nsp_getiobj"		)!=0) return -1;
	if (_get_func((void *)&nsp_setobj,		"nsp_setobj"		)!=0) return -1;
	if (_get_func((void *)&nsp_strcat,		"nsp_strcat"		)!=0) return -1;
	if (_get_func((void *)&nsp_tonum,		"nsp_tonum"		)!=0) return -1;
	if (_get_func((void *)&nsp_tostr,		"nsp_tostr"		)!=0) return -1;
	if (_get_func((void *)&nsp_eval,		"nsp_eval"		)!=0) return -1;
	if (_get_func((void *)&nsp_evalf,		"nsp_evalf"		)!=0) return -1;

	if (_get_func((void *)&sql_freeresult,		"sql_freeresult"	)!=0) return -1;
	if (_get_func((void *)&sql_update,		"sql_update"		)!=0) return -1;
	if (_get_func((void *)&sql_updatef,		"sql_updatef"		)!=0) return -1;
	if (_get_func((void *)&sql_query,		"sql_query"		)!=0) return -1;
	if (_get_func((void *)&sql_queryf,		"sql_queryf"		)!=0) return -1;
	if (_get_func((void *)&sql_getsequence,		"sql_getsequence"	)!=0) return -1;
	if (_get_func((void *)&sql_getname,		"sql_getname"		)!=0) return -1;
	if (_get_func((void *)&sql_getvalue,		"sql_getvalue"		)!=0) return -1;
	if (_get_func((void *)&sql_getvaluebyname,	"sql_getvaluebyname"	)!=0) return -1;
	if (_get_func((void *)&sql_numfields,		"sql_numfields"		)!=0) return -1;
	if (_get_func((void *)&sql_numtuples,		"sql_numtuples"		)!=0) return -1;
	if (_get_func((void *)&sys_system,		"sys_system"		)!=0) return -1;
	if (_get_func((void *)&ssl_connect,		"ssl_connect"		) != 0) return -1;
	if (_get_func((void *)&tcp_bind,		"tcp_bind"		)!=0) return -1;
	if (_get_func((void *)&tcp_accept,		"tcp_accept"		)!=0) return -1;
	if (_get_func((void *)&tcp_connect,		"tcp_connect"		)!=0) return -1;
	if (_get_func((void *)&tcp_fgets,		"tcp_fgets"		)!=0) return -1;
	if (_get_func((void *)&tcp_fprintf,		"tcp_fprintf"		)!=0) return -1;
	if (_get_func((void *)&tcp_recv,		"tcp_recv"		)!=0) return -1;
	if (_get_func((void *)&tcp_send,		"tcp_send"		)!=0) return -1;
	if (_get_func((void *)&tcp_close,		"tcp_close"		)!=0) return -1;
	if (_get_func((void *)&time_sql2unix,		"time_sql2unix"		)!=0) return -1;
	if (_get_func((void *)&time_unix2sql,		"time_unix2sql"		)!=0) return -1;
	if (_get_func((void *)&strncatf,		"strncatf"		)!=0) return -1;
	if (_get_func((void *)&striprn,		"striprn"		)!=0) return -1;
	if (_get_func((void *)&fixslashes,		"fixslashes"		)!=0) return -1;
	if (_get_func((void *)&str2sql,		"str2sql"		)!=0) return -1;
	if (_get_func((void *)&p_strcasestr,		"p_strcasestr"		)!=0) return -1;
	if (_get_func((void *)&ssl_accept,		"ssl_accept"		)!=0) return -1;
	if (_get_func((void *)&ssl_close,		"ssl_close"		)!=0) return -1;
	if (_get_func((void *)&addlistener,		"addlistener"		)!=0) return -1;
#ifdef WIN32
	if (_get_func((void *)&gettimeofday,		"gettimeofday"		)!=0) return -1;
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
