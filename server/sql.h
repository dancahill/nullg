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
#ifdef HAVE_MYSQL
	#ifdef HAVE_MYSQL_MYSQL_H
	#include "mysql/mysql.h"
	#else
	#include "sql/mysql.h"
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
	#include "sql/pgsql.h"
	#endif
#endif
#ifdef HAVE_SQLITE3
	#include "sql/sqlite3.h"
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
typedef	MYSQL_RES    *(STDCALL *LIBMYSQL_USE_RESULT)(MYSQL *);
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
#ifdef HAVE_SQLITE3
typedef	int  (*SQLITE_OPEN)(const char *filename, sqlite3 **ppDb);
typedef	int  (*SQLITE_EXEC)(sqlite3 *, const char *sql, sqlite_callback, void *, char **errmsg);
typedef	void (*SQLITE_CLOSE)(sqlite3 *);
#endif

/* global vars */
#ifdef HAVE_MYSQL
static struct {
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
	LIBMYSQL_USE_RESULT use_result;
} libmysql;
#endif
#ifdef HAVE_PGSQL
static struct {
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
#ifdef HAVE_SQLITE3
static struct {
	SQLITE_OPEN open;
	SQLITE_CLOSE close;
	SQLITE_EXEC exec;
} libsqlite;
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
#ifdef HAVE_SQLITE3
static sqlite3 *db;
#endif
/* shared vars */
static short int sql_is_connected=0;
#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif