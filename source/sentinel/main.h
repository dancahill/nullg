/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
/* #includes */
#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifdef WIN32
	#pragma comment(lib, "libcmt.lib")
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "libmysql.lib")
	#pragma comment(lib, "libpq.a")
	#define HAVE_MYSQL
	#define HAVE_ODBC
	#define HAVE_PGSQL
	#define _MT 1
	#include <winsock2.h>
	#include <windows.h>
	#include <process.h>
	#include <shellapi.h>
	#include <signal.h>
	#include <windowsx.h>
	#include <io.h>
	#include <direct.h>
	#include "resource.h"
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <dirent.h>
	#include <netdb.h>
	#include <paths.h>
	#include <pthread.h>
	#include <signal.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/wait.h>
#endif
/* #defines and typedefs */
#define SERVER_NAME "Sentinel Groupware 1.0"
#ifdef WIN32
#define APPTITLE "Sentinel Groupware Server"
#define DEFAULT_BASE_DIR "C:\\Program Files\\Sentinel Groupware"
#else
#define DEFAULT_BASE_DIR "/usr/local/sentinel"
#endif
#define MAX_POSTSIZE 33554432 /* arbitrary 32 MB limit for POST request sizes */
#define MAX_REPLYSIZE 65536 /* arbitrary 64 KB limit for reply buffering */
/* SQL limits */
#define MAX_TUPLE_SIZE 8192
#define MAX_FIELD_SIZE 1024
//#define DEBUG
#define COLOR_EDITFORM "#F0F0F0"
#define COLOR_FNAME    "#E0E0E0"
#define COLOR_FTEXT    "#F0F0F0"
#define COLOR_TMENU    "#C0C0C0"
#define COLOR_TRIM     "#00A5D0"

#define CALFIELDS 16
#define COMPANYFIELDS 13
#define CONTACTFIELDS 22
#define LINKFIELDS 7
#define ORDERDETAILFIELDS 11
#define ORDERFIELDS 11
#define PRODUCTFIELDS 11
#define TASKFIELDS 11
#define USERFIELDS 44
#define MAXFIELDS 50

#ifdef WIN32
#define	MAXNAMLEN  255
/* stat & lstat defines */
#define S_IFMT		_S_IFMT   
#define S_IFDIR		_S_IFDIR  
#define S_IFCHR		_S_IFCHR  
#define S_IFIFO		_S_IFIFO  
#define S_IFREG		_S_IFREG  
#define S_IREAD		_S_IREAD  
#define S_IWRITE	_S_IWRITE 
#define S_IEXEC		_S_IEXEC 
#define S_IFLNK		0000000
#define S_IFSOCK	0000000
#define S_IFBLK		0000000
#define S_IROTH		0000004
#define S_IWOTH		0000002
#define S_IXOTH		0000001
#define S_ISDIR(x)	(x & _S_IFDIR)
#define S_ISLNK(x)	0
/* find ALL files! */
#define ATTRIBUTES (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR)

struct direct {
	ino_t d_ino;
	int d_reclen;
	int d_namlen;
	char d_name[MAXNAMLEN+1];
};
struct _dircontents {
	char *_d_entry;
	struct _dircontents *_d_next;
};
typedef struct _dirdesc {
	int dd_id;
	long dd_loc;
	struct _dircontents *dd_contents;
	struct _dircontents *dd_cp;
} DIR;
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
#endif

#define MD5_LONG unsigned int
#define MD5_CBLOCK 64
#define MD5_LBLOCK (MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16
typedef struct MD5state_st {
	MD5_LONG A,B,C,D;
	MD5_LONG Nl,Nh;
	MD5_LONG data[MD5_LBLOCK];
	int num;
} MD5_CTX;

/*****************************************************************************
** The following is to emulate the posix thread interface
*****************************************************************************/
#ifdef WIN32
typedef HANDLE pthread_t;
typedef struct thread_attr {
	DWORD dwStackSize;
	DWORD dwCreatingFlag;
	int priority;
} pthread_attr_t;
typedef struct {
	int dummy;
} pthread_condattr_t;
typedef unsigned int uint;
typedef struct {
	uint waiting;
	HANDLE semaphore;
} pthread_cond_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif

typedef struct {
	char version[16];
	char tax1name[32];
	char tax2name[32];
	float tax1percent;
	float tax2percent;
} DBINFO;
typedef struct {
	char Name[32];
	char Email[32];
	char Expiration[32];
	char LicenseType[32];
	char LicenseClass[32];
	char ProductID[32];
	char Hash[128];
} KEY;
typedef struct {
	DBINFO info;
	KEY key;
	char server_base_dir[200];
	char server_bin_dir[200];
	char server_etc_dir[200];
	char server_file_dir[200];
	char server_http_dir[200];
	char server_hostname[64];
	short int server_port;
	short int server_loglevel;
	short int server_maxconn;
	short int server_maxidle;
	char sql_type[32];
	char sql_username[32];
	char sql_password[32];
	char sql_dbname[32];
	char sql_odbc_dsn[200];
	char sql_hostname[64];
	short int sql_port;
	short int sql_maxconn;
} CONFIG;
typedef struct {
	char name[64];
	char value[1024];
} SQL_STRUCT;
typedef struct {
	// incoming data
	char in_ClientIP[16];
	char in_username[64];
	char in_token[64];
	char in_Connection[16];
	int  in_ContentLength;
	char in_ContentType[128];
	char in_Cookie[1024];
	char in_Host[64];
	char in_IfModifiedSince[64];
	char in_PathInfo[128];
	char in_Protocol[16];
	char in_QueryString[1024];
	char in_Referer[128];
	char in_RequestMethod[8];
	char in_RequestURI[1024];
	char in_ScriptName[128];
	char in_UserAgent[128];
	// outgoing data
	short int out_status;
	char out_CacheControl[16];
	char out_Connection[16];
	int  out_ContentLength;
	char out_Date[64];
	char out_Expires[64];
	char out_LastModified[64];
	char out_Pragma[16];
	char out_Protocol[16];
	char out_Server[128];
	char out_SetCookieUser[128];
	char out_SetCookiePass[128];
	char out_ContentType[128];
	char out_ReplyData[MAX_REPLYSIZE];
	short int out_headdone;
	short int out_bodydone;
	short int out_flushed;
	// sql data
	SQL_STRUCT tuple[MAXFIELDS];
	char envbuf[8192];
	char sqlbuf[8192];
	// webmail data
	char wm_username[64];
	char wm_password[64];
	char wm_pop3server[64];
	char wm_smtpserver[64];
	int wm_socket;
	int wm_connected;
} CONNDATA;
typedef struct {
	pthread_t handle;
	unsigned long int id;
	short int socket;
	struct sockaddr_in ClientAddr;
	time_t ctime; // Creation time
	time_t atime; // Last Access time
	char *PostData;
	CONNDATA *dat;
} CONNECTION;
typedef struct {
	unsigned int ctime;
	unsigned int NumFields;
	unsigned int NumTuples;
	char fields[1024];
	char **cursor;
} SQLRES;

/* global vars */
#ifdef WIN32
HINSTANCE hInst;
#endif
int RunAsCGI;
unsigned char program_name[255];
CONFIG config;
CONNECTION *conn;
SQLRES *sqlreply;

struct {
	pthread_mutex_t Crypt;
	pthread_mutex_t Global;
	pthread_mutex_t SQL;
} Lock;

/* function forwards */
/* main.c functions */
void dorequest(int sid);
/* admin.c functions */
int configread(void);
void adminmain(int sid);
/* common.c */
int sqldate2unix(char *sqldate);
int sqldatetime2unix(char *sqldate);
char *sqldate2text(char *olddate);
char *sqltime2text(char *oldtime);
char *strcatf(char *dest, const char *format, ...);
char *strcasestr(char *src, char *query);
void striprn(char *string);
int printht(const char *format, ...);
void printline(char *msgtext);
char *getdate(char *src);
/* md5.c functions */
char *MD5_crypt(const char *pw, const char *salt);
void MD5_Init(MD5_CTX *c);
void MD5_Update(MD5_CTX *c, const void *data, unsigned long len);
void MD5_Final(unsigned char *md, MD5_CTX *c);
/* files.c functions */
int dirlist(int sid);
int fileul(int sid);
int sendfile(int sid, unsigned char *file);
int recvfile(int sid);
/* html.c functions */
void printlogin(int sid);
void printlogout(int sid);
void printmenu(int sid);
void printmain(int sid);
void printmainright(int sid);
void printmaintop(int sid);
void printmainbottom(int sid);
void printmotd(int sid);
void printheader(int sid, char *title);
void printfooter(int sid);
void printerror(int sid, int status, char* title, char* text);
void yearselect(int sid, int startyear, char *selected);
void monthselect(int sid, char *selected);
void dayselect(int sid, char *selected);
void numberselect(int sid, int selected, int start, int end);
void priorityselect(int sid, char *selected);
void timeselect(int sid, char *selected);
void assigntoselect(int sid, char *selected);
void contactselect(int sid, int selected);
void companyselect(int sid, int selected);
char *reminder(int selected); /* calendar.c */
void reminderselect(int sid, int selected);
void repeatselect(int sid, char *selected);
void productselect(int sid, char *selected);
/* http.c functions */
void SwapChar(char *string, char oldchar, char newchar);
void FixSlashes(char *pOriginal);
int IntFromHex(char *pChars);
void URLDecode(unsigned char *pEncoded);
int verifyimage(char *image);
char *get_mime_type(char *name);
char *str2html(char *instring);
void ReadPOSTData(int sid);
char *getgetenv(int sid, char *query);
char *getmimeenv(int sid, char *query, unsigned int *buffersize);
char *getpostenv(int sid, char *query);
void read_cgienv(int sid);
int read_header(int sid);
void send_header(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
void send_fileheader(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod);
/* security.c functions */
char *setpass(char *rpassword);
int  getcookie(int sid);
int  setcookie(int sid);
int  renewcookie(int sid);
void sentinellogout(int sid);
int  auth(int sid, char *service);
void logaccess(int loglevel, const char *format, ...);
void logerror(const char *format, ...);
#ifdef DEBUG
void logdata(const char *format, ...);
#endif
/* server.c functions */
int getsid(void);
void flushbuffer(int sid);
int prints(const char *format, ...);
int hexprints(const char *format, ...);
int sgets(char *buffer, int max, int fd);
int closeconnect(int sid, int exitflag);
int sockinit(void);
void cgiinit(void);
void init(void);
#ifdef WIN32
//int WinFork(void);
//int WinForkInit(char *szFileMappingObj);
void WSAReaper(void *x);
#endif
void accept_loop(void *x);
/* sql.c functions */
//int sqlConnect();
void sqlDisconnect();
void sqlFreeconnect(int sqr);
int sqlUpdate(char *sqlquery);
int sqlUpdatef(char *format, ...);
int sqlQuery(char *query);
int sqlQueryf(char *format, ...);
char *sqlGetfieldname(int sqr, int fieldnumber);
char *sqlGetvalue(int sqr, int tuple, int field);
int  sqlNumfields(int sqr);
int  sqlNumtuples(int sqr);
char *str2sql(char *inbuffer);
char *field(int numfields, char *fieldname);
/* users.c functions */
void useredit(int sid);
void userlist(int sid);
void usersave(int sid);
/* win32.c functions */
#ifdef WIN32
void TrayIcon(int newstatus);
void TrayBlink();
BOOL CALLBACK NullDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
unsigned sleep(unsigned seconds);
DIR	*opendir(const char *);
void	closedir(DIR *);
#define	rewinddir(dirp)	seekdir(dirp, 0L)
void	seekdir(DIR *, long);
long	telldir(DIR *);
struct	direct *readdir(DIR *);
int	gettimeofday(struct timeval *tv, struct timezone *tz); 

/* registry and service control functions */
int get_reg_entries(void);

/*****************************************************************************
** The following is to emulate the posix thread interface
*****************************************************************************/
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
//int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, pthread_handler func, void *param);
int pthread_create(pthread_t *thread_id, pthread_attr_t *attr, unsigned (__stdcall *func)( void * ), void *param);
void pthread_exit(unsigned A);
#endif

void calendarmain(int sid);	/* calendar.c  */
void companymain(int sid);	/* companies.c */
void contactmain(int sid);	/* contacts.c  */
void forummain(int sid);	/* forums.c    */
void linksmain(int sid);	/* links.c     */
void printmenu(int sid);	/* menu.c      */
void messageautocheck(int sid);	/* messages.c  */
void messagemain(int sid);	/* messages.c  */
void ordermain(int sid);	/* orders.c    */
void productmain(int sid);	/* products.c  */
void querymain(int sid);	/* queries.c   */
void tasklist(int sid);		/* tasks.c     */
void taskmain(int sid);		/* tasks.c     */
void profilemain(int sid);	/* users.c     */
void webmailmain(int sid);	/* webmail.c   */
