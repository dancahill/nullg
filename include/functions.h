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
char *auth_setpass(CONNECTION *sid, char *rpassword);
int   auth_getcookie(CONNECTION *sid);
int   auth_setcookie(CONNECTION *sid);
void  auth_logout(CONNECTION *sid);
int   auth_priv(CONNECTION *sid, int service);
/* cgi.c */
int   cgi_main(CONNECTION *sid);
/* config.c functions */
int   config_read(void);
/* connio.c functions */
void  flushheader(CONNECTION *sid);
void  flushbuffer(CONNECTION *sid);
int   prints(CONNECTION *sid, const char *format, ...);
int   sgets(CONNECTION *sid, char *buffer, int max, int fd);
/* dbio.c */
int   db_read(CONNECTION *sid, short int perm, short int table, int index, void *record);
int   db_write(CONNECTION *sid, short int table, int index, void *record);
int   db_log_activity(CONNECTION *sid, int loglevel, char *category, int indexid, char *action, const char *format, ...);
/* fileio.c functions */
int   dirlist(CONNECTION *sid);
int   fileul(CONNECTION *sid);
int   filesend(CONNECTION *sid, unsigned char *file);
int   filerecv(CONNECTION *sid);
/* format.c */
char   *getbuffer(CONNECTION *sid);
void    decodeurl(unsigned char *pEncoded);
void    fixslashes(char *pOriginal);
int     hex2int(char *pChars);
void    striprn(char *string);
void    swapchar(char *string, char oldchar, char newchar);
char   *strcasestr(char *src, char *query);
char   *strncatf(char *dest, int max, const char *format, ...);
int     printhex(CONNECTION *sid, const char *format, ...);
int     printht(CONNECTION *sid, const char *format, ...);
void    printline(CONNECTION *sid, short int reply, char *msgtext);
void    printline2(CONNECTION *sid, int dowrap, char *msgtext);
char   *str2html(CONNECTION *sid, char *instring);
char   *str2sql(CONNECTION *sid, char *inbuffer);
char   *str2sqlbuf(CONNECTION *sid, char *instring, char *outstring, int outsize);
void    htselect_timezone(CONNECTION *sid, short int selected);
time_t  time_sql2unix(char *sqldate);
char   *time_sql2datetext(CONNECTION *sid, char *sqldate);
char   *time_sql2timetext(CONNECTION *sid, char *sqldate);
char   *time_sql2lotimetext(CONNECTION *sid, char *sqldate);
char   *time_unix2sql(CONNECTION *sid, time_t unixdate);
char   *time_unix2sqldate(CONNECTION *sid, time_t unixdate);
char   *time_unix2sqltime(CONNECTION *sid, time_t unixdate);
char   *time_unix2text(CONNECTION *sid, time_t unixdate);
char   *time_unix2datetext(CONNECTION *sid, time_t unixdate);
char   *time_unix2timetext(CONNECTION *sid, time_t unixdate);
char   *time_unix2lotimetext(CONNECTION *sid, time_t unixdate);
int     time_tzoffset(CONNECTION *sid, time_t unixdate);
int     time_tzoffset2(CONNECTION *sid, time_t unixdate, int userid);
time_t  time_wmgetdate(char *src);
/* html.c functions */
void  htpage_login(CONNECTION *sid);
void  htpage_logout(CONNECTION *sid);
void  htpage_stats(CONNECTION *sid);
void  htpage_frameset(CONNECTION *sid);
void  htpage_sidemenu(CONNECTION *sid);
void  htpage_topmenu(CONNECTION *sid, int menu);
void  htpage_reload(CONNECTION *sid);
void  htpage_motd(CONNECTION *sid);
void  htpage_header(CONNECTION *sid, char *title);
void  htpage_footer(CONNECTION *sid);
void  htselect_callfilter(CONNECTION *sid, int selected, char *baseuri);
void  htselect_eventfilter(CONNECTION *sid, int userid, int groupid, char *baseuri);
void  htselect_logfilter(CONNECTION *sid, int selected, char *baseuri);
void  htselect_notefilter(CONNECTION *sid, int selected, char *baseuri);
void  htselect_mailjump(CONNECTION *sid, int selected);
void  htselect_mailmbox(CONNECTION *sid, char *selected);
void  htselect_day(CONNECTION *sid, char *selected);
void  htselect_hour(CONNECTION *sid, int selected);
void  htselect_month(CONNECTION *sid, char *selected);
void  htselect_qhours(CONNECTION *sid, int selected);
void  htselect_qminutes(CONNECTION *sid, int selected);
void  htselect_minutes(CONNECTION *sid, int selected);
void  htselect_time(CONNECTION *sid, time_t unixtime);
void  htselect_year(CONNECTION *sid, int startyear, char *selected);
void  htselect_bookmarkfolder(CONNECTION *sid, int selected);
void  htselect_callaction(CONNECTION *sid, int selected);
void  htselect_contact(CONNECTION *sid, int selected);
void  htselect_eventclosingstatus(CONNECTION *sid, int selected);
void  htselect_eventstatus(CONNECTION *sid, int selected);
void  htselect_eventtype(CONNECTION *sid, int selected);
void  htselect_forumgroup(CONNECTION *sid, int selected);
void  htselect_group(CONNECTION *sid, int selected);
void  htselect_layout(CONNECTION *sid, int selected);
void  htselect_mailaccount(CONNECTION *sid, int selected);
void  htselect_number(CONNECTION *sid, int selected, int start, int end);
void  htselect_priority(CONNECTION *sid, int selected);
void  htselect_product(CONNECTION *sid, int selected);
void  htselect_reminder(CONNECTION *sid, int selected);
void  htselect_user(CONNECTION *sid, int selected);
void  htselect_zone(CONNECTION *sid, int selected);
char  *htview_callaction(CONNECTION *sid, int selected);
char  *htview_contact(CONNECTION *sid, int selected);
char  *htview_eventclosingstatus(CONNECTION *sid, int selected);
char  *htview_eventstatus(CONNECTION *sid, int selected);
char  *htview_eventtype(CONNECTION *sid, int selected);
char  *htview_forumgroup(CONNECTION *sid, int selected);
char  *htview_holiday(char *date);
char  *htview_product(CONNECTION *sid, int selected);
char  *htview_reminder(CONNECTION *sid, int selected);
char  *htview_user(CONNECTION *sid, int selected);
/* http.c functions */
char *get_mime_type(char *name);
void ReadPOSTData(CONNECTION *sid);
char *getgetenv(CONNECTION *sid, char *query);
char *getmimeenv(CONNECTION *sid, char *query, unsigned int *buffersize);
char *getpostenv(CONNECTION *sid, char *query);
char *getxmlenv(CONNECTION *sid, char *query);
char *getxmlparam(CONNECTION *sid, int param, char *reqtype);
char *getxmlstruct(CONNECTION *sid, char *reqmember, char *reqtype);
void read_cgienv(CONNECTION *sid);
int read_header(CONNECTION *sid);
void send_error(CONNECTION *sid, int status, char* title, char* text);
void send_fileheader(CONNECTION *sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
void send_header(CONNECTION *sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
/* modctl.c functions */
void *module_call(CONNECTION *sid, char *fn_name);
int module_menucall(CONNECTION *sid);
int modules_init(void);
/* sanity.c functions */
int sanity_dbcheck(void);
/* logging.c functions */
void logaccess(CONNECTION *sid, int loglevel, const char *format, ...);
void logerror(CONNECTION *sid, char *srcfile, int line, const char *format, ...);
void logdebug(CONNECTION *sid, char *srcfile, int line, const char *format, ...);
#ifdef DEBUG
void logdata(const char *format, ...);
#endif
/* server.c functions */
int getsid(void);
int closeconnect(CONNECTION *sid, int exitflag);
void server_restart(void);
void server_shutdown(void);
void dorequest(CONNECTION *sid);
void cgiinit(void);
void init(void);
#ifdef WIN32
unsigned _stdcall conn_reaper(void *x);
unsigned _stdcall accept_loop(void *x);
#else
void *conn_reaper(void *x);
void *accept_loop(void *x);
#endif
/* sql.c functions */
void  sql_disconnect(CONNECTION *sid);
void  sql_unsafedisconnect(CONNECTION *sid);
void  sql_freeresult(int sqr);
int   sql_update(CONNECTION *sid, char *sqlquery);
int   sql_updatef(CONNECTION *sid, char *format, ...);
int   sql_query(CONNECTION *sid, char *query);
int   sql_queryf(CONNECTION *sid, char *format, ...);
char *sql_getname(int sqr, int field);
char *sql_getvalue(int sqr, int tuple, int field);
char *sql_getvaluebyname(int sqr, int tuple, char *fieldname);
int   sql_numfields(int sqr);
int   sql_numtuples(int sqr);
/* xml-rpc.c */
void xmlrpc_fault(CONNECTION *sid, int faultid, char *fault);
void xmlrpc_main(CONNECTION *sid);


/* wmcodec.c functions */
char *DecodeRFC2047(CONNECTION *sid, char *src);
int DecodeHTML(short int reply, char *src, char *ctype, short int crlf);
int DecodeQP(short int reply, char *src, char *ctype);
int DecodeText(short int reply, char *src);
int EncodeBase64(CONNECTION *sid, char *src, int srclen);
int EncodeBase64file(FILE *fp, char *src, int srclen);
int DecodeBase64(CONNECTION *sid, char *src, char *ctype);
char *EncodeBase64string(CONNECTION *sid, char *src);
char *DecodeBase64string(CONNECTION *sid, char *src);
/* wmmain.c functions */
void wmsync(CONNECTION *sid, int verbose);
void wmloginform(CONNECTION *sid);
//int webmailheader(CONNECTION *sid, wmheader *header);
/* wmserver.c functions */
void wmclose(CONNECTION *sid);
int  wmfgets(CONNECTION *sid, char *buffer, int max, int fd);
int  wmffgets(CONNECTION *sid, char *buffer, int max, FILE **fp);
int  wmprints(CONNECTION *sid, const char *format, ...);
int  wmserver_connect(CONNECTION *sid, int verbose);
void wmserver_disconnect(CONNECTION *sid);
int  wmserver_smtpconnect(CONNECTION *sid);
int  wmserver_smtpauth(CONNECTION *sid);
void wmserver_smtpdisconnect(CONNECTION *sid);
int  wmserver_count(CONNECTION *sid);
int  wmserver_msgdele(CONNECTION *sid, int message);
int  wmserver_msghead(CONNECTION *sid, int message);
int  wmserver_msgretr(CONNECTION *sid, int message);
int  wmserver_msgsync(CONNECTION *sid, int remoteid, int localid, int verbose);
int  wmserver_msgsize(CONNECTION *sid, int message);
int  wmserver_uidl(CONNECTION *sid, int message, char *uidl);
int  is_msg_end(CONNECTION *sid, char *buffer);
//int  wmserver_mlistsync(CONNECTION *sid, char **uid_plist);
int  wmserver_mlistsync(CONNECTION *sid, char ***uidls);

void adminmain(CONNECTION *sid);			/* admin.c     */
void bookmarksmain(CONNECTION *sid);			/* bookmarks.c */
void calendarmain(CONNECTION *sid);			/* calendar.c  */
void callsmain(CONNECTION *sid);			/* calls.c     */
void contactmain(CONNECTION *sid);			/* contacts.c  */
void filemain(CONNECTION *sid);				/* files.c     */
void forummain(CONNECTION *sid);			/* forums.c    */
void printmenu(CONNECTION *sid);			/* menu.c      */
void messageautocheck(CONNECTION *sid);			/* messages.c  */
void messagemain(CONNECTION *sid);			/* messages.c  */
void notessublist(CONNECTION *sid, char *table, int index, int colspan); /* notes.c */
void notesmain(CONNECTION *sid);			/* notes.c     */
void ordermain(CONNECTION *sid);			/* orders.c    */
void productmain(CONNECTION *sid);			/* products.c  */
void searchmain(CONNECTION *sid);			/* searches.c  */
void tasklist(CONNECTION *sid, int userid, int groupid);/* tasks.c     */
void taskmain(CONNECTION *sid);				/* tasks.c     */
void profilemain(CONNECTION *sid);			/* users.c     */
void wmaddr_main(CONNECTION *sid);			/* wmaddr.c    */
void webmailmain(CONNECTION *sid);			/* wmmain.c    */
