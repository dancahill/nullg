/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
typedef struct MD5Context MD5_CONTEXT;
#define MD5_SIZE 16
/* end MD5 */

typedef struct {
	int in;
	int out;
} pipe_fd;

typedef struct {
	short int socket;
	struct sockaddr_in ClientAddr;
	SSL *ssl;
	time_t ctime; /* Creation time */
	time_t atime; /* Last Access time */
	unsigned int bytes_in;
	unsigned int bytes_out;
	short int want_close;
	/* TCP INPUT BUFFER */
	short int recvbufsize;
	short int recvbufoffset;
	char      recvbuf[2048];
} TCP_SOCKET;

typedef struct {
	unsigned int ctime;
	unsigned int NumFields;
	unsigned int NumTuples;
	char fields[1024];
	char **cursor;
} SQLRES;

typedef struct {
	char      uid[33];
	char      gid[33];
	short int loglevel;
	int       umask;
	char      langcode[8];
	char      hostname[128];
	char      dir_base[255];
	char      dir_bin[255];
	char      dir_etc[255];
	char      dir_lib[255];
	char      dir_var[255];
	char      dir_var_backup[255];
	char      dir_var_cgi[255];
	char      dir_var_db[255];
	char      dir_var_domains[255];
	char      dir_var_htdocs[255];
	char      dir_var_log[255];
	char      dir_var_spool[255];
	char      dir_var_tmp[255];
	char      sql_type[32];
	char      sql_odbc_dsn[200];
	char      sql_hostname[128];
	short int sql_port;
	char      sql_dbname[32];
	char      sql_username[32];
	char      sql_password[32];
	char      ssl_cert[255];
	char      ssl_key[255];
} GLOBAL_CONFIG;
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
	int pop3_conns;
	int smtp_conns;
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
#ifdef WIN32
	HINSTANCE hinstLib;
#else
	void *hinstLib;
#endif
	void *mod_init;
	void *mod_exec;
	void *mod_cron;
	void *mod_exit;
} SRVMOD;
typedef struct {
	unsigned char config_filename[255];
	unsigned char program_name[255];
	pthread_t DaemonThread;
	GLOBAL_CONFIG config;
	DBINFO info;
	STATS stats;
	SRVMOD srvmod[MAX_MOD_FUNCTIONS+1];
	unsigned short ssl_is_loaded;
	SSL_CTX *ssl_ctx;
	SSL_METHOD *ssl_meth;
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

typedef	int (*SRVMOD_INIT)(_PROC *, FUNCTION *);
typedef	int (*SRVMOD_EXEC)(void);
