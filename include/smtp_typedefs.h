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
	// USER DATA
	char      user_username[64];
	char      user_token[64];
	short int user_uid;
	short int user_gid;
	short int user_did;
	short int user_mailcurrent;
	short int user_maildefault;
	// BUFFERS
	short int lastbuf;
	char      smallbuf[4][2048];
	short int recvbufsize;
	short int recvbufoffset;
	char      recvbuf[2048];
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
	char      util_scanfile[255];
	char      util_scanmail[255];
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
	unsigned char config_filename[255];
	unsigned char program_name[255];
	pthread_t ListenThread;
	pthread_t DaemonThread;
	int RunAsCGI;
	int ListenSocket;
	CONFIG config;
	DBINFO info;
	STATS stats;
	#ifdef WIN32
	HINSTANCE hInst;
	WSADATA wsaData;
	#endif
} _PROC;

typedef struct {
	pthread_mutex_t SQL;
} LOCKS;
