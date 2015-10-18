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
#include "nullsd/config-nt.h"
#define _USE_32BIT_TIME_T
#else
#include "nullsd/config.h"
#endif
/* #includes */
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _MT 1
	#define PATH_MAX 512 /* from limits.h, but _POSIX_ breaks stuff */
//	#pragma comment(lib, "libcmt.lib")
	#pragma comment(lib, "ws2_32.lib")
	#include <winsock2.h>
	#include <windows.h>
	#include <process.h>
	#include <shellapi.h>
	#include <signal.h>
	#include <windowsx.h>
	#include <basetsd.h>
	#include <io.h>
	#include <direct.h>
	#include <limits.h>
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <dirent.h>
	#include <netdb.h>
#ifdef HAVE_PATHS_H
	#include <paths.h>
#endif
#ifdef HAVE_PTHREAD_MIT_PTHREAD_H
	#include <pthread/mit/pthread.h>
#else
	#include <pthread.h>
#endif
	#include <signal.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#define closesocket close
#endif
#include "nullsd/ssl.h"

#include "nullsd/defines.h"
#include "nsp/nsp.h"
#include "nsd.h"
#include "nullsd/typedefs.h"

#ifdef MAIN_GLOBALS
	LOCKS   Lock;
	_PROC   proc;
	pthread_t CronThread;
	pthread_t ListenThread;
#else
	extern LOCKS   Lock;
	extern _PROC   proc;
	extern pthread_t CronThread;
	extern pthread_t ListenThread;
#endif


#ifdef WIN32
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

/* config.c functions */
int     conf_read(nsp_state *N);
/* domains.c functions */
char   *domain_getname(char *outstring, int outlen, int domainid);
int     domain_getid(char *domainname);
/* format.c */
char   *decode_base64(char *dest, int szdest, char *src);
int     hex2int(char *pChars);
char   *p_strcasestr(char *src, char *query);
char   *strncatf(char *dest, int max, const char *format, ...);
char   *str2sql(char *outstring, int outlen, char *instring);
time_t  time_sql2unix(char *sqldate);
char   *time_unix2sql(char *outstring, int outlen, time_t unixdate);
/* language.c functions */
char   *language_gets(nsp_state *N, char *lang, char *sect, char *label);
/* modctl.c functions */
#ifdef WIN32
unsigned _stdcall cronloop(void *x);
#else
void *cronloop(void *x);
#endif
int     modules_init(nsp_state *N);
int     modules_exec(void);
int     modules_exit(void);
int     modules_cron(void);
int     module_load(char *modname);
/* sanity.c functions */
int     sanity_checkdb(void);
int     sanity_checkdirs(void);
/* server.c functions */
/* void    server_restart(void); */
void    server_shutdown(void);
void    init(nsp_state *N);
int     addlistener(char *modname, TCP_SOCKET *sock, void *fn_getconn, void *fn_doreq, int use_ssl);
int     startlisteners(void);
/* sql.c functions */
void    sql_disconnect(nsp_state *N);
void    sql_freeresult(nsp_state *N, obj_t **qobj);
int     sql_update(nsp_state *N, char *sqlquery);
int     sql_query(nsp_state *N, obj_t **qobj, char *query);
int     sql_updatef(nsp_state *N, char *format, ...);
int     sql_queryf(nsp_state *N, obj_t **qobj, char *format, ...);
int     sql_getsequence(nsp_state *N, char *table);
/* ssl.c functions */
int     ssl_init();
int     ssl_accept(TCP_SOCKET *sock);
int     ssl_connect(TCP_SOCKET *sock);
int     ssl_read(SSL *ssl, void *buf, int len);
int     ssl_write(SSL *ssl, const void *buf, int len);
int     ssl_close(TCP_SOCKET *sock);
int     ssl_shutdown();
/* tcp.c functions */
int     tcp_bind(char *ifname, unsigned short port);
int     tcp_accept(int listensock, TCP_SOCKET *sock);
int     tcp_connect(TCP_SOCKET *socket, char *host, unsigned short port, short int use_ssl);
int     tcp_fgets(char *buffer, int max, TCP_SOCKET *socket);
int     tcp_fprintf(TCP_SOCKET *socket, const char *format, ...);
int     tcp_recv(TCP_SOCKET *socket, char *buffer, int len, int flags);
int     tcp_send(TCP_SOCKET *socket, const char *buffer, int len, int flags);
int     tcp_close(TCP_SOCKET *socket, short int owner_killed);
