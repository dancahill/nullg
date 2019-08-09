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
#if !defined(WIN32)
#define HAVE_FBSQL
#endif
#define HAVE_MYSQL
#ifdef WIN32
#define HAVE_ODBC
#endif
#define HAVE_PGSQL
#define HAVE_SQLITE2
#define HAVE_SQLITE3

#ifdef HAVE_FBSQL
	#ifdef HAVE_FBSQL_FBSQL_H
	#include "<ibase.h>"
	#else
	#include "nullgw/sql/fbsql_ibase.h"
	#endif
#endif

#ifdef HAVE_MYSQL
	#ifdef HAVE_MYSQL_MYSQL_H
	#include "mysql/mysql.h"
	#else
	#include "nullgw/sql/mysql.h"
	#endif
#endif
#ifdef HAVE_ODBC
	#include <sql.h>
	#include <sqlext.h>
#endif
#ifdef HAVE_PGSQL
	#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	#include "postgresql/libpq-fe.h"
	#else
	#include "nullgw/sql/pgsql.h"
	#endif
#endif
#ifdef HAVE_SQLITE2
	#include "nullgw/sql/sqlite2.h"
#endif
#ifdef HAVE_SQLITE3
	#include "nullgw/sql/sqlite3.h"
#endif

#ifdef HAVE_FBSQL
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_ATTACH_DATABASE)(ISC_STATUS*, short, const ISC_SCHAR*, isc_db_handle*, short, const ISC_SCHAR*);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_ALLOCATE_STATEMENT)(ISC_STATUS *, isc_db_handle *, isc_stmt_handle *);
typedef ISC_STATUS ISC_EXPORT_VARARG (*LIBFBSQL_ISC_START_TRANSACTION)(ISC_STATUS*, isc_tr_handle*, short, ...);
typedef ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_PREPARE)(ISC_STATUS*, isc_tr_handle*, isc_stmt_handle*, unsigned short, const ISC_SCHAR*, unsigned short, XSQLDA*);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_SQL_INFO)(ISC_STATUS*, isc_stmt_handle*, short, const ISC_SCHAR*, short, ISC_SCHAR*);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_EXECUTE)(ISC_STATUS*, isc_tr_handle*, isc_stmt_handle*, unsigned short, const XSQLDA*);
typedef	ISC_LONG   ISC_EXPORT (*LIBFBSQL_ISC_VAX_INTEGER)(const ISC_SCHAR*, short);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_DESCRIBE)(ISC_STATUS *, isc_stmt_handle *, unsigned short, XSQLDA *);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_COMMIT_TRANSACTION)(ISC_STATUS *, isc_tr_handle *);
typedef	ISC_LONG   ISC_EXPORT (*LIBFBSQL_FB_INTERPRET)(ISC_SCHAR*, unsigned int, const ISC_STATUS**);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_FREE_STATEMENT)(ISC_STATUS *, isc_stmt_handle *, unsigned short);

typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_FETCH)(ISC_STATUS *, isc_stmt_handle *, unsigned short, const XSQLDA *);
typedef	void       ISC_EXPORT (*LIBFBSQL_ISC_DECODE_SQL_TIME)(const ISC_TIME*, void*);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_OPEN_BLOB)(ISC_STATUS*, isc_db_handle*, isc_tr_handle*, isc_blob_handle*, ISC_QUAD*);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_GET_SEGMENT)(ISC_STATUS *, isc_blob_handle *, unsigned short *, unsigned short, ISC_SCHAR *);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_CLOSE_BLOB)(ISC_STATUS *, isc_blob_handle *);
typedef	void       ISC_EXPORT (*LIBFBSQL_ISC_DECODE_TIMESTAMP)(const ISC_TIMESTAMP*, void*);
typedef	void       ISC_EXPORT (*LIBFBSQL_ISC_DECODE_SQL_DATE)(const ISC_DATE*, void*);
typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DETACH_DATABASE)(ISC_STATUS *, isc_db_handle *);

#endif
#ifdef HAVE_MYSQL
typedef	void          (STDCALL *LIBMYSQL_CLOSE)(MYSQL *);
typedef	void          (STDCALL *LIBMYSQL_DATA_SEEK)(MYSQL_RES *, my_ulonglong);
typedef	char         *(STDCALL *LIBMYSQL_ERROR)(MYSQL *);
typedef	MYSQL_FIELD  *(STDCALL *LIBMYSQL_FETCH_FIELD_DIRECT)(MYSQL_RES *, unsigned int);
typedef	MYSQL_ROW     (STDCALL *LIBMYSQL_FETCH_ROW)(MYSQL_RES *);
typedef	void          (STDCALL *LIBMYSQL_FREE_RESULT)(MYSQL_RES *);
typedef	MYSQL        *(STDCALL *LIBMYSQL_INIT)(MYSQL *);
typedef	unsigned int  (STDCALL *LIBMYSQL_NUM_FIELDS)(MYSQL_RES *);
typedef	my_ulonglong  (STDCALL *LIBMYSQL_NUM_ROWS)(MYSQL_RES *);
typedef	int           (STDCALL *LIBMYSQL_PING)(MYSQL *);
typedef	int           (STDCALL *LIBMYSQL_QUERY)(MYSQL *, const char *);
typedef	MYSQL        *(STDCALL *LIBMYSQL_REAL_CONNECT)(MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned int);
typedef	MYSQL_RES    *(STDCALL *LIBMYSQL_STORE_RESULT)(MYSQL *);
#endif
#ifdef HAVE_PGSQL
typedef	void           (*LIBPGSQL_CLEAR)(PGresult *);
typedef	char          *(*LIBPGSQL_ERRORMESSAGE)(const PGconn *);
typedef	PGresult      *(*LIBPGSQL_EXEC)(PGconn *, const char *);
typedef	void           (*LIBPGSQL_FINISH)(PGconn *);
typedef	char          *(*LIBPGSQL_FNAME)(const PGresult *, int);
typedef	char          *(*LIBPGSQL_GETVALUE)(const PGresult *, int, int);
typedef	int            (*LIBPGSQL_NFIELDS)(const PGresult *);
typedef	int            (*LIBPGSQL_NTUPLES)(const PGresult *);
typedef	ExecStatusType (*LIBPGSQL_RESULTSTATUS)(const PGresult *);
typedef	PGconn        *(*LIBPGSQL_SETDBLOGIN)(const char *, const char *, const char *, const char *, const char *, const char *, const char *);
typedef	ConnStatusType (*LIBPGSQL_STATUS)(const PGconn *);
#endif
#ifdef HAVE_SQLITE2
typedef	sqlite *(*SQLITE2_OPEN)(const char *, int, char **);
typedef	int     (*SQLITE2_EXEC)(sqlite *, const char *, sqlite_callback, void *, char **);
typedef	void    (*SQLITE2_CLOSE)(sqlite *);
#endif
#ifdef HAVE_SQLITE3
typedef	int  (*SQLITE3_OPEN)(const char *, sqlite3 **);
typedef	int  (*SQLITE3_EXEC)(sqlite3 *, const char *, sqlite3_callback, void *, char **);
typedef	void (*SQLITE3_CLOSE)(sqlite3 *);
#endif

/* global vars */
#ifdef HAVE_FBSQL
struct {
	LIBFBSQL_ISC_ATTACH_DATABASE isc_attach_database;
	LIBFBSQL_ISC_DSQL_ALLOCATE_STATEMENT isc_dsql_allocate_statement;
	LIBFBSQL_ISC_START_TRANSACTION isc_start_transaction;
	LIBFBSQL_ISC_DSQL_PREPARE isc_dsql_prepare;
	LIBFBSQL_ISC_DSQL_SQL_INFO isc_dsql_sql_info;
	LIBFBSQL_ISC_DSQL_EXECUTE isc_dsql_execute;
	LIBFBSQL_ISC_VAX_INTEGER isc_vax_integer;
	LIBFBSQL_ISC_DSQL_DESCRIBE isc_dsql_describe;
	LIBFBSQL_ISC_COMMIT_TRANSACTION isc_commit_transaction;
	LIBFBSQL_FB_INTERPRET fb_interpret;
	LIBFBSQL_ISC_DSQL_FREE_STATEMENT isc_dsql_free_statement;
	LIBFBSQL_ISC_DSQL_FETCH isc_dsql_fetch;
	LIBFBSQL_ISC_DECODE_SQL_TIME isc_decode_sql_time;
	LIBFBSQL_ISC_OPEN_BLOB isc_open_blob;
	LIBFBSQL_ISC_GET_SEGMENT isc_get_segment;
	LIBFBSQL_ISC_CLOSE_BLOB isc_close_blob;
	LIBFBSQL_ISC_DECODE_TIMESTAMP isc_decode_timestamp;
	LIBFBSQL_ISC_DECODE_SQL_DATE isc_decode_sql_date;
	LIBFBSQL_ISC_DETACH_DATABASE isc_detach_database;
} libfbsql;
#endif
#ifdef HAVE_MYSQL
struct {
	LIBMYSQL_CLOSE close;
	LIBMYSQL_DATA_SEEK data_seek;
	LIBMYSQL_ERROR error;
	LIBMYSQL_FETCH_FIELD_DIRECT fetch_field_direct;
	LIBMYSQL_FETCH_ROW fetch_row;
	LIBMYSQL_FREE_RESULT free_result;
	LIBMYSQL_INIT init;
	LIBMYSQL_NUM_FIELDS num_fields;
	LIBMYSQL_NUM_ROWS num_rows;
	LIBMYSQL_PING ping;
	LIBMYSQL_QUERY query;
	LIBMYSQL_REAL_CONNECT real_connect;
	LIBMYSQL_STORE_RESULT store_result;
} libmysql;
#endif
#ifdef HAVE_PGSQL
struct {
	LIBPGSQL_CLEAR clear;
	LIBPGSQL_ERRORMESSAGE errormessage;
	LIBPGSQL_EXEC exec;
	LIBPGSQL_FINISH finish;
	LIBPGSQL_FNAME fname;
	LIBPGSQL_GETVALUE getvalue;
	LIBPGSQL_NFIELDS nfields;
	LIBPGSQL_NTUPLES ntuples;
	LIBPGSQL_RESULTSTATUS resultstatus;
	LIBPGSQL_SETDBLOGIN setdblogin;
	LIBPGSQL_STATUS status;
} libpgsql;
#endif
#ifdef HAVE_SQLITE2
struct {
	SQLITE2_OPEN open;
	SQLITE2_CLOSE close;
	SQLITE2_EXEC exec;
} libsqlite2;
#endif
#ifdef HAVE_SQLITE3
struct {
	SQLITE3_OPEN open;
	SQLITE3_CLOSE close;
	SQLITE3_EXEC exec;
} libsqlite3;
#endif

#ifdef HAVE_FBSQL
static isc_db_handle isc_db;
static isc_tr_handle trans;
static isc_stmt_handle stmt;
static XSQLDA ISC_FAR *sqlda;
static long fetch_stat;
static short trans_block;
static int numfields;
static int numtuples;
#define    MAXLEN    8192
static long buffer[MAXLEN];
#endif
#ifdef HAVE_MYSQL
static MYSQL mysql;
static MYSQL *mysock=NULL;
#endif
#ifdef HAVE_ODBC
static SQLHENV hEnv=NULL;
static SQLHDBC hDBC=NULL;
static SQLHSTMT hStmt=NULL;
#endif
#ifdef HAVE_PGSQL
static PGconn *pgconn=NULL;
static PGresult *pgres=NULL;
#endif
#ifdef HAVE_SQLITE2
static sqlite *db2;
#endif
#ifdef HAVE_SQLITE3
static sqlite3 *db3;
#endif
/* shared vars */
static short int sql_is_connected=0;
#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif
