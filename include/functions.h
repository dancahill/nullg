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
/* auth.c functions */
char   *auth_setpass(CONN *sid, char *rpassword);
int     auth_getcookie(CONN *sid);
int     auth_setcookie(CONN *sid);
void    auth_logout(CONN *sid);
int     auth_priv(CONN *sid, char *service);
/* config.c functions */
int     config_read(CONFIG *config);
int     config_write(CONFIG *config);
/* format.c */
char   *getbuffer(CONN *sid);
void    decodeurl(unsigned char *pEncoded);
void    fixslashes(char *pOriginal);
int     hex2int(char *pChars);
void    striprn(char *string);
void    swapchar(char *string, char oldchar, char newchar);
char   *strcasestr(char *src, char *query);
char   *strncatf(char *dest, int max, const char *format, ...);
int     printhex(CONN *sid, const char *format, ...);
int     printht(CONN *sid, const char *format, ...);
void    printline(CONN *sid, short int reply, char *msgtext);
void    printline2(CONN *sid, int dowrap, char *msgtext);
char   *str2html(CONN *sid, char *instring);
char   *str2sql(CONN *sid, char *inbuffer);
char   *str2sqlbuf(CONN *sid, char *instring, char *outstring, int outsize);
void    htselect_timezone(CONN *sid, short int selected);
time_t  time_sql2unix(char *sqldate);
char   *time_sql2datetext(CONN *sid, char *sqldate);
char   *time_sql2timetext(CONN *sid, char *sqldate);
char   *time_sql2lotimetext(CONN *sid, char *sqldate);
char   *time_unix2sql(CONN *sid, time_t unixdate);
char   *time_unix2sqldate(CONN *sid, time_t unixdate);
char   *time_unix2sqltime(CONN *sid, time_t unixdate);
char   *time_unix2text(CONN *sid, time_t unixdate);
char   *time_unix2datetext(CONN *sid, time_t unixdate);
char   *time_unix2timetext(CONN *sid, time_t unixdate);
char   *time_unix2lotimetext(CONN *sid, time_t unixdate);
int     time_tzoffset(CONN *sid, time_t unixdate);
int     time_tzoffset2(CONN *sid, time_t unixdate, int userid);
time_t  time_wmgetdate(char *src);
char   *DecodeBase64string(CONN *sid, char *src);
/* html.c functions */
void    htpage_login(CONN *sid);
void    htpage_logout(CONN *sid);
void    htpage_stats(CONN *sid);
void    htpage_frameset(CONN *sid);
void    htpage_sidemenu(CONN *sid);
void    htpage_topmenu(CONN *sid, int menu);
void    htpage_reload(CONN *sid);
void    htpage_motd(CONN *sid);
void    htpage_header(CONN *sid, char *title);
void    htpage_footer(CONN *sid);
void    htselect_callfilter(CONN *sid, int selected, char *baseuri);
void    htselect_eventfilter(CONN *sid, int userid, int groupid, char *baseuri);
void    htselect_logfilter(CONN *sid, int selected, char *baseuri);
void    htselect_notefilter(CONN *sid, int selected, char *baseuri);
void    htselect_mailjump(CONN *sid, int selected);
void    htselect_mailmbox(CONN *sid, char *selected);
void    htselect_day(CONN *sid, char *selected);
void    htselect_hour(CONN *sid, int selected);
void    htselect_month(CONN *sid, char *selected);
void    htselect_qhours(CONN *sid, int selected);
void    htselect_qminutes(CONN *sid, int selected);
void    htselect_minutes(CONN *sid, int selected);
void    htselect_time(CONN *sid, time_t unixtime);
void    htselect_year(CONN *sid, int startyear, char *selected);
void    htselect_bookmarkfolder(CONN *sid, int selected);
void    htselect_callaction(CONN *sid, int selected);
void    htselect_contact(CONN *sid, int selected);
void    htselect_eventclosingstatus(CONN *sid, int selected);
void    htselect_eventstatus(CONN *sid, int selected);
void    htselect_eventtype(CONN *sid, int selected);
void    htselect_forumgroup(CONN *sid, int selected);
void    htselect_group(CONN *sid, int selected);
void    htselect_layout(CONN *sid, int selected);
void    htselect_mailaccount(CONN *sid, int selected);
void    htselect_number(CONN *sid, int selected, int start, int end);
void    htselect_priority(CONN *sid, int selected);
void    htselect_product(CONN *sid, int selected);
void    htselect_reminder(CONN *sid, int selected);
void    htselect_user(CONN *sid, int selected);
void    htselect_zone(CONN *sid, int selected);
char   *htview_callaction(CONN *sid, int selected);
char   *htview_contact(CONN *sid, int selected);
char   *htview_eventclosingstatus(CONN *sid, int selected);
char   *htview_eventstatus(CONN *sid, int selected);
char   *htview_eventtype(CONN *sid, int selected);
char   *htview_forumgroup(CONN *sid, int selected);
char   *htview_holiday(char *date);
char   *htview_product(CONN *sid, int selected);
char   *htview_reminder(CONN *sid, int selected);
char   *htview_user(CONN *sid, int selected);
/* http.c functions */
char   *get_mime_type(char *name);
void    ReadPOSTData(CONN *sid);
char   *getgetenv(CONN *sid, char *query);
char   *getmimeenv(CONN *sid, char *query, unsigned int *buffersize);
char   *getpostenv(CONN *sid, char *query);
char   *getxmlenv(CONN *sid, char *query);
char   *getxmlparam(CONN *sid, int param, char *reqtype);
char   *getxmlstruct(CONN *sid, char *reqmember, char *reqtype);
void    read_cgienv(CONN *sid);
int     read_header(CONN *sid);
void    send_error(CONN *sid, int status, char* title, char* text);
void    send_fileheader(CONN *sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
void    send_header(CONN *sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
/* io.c functions */
void    flushheader(CONN *sid);
void    flushbuffer(CONN *sid);
int     prints(CONN *sid, const char *format, ...);
int     sgets(CONN *sid, char *buffer, int max, int fd);
int     filesend(CONN *sid, unsigned char *file);
int     tcp_send(int s, const char *buffer, int len, int flags);
/* md5.c functions */
void    md5_init(struct MD5Context *context);
void    md5_update(struct MD5Context *context, unsigned char const *buf, unsigned len);
void    md5_final(unsigned char digest[16], struct MD5Context *context);
void    md5_transform(uint32 buf[4], uint32 const in[16]);
char   *md5_crypt(CONN *sid, char *pw, char *salt);	/* FreBSD password hashing */
/* modctl.c functions */
int     module_exists(CONN *sid, char *mod_name);
void   *module_call(CONN *sid, char *fn_name);
int     module_menucall(CONN *sid);
int     modules_init(void);
/* sanity.c functions */
int     sanity_dbcheck(void);
/* logging.c functions */
int     db_log_activity(CONN *sid, int loglevel, char *category, int indexid, char *action, const char *format, ...);
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
void    cgiinit(void);
void    init(void);
#ifdef WIN32
unsigned _stdcall conn_reaper(void *x);
unsigned _stdcall accept_loop(void *x);
#else
void   *conn_reaper(void *x);
void   *accept_loop(void *x);
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
