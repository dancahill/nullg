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
#ifdef WIN32
#define HAVE_ODBC
#define DEFAULT_BASE_DIR "C:\\Program Files\\Sentinel Groupware"
#else
#define DEFAULT_BASE_DIR "/usr/local/sentinel"
#endif
//#define DEBUG

#include <ctype.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "db-mdac.h"
#include "db-mysql.h"
#include "db-pgsql.h"

#ifdef WIN32
	#define HAVE_MYSQL
	#define HAVE_PGSQL
	#include <windows.h>
	#include <process.h>
	#include <io.h>
	#include <direct.h>
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <dirent.h>
	#include <netdb.h>
	#include <paths.h>
	#include <signal.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <sys/wait.h>
#endif

typedef struct {
	char sql_type[32];
	char sql_hostname[64];
	char sql_port[10];
	char sql_username[32];
	char sql_password[32];
	char sql_dbname[32];
	char sql_odbc_dsn[100];
} db_struct;
typedef struct {
	char name[40];
	char value[512];
} sql_struct;

db_struct sql;

/* sql.c functions */
void sqlDisconnect();
void sqlFreeconnect(int sqr);
int sqlUpdate(int verbose, char *sqlquery);
int sqlUpdatef(int verbose, char *format, ...);
int sqlQuery(char *query);
int sqlQueryf(char *format, ...);
char *sqlGetfieldname(int sqr, int fieldnumber);
char *sqlGetvalue(int sqr, int tuple, int field);
int  sqlNumfields(int sqr);
int  sqlNumtuples(int sqr);
char *str2sql(char *inbuffer);

int configread(void);
int dump_db(char *filename);
int init_db(void);
int restore_db(char *filename);
#ifdef WIN32
int makemdb(char *filename);
#endif
