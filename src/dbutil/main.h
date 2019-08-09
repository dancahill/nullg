/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "nullgw/config-nt.h"
#else
#include "nullgw/config.h"
#endif
#include <stdio.h>
#include <time.h>
#ifdef WIN32
	#include <winsock2.h>
	#include <windows.h>
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp stricmp
	#define strncasecmp strnicmp
#else
	#include <ctype.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/stat.h>
#endif
#include "nullgw/common/defines.h"
#include "nsp/nsp.h"

typedef struct {
	unsigned int ctime;
	unsigned int NumFields;
	unsigned int NumTuples;
	char fields[1024];
	char **cursor;
} SQLRES;
typedef struct {
	char name[40];
	char value[512];
} sql_struct;
typedef struct {
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
	char      sql_dbname[64];
	char      sql_username[32];
	char      sql_password[32];
} CONFIG;
CONFIG config;
char rootpass[40];

int conf_read(void);
int dump_db(char *filename);
int dump_db_v2(char *filename, int dumptype);
int init_db(void);
int restore_db(char *filename);
/* dbiov2.c */
int dump_db_version2(char *filename);
/* md5.c functions */
char *MD5Crypt(char *pw, char *salt);
/* sql.c functions */
int   sql_dll_unload();
void  sql_disconnect();
void  sql_freeresult(SQLRES *sqr);
int   sql_update(int verbose, char *sqlquery);
int   sql_updatef(int verbose, char *format, ...);
int   sql_query(SQLRES *sqr, char *query);
int   sql_queryf(SQLRES *sqr, char *format, ...);
char *sql_getname(SQLRES *sqr, int fieldnumber);
char *sql_getvalue(SQLRES *sqr, int tuple, int field);
char *sql_getvaluebyname(SQLRES *sqr, int tuple, char *fieldname);
int   sql_numfields(SQLRES *sqr);
int   sql_numtuples(SQLRES *sqr);
char *str2sql(char *inbuffer);
int   sqlfprintf(FILE *fp, const char *format, ...);

#ifdef WIN32
//int makemdb(char *filename);
#endif

#define DUMP_NGW1 1
#define DUMP_NGW2 2
#define DUMP_LDIF 3
