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
/* win32.c functions */
#ifdef WIN32
int	winsystem(const char *format, ...);
void	TrayIcon(int newstatus);
void	TrayBlink();
BOOL	CALLBACK NullDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
typedef unsigned (__cdecl *pthread_handler)(void *);
int pthread_attr_init(pthread_attr_t *connect_att);
int pthread_attr_setstacksize(pthread_attr_t *connect_att, DWORD stack);
int pthread_attr_setprio(pthread_attr_t *connect_att, int priority);
int pthread_attr_destroy(pthread_attr_t *connect_att);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
int pthread_kill(pthread_t handle, int sig);
void pthread_exit(unsigned A);
#endif

#ifdef WIN32
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

/* auth.c functions */
int auth_login(CONN *sid, char *username, char *password, int mbox);
/* config.c functions */
int     config_read(CONFIG *config);
/* format.c */
char   *getbuffer(CONN *sid);
void    fixslashes(char *pOriginal);
void    striprn(char *string);
void    swapchar(char *string, char oldchar, char newchar);
char   *p_strcasestr(char *src, char *query);
char   *strncatf(char *dest, int max, const char *format, ...);
char   *str2sql(CONN *sid, char *inbuffer);
char   *str2sqlbuf(CONN *sid, char *instring, char *outstring, int outsize);
char   *time_unix2sql(CONN *sid, time_t unixdate);
char   *DecodeBase64string(CONN *sid, char *src);
/* io.c functions */
int     prints(CONN *sid, const char *format, ...);
int     sgets(CONN *sid, char *buffer, int max, int fd);
/* md5.c functions */
void    md5_init(struct MD5Context *context);
void    md5_update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void    md5_final(unsigned char digest[16], struct MD5Context *context);
void    md5_transform(uint32 buf[4], uint32 const in[16]);
char   *md5_crypt(CONN *sid, char *pw, char *salt);	/* FreBSD password hashing */
/* sanity.c functions */
int     sanity_dbcheck(void);
/* logging.c functions */
void    logaccess(CONN *sid, int loglevel, const char *format, ...);
void    logerror(CONN *sid, char *srcfile, int line, const char *format, ...);
void    logdebug(CONN *sid, char *srcfile, int line, const char *format, ...);
#ifdef DEBUG
void    logdata(const char *format, ...);
#endif
/* server.c functions */
int     closeconnect(CONN *sid, int exitflag);
void    server_restart(void);
void    server_shutdown(void);
void    dorequest(CONN *sid);
void    init(void);
#ifdef WIN32
unsigned _stdcall conn_reaper(void *x);
unsigned _stdcall accept_loop(void *x);
#else
void   *conn_reaper(void *x);
void   *accept_loop(void *x);
#endif
/* smtp.c functions */
void smtp_main(CONN *sid);
/* smtpclient.c functions */
#ifdef WIN32
unsigned _stdcall smtp_spool(void *x);
#else
void   *smtp_spool(void *x);
#endif
/* sql.c functions */
void    sql_disconnect(CONN *sid);
void    sql_unsafedisconnect(CONN *sid);
void    sql_freeresult(int sqr);
int     sql_update(CONN *sid, char *sqlquery);
int     sql_updatef(CONN *sid, char *format, ...);
int     sql_query(CONN *sid, char *query);
int     sql_queryf(CONN *sid, char *format, ...);
char   *sql_getname(int sqr, int field);
char   *sql_getvalue(int sqr, int tuple, int field);
char   *sql_getvaluebyname(int sqr, int tuple, char *fieldname);
int     sql_numfields(int sqr);
int     sql_numtuples(int sqr);
