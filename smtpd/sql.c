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
#include "smtp_main.h"

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
#ifdef HAVE_SQLITE
typedef	sqlite *(*SQLITE_OPEN)(const char *filename, int mode, char **errmsg);
typedef	void    (*SQLITE_CLOSE)(sqlite *);
typedef	int     (*SQLITE_EXEC)(sqlite *, const char *sql, sqlite_callback, void *, char **errmsg);
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
#ifdef HAVE_SQLITE
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
#ifdef HAVE_SQLITE
static sqlite *db;
#endif
/* shared vars */
static short int sql_is_connected=0;
#ifdef WIN32
static HINSTANCE hinstLib=NULL;
#else
static void *hinstLib=NULL;
#endif

/****************************************************************************
 *	*DLLInit()
 *
 *	Purpose	: MySQL specific function to link the dll
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
int mysqlDLLInit()
{
#ifdef WIN32
	char *libext="dll";
#else
	char *libext="so";
#endif
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", proc.config.server_dir_lib, libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", proc.config.server_dir_lib, libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.12", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
//	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.10", libext);
//	fixslashes(libname);
//	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
#ifdef WIN32
	snprintf(libname, sizeof(libname)-1, "libmysql.%s", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
#endif
	goto fail;
found:
	if ((libmysql.close=(LIBMYSQL_CLOSE)dlsym(hinstLib, "mysql_close"))==NULL) goto fail;
	if ((libmysql.data_seek=(LIBMYSQL_DATA_SEEK)dlsym(hinstLib, "mysql_data_seek"))==NULL) goto fail;
	if ((libmysql.error=(LIBMYSQL_ERROR)dlsym(hinstLib, "mysql_error"))==NULL) goto fail;
	if ((libmysql.fetch_field_direct=(LIBMYSQL_FETCH_FIELD_DIRECT)dlsym(hinstLib, "mysql_fetch_field_direct"))==NULL) goto fail;
	if ((libmysql.fetch_row=(LIBMYSQL_FETCH_ROW)dlsym(hinstLib, "mysql_fetch_row"))==NULL) goto fail;
	if ((libmysql.free_result=(LIBMYSQL_FREE_RESULT)dlsym(hinstLib, "mysql_free_result"))==NULL) goto fail;
	if ((libmysql.init=(LIBMYSQL_INIT)dlsym(hinstLib, "mysql_init"))==NULL) goto fail;
	if ((libmysql.num_fields=(LIBMYSQL_NUM_FIELDS)dlsym(hinstLib, "mysql_num_fields"))==NULL) goto fail;
	if ((libmysql.num_rows=(LIBMYSQL_NUM_ROWS)dlsym(hinstLib, "mysql_num_rows"))==NULL) goto fail;
	if ((libmysql.ping=(LIBMYSQL_PING)dlsym(hinstLib, "mysql_ping"))==NULL) goto fail;
	if ((libmysql.query=(LIBMYSQL_QUERY)dlsym(hinstLib, "mysql_query"))==NULL) goto fail;
	if ((libmysql.real_connect=(LIBMYSQL_REAL_CONNECT)dlsym(hinstLib, "mysql_real_connect"))==NULL) goto fail;
	if ((libmysql.use_result=(LIBMYSQL_USE_RESULT)dlsym(hinstLib, "mysql_use_result"))==NULL) goto fail;
	return 0;
fail:
	logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to load %s", libname);
	memset((char *)&libmysql, 0, sizeof(libmysql));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
}

int odbcDLLInit()
{
#ifdef HAVE_ODBC
	return 0;
#else
	return -1;
#endif
}

int pgsqlDLLInit()
{
#ifdef WIN32
	char *libext="dll";
#else
	char *libext="so";
#endif
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libpq.%s", proc.config.server_dir_lib, libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libpq.%s", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
found:
	if ((libpgsql.clear=(LIBPGSQL_CLEAR)dlsym(hinstLib, "PQclear"))==NULL) goto fail;
	if ((libpgsql.errormessage=(LIBPGSQL_ERRORMESSAGE)dlsym(hinstLib, "PQerrorMessage"))==NULL) goto fail;
	if ((libpgsql.exec=(LIBPGSQL_EXEC)dlsym(hinstLib, "PQexec"))==NULL) goto fail;
	if ((libpgsql.finish=(LIBPGSQL_FINISH)dlsym(hinstLib, "PQfinish"))==NULL) goto fail;
	if ((libpgsql.fname=(LIBPGSQL_FNAME)dlsym(hinstLib, "PQfname"))==NULL) goto fail;
	if ((libpgsql.getvalue=(LIBPGSQL_GETVALUE)dlsym(hinstLib, "PQgetvalue"))==NULL) goto fail;
	if ((libpgsql.nfields=(LIBPGSQL_NFIELDS)dlsym(hinstLib, "PQnfields"))==NULL) goto fail;
	if ((libpgsql.ntuples=(LIBPGSQL_NTUPLES)dlsym(hinstLib, "PQntuples"))==NULL) goto fail;
	if ((libpgsql.resultstatus=(LIBPGSQL_RESULTSTATUS)dlsym(hinstLib, "PQresultStatus"))==NULL) goto fail;
	if ((libpgsql.setdblogin=(LIBPGSQL_SETDBLOGIN)dlsym(hinstLib, "PQsetdbLogin"))==NULL) goto fail;
	if ((libpgsql.status=(LIBPGSQL_STATUS)dlsym(hinstLib, "PQstatus"))==NULL) goto fail;
	return 0;
fail:
	logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to load %s", libname);
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
}

int SQLiteDLLInit()
{
#ifdef WIN32
	char *libext="dll";
#else
	char *libext="so";
#endif
	char libname[255];

#ifdef HAVE_SQLITE
	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite.%s", proc.config.server_dir_lib, libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite.%s", proc.config.server_dir_lib, libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite.%s", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite.%s", libext);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite.open=(SQLITE_OPEN)dlsym(hinstLib, "sqlite_open"))==NULL) goto fail;
	if ((libsqlite.exec=(SQLITE_EXEC)dlsym(hinstLib, "sqlite_exec"))==NULL) goto fail;
	if ((libsqlite.close=(SQLITE_CLOSE)dlsym(hinstLib, "sqlite_close"))==NULL) goto fail;
	return 0;
fail:
	logerror(NULL, __FILE__, __LINE__, "ERROR: Failed to load %s", libname);
	memset((char *)&libsqlite, 0, sizeof(libsqlite));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
#else
	return -1;
#endif
}

int sql_dll_unload()
{
	memset((char *)&libmysql, 0, sizeof(libmysql));
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	memset((char *)&libsqlite, 0, sizeof(libsqlite));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return 0;
}

/****************************************************************************
 *	mysqlDisconnect()
 *
 *	Purpose	: MySQL specific function to disconnect from the SQL server
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void mysqlDisconnect()
{
#ifdef HAVE_MYSQL
	if (mysock!=NULL) {
		libmysql.close(mysock);
		mysock=NULL;
	}
	sql_is_connected=0;
#endif
	return;
}

void odbcDisconnect()
{
#ifdef HAVE_ODBC
	SQLDisconnect(hDBC);
	if (hDBC) {
		SQLFreeHandle(SQL_HANDLE_DBC, hDBC);
		hDBC=NULL;
	}
	if (hEnv) {
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		hEnv=NULL;
	}
	sql_is_connected=0;
#endif
	return;
}

void pgsqlDisconnect()
{
#ifdef HAVE_PGSQL
	if (pgconn) {
		libpgsql.finish(pgconn);
		pgconn=NULL;
	}
	sql_is_connected=0;
#endif
	return;
}

void sqliteDisconnect()
{
#ifdef HAVE_SQLITE
	if (sql_is_connected==0) return;
	libsqlite.close(db);
	sql_is_connected=0;
#endif
	return;
}

/****************************************************************************
 *	mysqlConnect()
 *
 *	Purpose	: MySQL specific function to connect to the SQL server
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
int mysqlConnect(CONN *sid)
{
#ifdef HAVE_MYSQL
	if(mysqlDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	libmysql.init(&mysql);
	if (!(mysock=libmysql.real_connect(&mysql, proc.config.sql_hostname, proc.config.sql_username, proc.config.sql_password, proc.config.sql_dbname, proc.config.sql_port, NULL, 0))) {
		logerror(sid, __FILE__, __LINE__, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sql_is_connected=1;
	return 0;
#else
	return -1;
#endif
}

int odbcConnect(CONN *sid)
{
#ifdef HAVE_ODBC
	SQLCHAR szConnStr[255];
	SWORD cbConnStr;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];

	if (sql_is_connected) return 0;
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)==SQL_ERROR) {
		logerror(sid, __FILE__, __LINE__, "ODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, proc.config.sql_odbc_dsn, (short int)strlen(proc.config.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	sql_is_connected=1;
	return 0;
#else
	return -1;
#endif
}

int pgsqlConnect(CONN *sid)
{
#ifdef HAVE_PGSQL
	char port[12];

	if(pgsqlDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	memset (port, 0, sizeof(port));
	snprintf(port, sizeof(port)-1, "%d", proc.config.sql_port);
	pgconn=libpgsql.setdblogin(proc.config.sql_hostname, port, NULL, NULL, proc.config.sql_dbname, proc.config.sql_username, proc.config.sql_password);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		logerror(sid, __FILE__, __LINE__, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	sql_is_connected=1;
	return 0;
#else
	return -1;
#endif
}

int sqliteConnect(CONN *sid)
{
#ifdef HAVE_SQLITE
	char dbname[255];
	char *zErrMsg=0;

	if (SQLiteDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/%s.db", proc.config.server_dir_var_db, SERVER_BASENAME);
	fixslashes(dbname);
	db=libsqlite.open(dbname, 0, &zErrMsg);
	if (db==0) {
		logerror(sid, __FILE__, __LINE__, "SQLite Connect - %s", zErrMsg);
		return -1;
	}
	sql_is_connected=1;
	return 0;
#else
	return -1;
#endif
}

/****************************************************************************
 *	mysqlUpdate()
 *
 *	Purpose	: MySQL specific function to update SQL data
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
int mysqlUpdate(CONN *sid, char *sqlquery)
{
#ifdef HAVE_MYSQL
	if (libmysql.query(mysock, sqlquery)) {
		logerror(sid, __FILE__, __LINE__, "MYSQL error: %s", libmysql.error(&mysql));
		logerror(sid, __FILE__, __LINE__, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	return 0;
#else
	return -1;
#endif
}

int odbcUpdate(CONN *sid, char *sqlquery)
{
#ifdef HAVE_ODBC
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Update - SQLExecDirect %s", buf);
		logerror(sid, __FILE__, __LINE__, "ODBC UPDATE: [%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		hStmt=NULL;
		return -1;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
#else
	return -1;
#endif
}

int pgsqlUpdate(CONN *sid, char *sqlquery)
{
#ifdef HAVE_PGSQL
	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		logerror(sid, __FILE__, __LINE__, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		logerror(sid, __FILE__, __LINE__, "PGSQL error: %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);
	pgres=NULL;
	return 0;
#else
	return -1;
#endif
}

int sqliteUpdate(CONN *sid, char *sqlquery)
{
#ifdef HAVE_SQLITE
	char *zErrMsg=0;
	int rc;

	rc=libsqlite.exec(db, sqlquery, NULL, 0, &zErrMsg);
	if (rc!=SQLITE_OK) {
		logerror(sid, __FILE__, __LINE__, "SQLite error: %s", zErrMsg);
		logerror(sid, __FILE__, __LINE__, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
#else
	return -1;
#endif
}

/****************************************************************************
 *	mysqlQuery()
 *
 *	Purpose	: MySQL specific function to query the SQL server
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
int mysqlQuery(CONN *sid, int sqr, char *sqlquery)
{
#ifdef HAVE_MYSQL
	MYSQL_RES *myres;
	MYSQL_FIELD *MYfield;
	MYSQL_ROW MYrow;
	char *column;
	unsigned int field;
	unsigned int tuple;
	unsigned int rowsalloc;

	if (libmysql.ping(mysock)!=0) {
		if (mysqlConnect(sid)<0) return -1;
	}
	if (libmysql.query(mysock, sqlquery)) {
		logerror(sid, __FILE__, __LINE__, "MYSQL error: %s", libmysql.error(&mysql));
		logerror(sid, __FILE__, __LINE__, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.use_result(mysock))) {
		logerror(sid, __FILE__, __LINE__, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sqlreply[sqr].NumFields=(int)libmysql.num_fields(myres);
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		MYfield=libmysql.fetch_field_direct(myres, field);
		snprintf(column, MAX_FIELD_SIZE, "%s", MYfield->name);
		column+=strlen(column)+1;
	}
	// build our cursor and track the number of tuples
	rowsalloc=50;
	sqlreply[sqr].cursor=(char **)calloc(rowsalloc, sizeof(char *));
	// now to populate the cursor
	for (tuple=0;;tuple++) {
		if ((MYrow=libmysql.fetch_row(myres))==NULL) break;
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			logerror(sid, __FILE__, __LINE__, "malloc() error creating SQL cursor tuple.");
			exit(0);
		}
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
//			libmysql.data_seek(myres, tuple);
			snprintf(column, MAX_FIELD_SIZE, "%s", MYrow[field]);
			column+=strlen(column)+1;
		}
		if (tuple+2>rowsalloc) {
			rowsalloc+=50;
			sqlreply[sqr].cursor=(char **)realloc(sqlreply[sqr].cursor, rowsalloc*sizeof(char *));
		}
	}
	sqlreply[sqr].NumTuples=tuple;
	libmysql.free_result(myres);
	myres=NULL;
	return sqr;
#else
	return -1;
#endif
}

int odbcQuery(CONN *sid, int sqr, char *sqlquery)
{
#ifdef HAVE_ODBC
	SQLSMALLINT pccol;
	SDWORD collen;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];
	char *column;
	unsigned int field;
	unsigned int tuple;
	unsigned int rowsalloc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Query - SQLExecDirect %s", buf);
		logerror(sid, __FILE__, __LINE__, "ODBC SELECT: [%s]", sqlquery);
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror(sid, __FILE__, __LINE__, "ODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	sqlreply[sqr].NumFields=pccol;
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		rc=SQLDescribeCol(hStmt, (SQLSMALLINT)(field+1), column, MAX_FIELD_SIZE, NULL, NULL, NULL, NULL, NULL);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			logerror(sid, __FILE__, __LINE__, "ODBC Query - SQLDescribeCol %s", buf);
			return -1;
		}
		column+=strlen(column)+1;
	}
	// build our cursor and track the number of tuples
	rowsalloc=50;
	sqlreply[sqr].cursor=(char **)calloc(rowsalloc, sizeof(char *));
	// now to populate the cursor
	for (tuple=0;;tuple++) {
		rc=SQLFetch(hStmt);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) break;
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			logerror(sid, __FILE__, __LINE__, "malloc() error creating SQL cursor tuple.");
			exit(0);
		}
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, MAX_FIELD_SIZE, &collen);
			if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
				SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
				logerror(sid, __FILE__, __LINE__, "ODBC Getvalue - SQLGetData %s", buf);
				return -1;
			}
			column+=strlen(column)+1;
		}
		if (tuple+2>rowsalloc) {
			rowsalloc+=50;
			sqlreply[sqr].cursor=(char **)realloc(sqlreply[sqr].cursor, rowsalloc*sizeof(char *));
		}
	}
	sqlreply[sqr].NumTuples=tuple;
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return sqr;
#else
	return -1;
#endif
}

int pgsqlQuery(CONN *sid, int sqr, char *sqlquery)
{
#ifdef HAVE_PGSQL
	char query[8192];
	char *column;
	unsigned int field;
	unsigned int tuple;

	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		logerror(sid, __FILE__, __LINE__, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres = libpgsql.exec(pgconn, query);
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		logerror(sid, __FILE__, __LINE__, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		logerror(sid, __FILE__, __LINE__, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	sqlreply[sqr].NumFields=libpgsql.nfields(pgres);
	sqlreply[sqr].NumTuples=libpgsql.ntuples(pgres);
	// k..  now we know how many tuples and fields, we can build our cursor
	sqlreply[sqr].cursor=(char **)calloc(sqlreply[sqr].NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			logerror(sid, __FILE__, __LINE__, "Memory allocation error while creating SQL cursor tuple.");
//			closeconnect(1);
			exit(0);
		}
	}
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		snprintf(column, MAX_FIELD_SIZE, "%s", libpgsql.fname(pgres, field));
		column+=strlen(column)+1;
	}
	// now to populate the cursor
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", libpgsql.getvalue(pgres, tuple, field));
			column+=strlen(column)+1;
		}
	}
	if (pgres) {
		libpgsql.clear(pgres);
		pgres = libpgsql.exec(pgconn, "CLOSE myportal");
		libpgsql.clear(pgres);
		pgres = libpgsql.exec(pgconn, "END");
		libpgsql.clear(pgres);
		pgres=NULL;
	}
	return sqr;
#else
	return -1;
#endif
}

static int sqliteCallback(void *vpsqr, int argc, char **argv, char **azColName)
{
	char *column;
	static unsigned int rowsalloc;
	unsigned int field;
	int sqr=(int)vpsqr;

	sqlreply[sqr].NumFields=argc;
	if (sqlreply[sqr].NumTuples==0) {
		// retreive the field names
		column=sqlreply[sqr].fields;
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", azColName[field]);
			column+=strlen(column)+1;
		}
		// build our cursor and track the number of tuples
		sqlreply[sqr].NumTuples=0;
		rowsalloc=50;
//		sqlreply[sqr].cursor=(char **)calloc(rowsalloc, sizeof(char *));
	}
	// now to populate the cursor
	sqlreply[sqr].cursor[sqlreply[sqr].NumTuples]=calloc(MAX_TUPLE_SIZE, sizeof(char));
	if (sqlreply[sqr].cursor[sqlreply[sqr].NumTuples]==NULL) {
		printf("\nmalloc() error creating SQL cursor tuple.\n");
		exit(0);
	}
	column=sqlreply[sqr].cursor[sqlreply[sqr].NumTuples];
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		snprintf(column, MAX_FIELD_SIZE, "%s", argv[field]?argv[field]:"NULL");
		column+=strlen(column)+1;
	}
	if (sqlreply[sqr].NumTuples+2>rowsalloc) {
		rowsalloc+=50;
		sqlreply[sqr].cursor=(char **)realloc(sqlreply[sqr].cursor, rowsalloc*sizeof(char *));
	}
	sqlreply[sqr].NumTuples++;
	return 0;
}

int sqliteQuery(CONN *sid, int sqr, char *sqlquery)
{
#ifdef HAVE_SQLITE
	char *zErrMsg=0;
	int rc;

	sqlreply[sqr].NumFields=0;
	sqlreply[sqr].NumTuples=0;
	sqlreply[sqr].cursor=(char **)calloc(50, sizeof(char *));
	rc=libsqlite.exec(db, sqlquery, sqliteCallback, (void *)sqr, &zErrMsg);
	if (rc!=SQLITE_OK) {
		logerror(sid, __FILE__, __LINE__, "SQLite error: %s", zErrMsg);
		logerror(sid, __FILE__, __LINE__, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return sqr;
#else
	return -1;
#endif
}

/****************************************************************************
 *	sqlDisconnect()
 *
 *	Purpose	: Generic call to connect to the appropriate SQL data source
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void sql_disconnect(CONN *sid)
{
	if (!proc.RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	logaccess(sid, 4, "SQL Disconnection");
	if (strcmp(proc.config.sql_type, "ODBC")==0) {
		odbcDisconnect();
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		sqliteDisconnect();
	}
	sql_dll_unload();
	if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return;
}

void sql_unsafedisconnect(CONN *sid)
{
	logaccess(sid, 4, "SQL Disconnection");
	if (strcmp(proc.config.sql_type, "ODBC")==0) {
		odbcDisconnect();
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		sqliteDisconnect();
	}
	sql_dll_unload();
	return;
}

void sql_freeresult(int sqr)
{
	unsigned int tuple;

	if (!proc.RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	memset(sqlreply[sqr].fields, 0, sizeof(sqlreply[sqr].fields));
	if (sqlreply[sqr].cursor!=NULL) {
		for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
			if (sqlreply[sqr].cursor[tuple]) {
				free(sqlreply[sqr].cursor[tuple]);
				sqlreply[sqr].cursor[tuple]=NULL;
			}
		}
		free(sqlreply[sqr].cursor);
		sqlreply[sqr].cursor=NULL;
	}
	sqlreply[sqr].NumFields=0;
	sqlreply[sqr].NumTuples=0;
	proc.stats.sql_handlecount--;
//	logaccess(sid, 3, "SQL query [%d] freed", sqr);
	if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return;
}

int sql_update(CONN *sid, char *sqlquery)
{
	int rc=-1;

	if (!proc.RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	proc.stats.sql_updates++;
	if (sid==NULL) {
		logaccess(sid, 2, "SQL update by system: %s", sqlquery);
	} else {
		logaccess(sid, 2, "SQL update by %s: %s", sid->dat->user_username, sqlquery);
	}
	if (strcmp(proc.config.sql_type, "ODBC")==0) {
		if (odbcConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=odbcUpdate(sid, sqlquery);
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		if (mysqlConnect(sid)<0) {
			return -1;
		}
		rc=mysqlUpdate(sid, sqlquery);
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=pgsqlUpdate(sid, sqlquery);
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		if (sqliteConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=sqliteUpdate(sid, sqlquery);
	}
	if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_updatef(CONN *sid, char *format, ...)
{
	unsigned char sqlquery[8192];
	va_list ap;

	memset(sqlquery, 0, sizeof(sqlquery));
	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	va_end(ap);
	return sql_update(sid, sqlquery);
}

int sql_query(CONN *sid, char *query)
{
	int i;
	int rc=-1;

	if (!proc.RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	for (i=0;;i++) {
		if (i>=proc.config.sql_maxconn) {
			sleep(1);
			i=0;
			continue;
		}
		if (sqlreply[i].cursor==NULL) break;
	}
	proc.stats.sql_queries++;
	proc.stats.sql_handlecount++;
	if (sid==NULL) {
		logaccess(sid, 3, "SQL query [%d] by system: %s", i, query);
	} else {
		logaccess(sid, 3, "SQL query [%d] by %s: %s", i, sid->dat->user_username, query);
	}
	if (strcmp(proc.config.sql_type, "ODBC")==0) {
		if (odbcConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=odbcQuery(sid, i, query))<0) {
			odbcDisconnect();
			odbcConnect(sid);
		}
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		if (mysqlConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=mysqlQuery(sid, i, query))<0) {
			mysqlDisconnect();
			mysqlConnect(sid);
		}
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=pgsqlQuery(sid, i, query))<0) {
			pgsqlDisconnect();
			pgsqlConnect(sid);
		}
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		if (sqliteConnect(sid)<0) {
			if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=sqliteQuery(sid, i, query))<0) {
			sqliteDisconnect();
			sqliteConnect(sid);
		}
	}
	if (!proc.RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_queryf(CONN *sid, char *format, ...)
{
	unsigned char sqlquery[8192];
	va_list ap;

	memset(sqlquery, 0, sizeof(sqlquery));
	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	va_end(ap);
	return sql_query(sid, sqlquery);
}

char *sql_getname(int sqr, int fieldnumber)
{
	char *column=NULL;
	int i;

	if ((fieldnumber<0)||(fieldnumber+1>(int)sqlreply[sqr].NumFields)) return NULL;
	column=sqlreply[sqr].fields;
	for (i=0;i<fieldnumber;i++) {
		column+=strlen(column)+1;
	}
	return column;
}

char *sql_getvalue(int sqr, int tuple, int field)
{
	char *column=NULL;
	int i;

	if ((tuple<0)||(tuple+1>(int)sqlreply[sqr].NumTuples)) return NULL;
	if ((field<0)||(field+1>(int)sqlreply[sqr].NumFields)) return NULL;
	column=sqlreply[sqr].cursor[tuple];
	for (i=0;i<field;i++) {
		column+=strlen(column)+1;
	}
	return column;
}

char *sql_getvaluebyname(int sqr, int tuple, char *fieldname)
{
	char *column=NULL;
	unsigned int i;

	if ((tuple<0)||(tuple+1>(int)sqlreply[sqr].NumTuples)) return NULL;
//	if ((field<0)||(field+1>(int)sqlreply[sqr].NumFields)) return NULL;
	column=sqlreply[sqr].cursor[tuple];
	for (i=0;i<sqlreply[sqr].NumFields;i++) {
		if (strcasecmp(fieldname, sql_getname(sqr, i))==0) {
			return column;
		} else {
			column+=strlen(column)+1;
		}
	}
	return NULL;
}

int sql_numfields(int sqr)
{
	return sqlreply[sqr].NumFields;
}

int sql_numtuples(int sqr)
{
	return sqlreply[sqr].NumTuples;
}
