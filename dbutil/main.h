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
#ifdef WIN32
#define DEFAULT_BASE_DIR "C:\\Program Files\\NullLogic Groupware"
#else
#define DEFAULT_BASE_DIR "/usr/local/nullgroupware"
#endif

#include "config.h"
#include <stdio.h>
#include <time.h>
#ifdef WIN32
	#include <windows.h>
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <ctype.h>
	#include <dlfcn.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <string.h>
#endif

typedef struct {
	char name[40];
	char value[512];
} sql_struct;
typedef struct {
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
	char      sql_fwdhost[128];
	short int sql_port;
	short int sql_maxconn;
	char      util_virusscan[255];
} CONFIG;
CONFIG config;
char rootpass[40];

int configread(void);
int dump_db(char *filename);
int init_db(void);
int restore_db(char *filename);
/* md5.c functions */
char *MD5Crypt(char *pw, char *salt);
/* sql.c functions */
void sqlDisconnect();
void sqlFreeconnect(int sqr);
int  sqlUpdate(int verbose, char *sqlquery);
int  sqlUpdatef(int verbose, char *format, ...);
int  sqlQuery(char *query);
int  sqlQueryf(char *format, ...);
char *sqlGetfieldname(int sqr, int fieldnumber);
char *sqlGetvalue(int sqr, int tuple, int field);
int  sqlNumfields(int sqr);
int  sqlNumtuples(int sqr);
char *str2sql(char *inbuffer);
int  sqlfprintf(FILE *fp, const char *format, ...);

#ifdef WIN32
#define dlopen(x, y) LoadLibrary(x)
#define dlsym(x, y) GetProcAddress(x, y)
#define dlclose(x) FreeLibrary(x)
int makemdb(char *filename);
#endif
