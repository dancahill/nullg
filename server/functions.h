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
/* win32.c functions */
#ifdef WIN32
unsigned sleep(unsigned seconds);
DIR	*opendir(const char *);
void	closedir(DIR *);
#define	rewinddir(dirp)	seekdir(dirp, 0L)
void	seekdir(DIR *, long);
long	telldir(DIR *);
struct	direct *readdir(DIR *);
int	gettimeofday(struct timeval *tv, struct timezone *tz); 

#define dlopen(x, y) LoadLibrary(x)
#define dlsym(x, y) GetProcAddress(x, y)
#define dlclose(x) FreeLibrary(x)
/* registry and service control functions */
int get_reg_entries(void);
/* The following is to emulate the posix thread interface */
#define pthread_mutex_init(A,B)  InitializeCriticalSection(A)
#define pthread_mutex_lock(A)    (EnterCriticalSection(A),0)
#define pthread_mutex_unlock(A)  LeaveCriticalSection(A)
#define pthread_mutex_destroy(A) DeleteCriticalSection(A)
#define pthread_handler_decl(A,B) unsigned __cdecl A(void *B)
#define pthread_self() GetCurrentThreadId()
#define pthread_exit(A) _endthread()
typedef unsigned (__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
int pthread_kill(pthread_t handle, int sig);
#endif

#ifdef WIN32
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

/* config.c functions */
int     config_read(char *section, void *callback);
//int     config_read(GLOBAL_CONFIG *config);
int     config_write(GLOBAL_CONFIG *config);
int     conf_read(void);
/* dns.c functions */
char   *dns_getmxbyname(char *dest, int destlen, char *domain);
/* domains.c functions */
char   *domain_getname(char *outstring, int outlen, int domainid);
int     domain_getid(char *domainname);
/* format.c */
char   *decode_base64(char *dest, int szdest, char *src);
//char   *getbuffer(CONN *sid);
void    fixslashes(char *pOriginal);
int     hex2int(char *pChars);
void    striprn(char *string);
void    swapchar(char *string, char oldchar, char newchar);
char   *p_strcasestr(char *src, char *query);
char   *strncatf(char *dest, int max, const char *format, ...);
char   *str2sql(char *outstring, int outlen, char *instring);
time_t  time_sql2unix(char *sqldate);
char   *time_unix2sql(char *outstring, int outlen, time_t unixdate);
/* io.c functions */
int     tcp_bind(char *ifname, unsigned short port);
int     tcp_accept(int listensock, struct sockaddr *addr);
int     tcp_fgets(char *buffer, int max, TCP_SOCKET *socket);
int     tcp_fprintf(TCP_SOCKET *socket, const char *format, ...);
int     tcp_recv(TCP_SOCKET *socket, char *buffer, int len, int flags);
int     tcp_send(TCP_SOCKET *socket, const char *buffer, int len, int flags);
//int     tcp_close(TCP_SOCKET *socket);
int     tcp_close(TCP_SOCKET *socket, short int owner_killed);
/* modctl.c functions */
#ifdef WIN32
unsigned _stdcall cronloop(void *x);
#else
void *cronloop(void *x);
#endif
int     modules_init(void);
int     modules_exec(void);
int     modules_exit(void);
int     modules_cron(void);
int     module_load(char *modname);
/* sanity.c functions */
int     sanity_checkdb(void);
int     sanity_checkdirs(void);
/* log.c functions */
void    log_access(char *logsrc, const char *format, ...);
void    log_error(char *logsrc, char *srcfile, int line, int loglevel, const char *format, ...);
/* md5.c functions */
void md5_init(struct MD5Context *ctx);
void md5_update(struct MD5Context *ctx, unsigned char const *buf, unsigned len);
void md5_final(unsigned char digest[16], struct MD5Context *ctx);
char *md5_crypt(char *passwd, char *pw, char *salt);
/* server.c functions */
//int     closeconnect(CONN *sid, int exitflag);
void    server_restart(void);
void    server_shutdown(void);
//void    dorequest(CONN *sid);
//void    cgiinit(void);
void    init(void);
/*
#ifdef WIN32
unsigned _stdcall conn_reaper(void *x);
unsigned _stdcall accept_loop(void *x);
#ifdef HAVE_SSL
unsigned _stdcall accept_loop_ssl(void *x);
#endif
#else
void   *conn_reaper(void *x);
void   *accept_loop(void *x);
#ifdef HAVE_SSL
void *accept_loop_ssl(void *x);
#endif
#endif
*/
/* sql.c functions */
void    sql_disconnect(void);
void    sql_unsafedisconnect(void);
void    sql_freeresult(int sqr);
int     sql_update(char *sqlquery);
int     sql_updatef(char *format, ...);
int     sql_query(char *query);
int     sql_queryf(char *format, ...);
char   *sql_getname(int sqr, int field);
char   *sql_getvalue(int sqr, int tuple, int field);
char   *sql_getvaluebyname(int sqr, int tuple, char *fieldname);
int     sql_numfields(int sqr);
int     sql_numtuples(int sqr);
/* ssl.c functions */
#ifdef HAVE_SSL
int     ssl_init();
int     ssl_accept(TCP_SOCKET *sock);
int     ssl_read(SSL *ssl, void *buf, int len);
int     ssl_write(SSL *ssl, const void *buf, int len);
int     ssl_close(TCP_SOCKET *sock);
int     ssl_shutdown();
#endif
/* sys.c functions */
int sys_system(const char *format, ...);
