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
/* typedefs */
#ifdef WIN32
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
/* pthread_ typedefs */
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
	int in;
	int out;
} pipe_fd;
typedef struct {
	int userid;
	int time;
} u_avail;
typedef struct {
	short int minutes;
	short int dst;
	char *name;
} tzentry;
typedef struct {
	int lastref;
	int printed;
} _btree;
typedef struct {
	int id;
	int depth;
	int numchildren;
} _ptree;

/* MD5 */
#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif
struct MD5Context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};
typedef struct MD5Context MD5_CTX;
/* end MD5 */

typedef struct {
	short int popport;
	short int smtpport;
	short int socket;
	short int connected;
	short int remove;
	short int imapidx;
	short int codecstate;
	short int numfiles;
	int       imapmread;
	int       imapmsize;
	char      files[20][100];
	char      username[64];
	char      password[64];
	char      realname[64];
	char      organization[64];
	char      address[64];
	char      replyto[64];
	char      servertype[64];
	char      popserver[64];
	char      smtpserver[64];
	char      smtpauth[10];
	char      signature[1025];
	short int recvbufsize;
	short int recvbufoffset;
	char      recvbuf[2048];
} WEBMAIL;
typedef struct {
	char name[17];
	short int val;
} AUTH;
typedef struct {
	char name[17];
	short int val;
} PREF;
typedef struct {
	// USER DATA
	char       user_username[64];
	char       user_token[64];
	short int  user_uid;
	short int  user_gid;
	short int  user_did;
	AUTH       auth[MAX_AUTH_FIELDS];
	PREF       pref[MAX_PREF_FIELDS];
	short int  user_daystart;
	short int  user_daylength;
	short int  user_mailcurrent;
	short int  user_maildefault;
	short int  user_menustyle;
	short int  user_maxlist;
	short int  user_timezone;
	WEBMAIL   *wm;
	// INCOMING DATA
	short int cgi_lite;
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
	char in_RemoteAddr[16];
	int  in_RemotePort;
	char in_RequestMethod[8];
	char in_RequestURI[1024];
	char in_ScriptName[128];
	char in_UserAgent[128];
	char in_CGIPathInfo[128];
	char in_CGIScriptName[128];
	// TCP INPUT BUFFER
	short int recvbufsize;
	short int recvbufoffset;
	char      recvbuf[2048];
	// OUTGOING DATA
	short int out_status;
	short int out_headdone;
	short int out_bodydone;
	int out_bytecount;
	short int out_flushed;
	char out_CacheControl[40];
	char out_Connection[20];
	char out_ContentDisposition[64];
	int  out_ContentLength;
	char out_ContentType[128];
	char out_Date[40];
	char out_Expires[40];
	char out_LastModified[64];
	char out_Location[128];
	char out_Pragma[40];
	char out_Protocol[20];
	char out_Server[128];
	char out_SetCookieUser[128];
	char out_SetCookiePass[128];
	unsigned char out_ReplyData[MAX_REPLYSIZE];
	// BUFFERS
	short int lastbuf;
	char      smallbuf[4][4096];
	char      largebuf[16384];
} CONNDATA;
typedef struct {
	pthread_t handle;
#ifdef WIN32
	unsigned int id;
#else
	pthread_t id;
#endif
	short int state;
	short int socket;
	struct sockaddr_in ClientAddr;
	time_t ctime; // Creation time
	time_t atime; // Last Access time
	char *PostData;
	CONNDATA *dat;
} CONN;

typedef struct {
	unsigned int ctime;
	unsigned int NumFields;
	unsigned int NumTuples;
	char fields[1024];
	char **cursor;
} SQLRES;

typedef struct {
	char      colour_topmenu[10];
	char      colour_editform[10];
	char      colour_fieldname[10];
	char      colour_fieldnametext[10];
	char      colour_fieldval[10];
	char      colour_fieldvaltext[10];
	char      colour_tabletrim[10];
	char      colour_th[10];
	char      colour_thtext[10];
	char      colour_thlink[10];
	char      colour_links[10];
	char      server_dir_base[255];
	char      server_dir_bin[255];
	char      server_dir_cgi[255];
	char      server_dir_etc[255];
	char      server_dir_lib[255];
	char      server_dir_var[255];
	char      server_dir_var_backup[255];
	char      server_dir_var_db[255];
	char      server_dir_var_files[255];
	char      server_dir_var_htdocs[255];
	char      server_dir_var_log[255];
	char      server_dir_var_mail[255];
	char      server_dir_var_tmp[255];
	short int server_loglevel;
	char      server_username[33];
	char      http_hostname[128];
	short int http_port;
	short int http_maxconn;
	short int http_maxidle;
	int       http_maxpostsize;
	char      pop3_hostname[128];
	short int pop3_port;
	short int pop3_maxconn;
	short int pop3_maxidle;
	char      smtp_hostname[128];
	char      smtp_relayhost[128];
	short int smtp_port;
	short int smtp_maxconn;
	short int smtp_maxidle;
	char      sql_type[32];
	char      sql_username[32];
	char      sql_password[32];
	char      sql_dbname[32];
	char      sql_odbc_dsn[200];
	char      sql_hostname[128];
	short int sql_port;
	short int sql_maxconn;
	char      util_virusscan[255];
} CONFIG;
typedef struct {
	char version[11];
	char tax1name[21];
	char tax2name[21];
	float tax1percent;
	float tax2percent;
} DBINFO;
typedef struct {
	time_t starttime;
	int http_pages;
	int http_conns;
	int sql_queries;
	int sql_updates;
	int sql_handlecount;
} STATS;

typedef struct {
	char *fn_name;
	void *fn_ptr;
} FUNCTION;
typedef struct {
	char mod_name[40];
	short int mod_submenu;
	char mod_menuname[40];
	char mod_menuuri[40];
	char mod_menuperm[40];
	char fn_name[40];
	char fn_uri[40];
	void *fn_ptr;
} MODULE_MENU;
typedef struct {
	char mod_name[40];
	char fn_name[40];
	void *fn_ptr;
} MODULE_FUNC;
typedef struct {
	unsigned char config_filename[255];
	unsigned char program_name[255];
	pthread_t ListenThread;
	pthread_t DaemonThread;
	int RunAsCGI;
	int ListenSocket;
	CONFIG config;
	DBINFO info;
	STATS stats;
	MODULE_MENU mod_menuitems[MAX_MOD_MENUITEMS+1];
	MODULE_FUNC mod_functions[MAX_MOD_FUNCTIONS+1];
	#ifdef WIN32
	HINSTANCE hInst;
	WSADATA wsaData;
	#endif
} _PROC;

typedef struct {
	pthread_mutex_t Global;
	pthread_mutex_t DB_mheader;
	pthread_mutex_t FileList;
	pthread_mutex_t SQL;
} LOCKS;

typedef	int  (*MOD_INIT)(_PROC *, FUNCTION *);
typedef	void (*MOD_MAIN)(CONN *);

typedef	void (*MOD_HTML_HEADER)(CONN *, char *);
typedef	void (*MOD_HTML_FOOTER)(CONN *);
typedef	void (*MOD_HTML_LOGIN)(CONN *);
typedef	void (*MOD_HTML_LOGOUT)(CONN *);
typedef	void (*MOD_HTML_MOTD)(CONN *);
typedef	void (*MOD_HTML_FRAMESET)(CONN *);
typedef	void (*MOD_HTML_MENUFRAME)(CONN *);
typedef	void (*MOD_HTML_RELOADFRAME)(CONN *);
typedef	void (*MOD_HTML_TOPMENU)(CONN *, int);

typedef	int  (*MOD_MAIL_SYNC)(CONN *, int);
typedef	void (*MOD_NOTES_SUBLIST)(CONN *, char *, int, int);
typedef	void (*MOD_TASKS_LIST)(CONN *, int, int);
