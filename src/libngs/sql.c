/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "libngs.h"

#ifdef HAVE_MYSQL
	#ifdef HAVE_MYSQL_MYSQL_H
	#include "mysql/mysql.h"
	#else
	#include "stubs/mysql.h"
	#endif
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
	typedef	my_ulonglong  (STDCALL *LIBMYSQL_INSERT_ID)(MYSQL *);
	static struct {
		LIBMYSQL_CLOSE              close;
		LIBMYSQL_DATA_SEEK          data_seek;
		LIBMYSQL_ERROR              error;
		LIBMYSQL_FETCH_FIELD_DIRECT fetch_field_direct;
		LIBMYSQL_FETCH_ROW          fetch_row;
		LIBMYSQL_FREE_RESULT        free_result;
		LIBMYSQL_INIT               init;
		LIBMYSQL_NUM_FIELDS         num_fields;
		LIBMYSQL_NUM_ROWS           num_rows;
		LIBMYSQL_PING               ping;
		LIBMYSQL_QUERY              query;
		LIBMYSQL_REAL_CONNECT       real_connect;
		LIBMYSQL_USE_RESULT         use_result;
		LIBMYSQL_INSERT_ID          insert_id;
	} libmysql;
	static MYSQL mysql;
	static MYSQL *mysock=NULL;
#endif
#ifdef HAVE_ODBC
	#include <sql.h>
	#include <sqlext.h>
	static SQLHENV hEnv=NULL;
	static SQLHDBC hDBC=NULL;
	static SQLHSTMT hStmt=NULL;
#endif
#ifdef HAVE_PGSQL
	#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	#include "postgresql/libpq-fe.h"
	#else
	#include "stubs/pgsql.h"
	#endif
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
	static struct {
		LIBPGSQL_CLEAR        clear;
		LIBPGSQL_ERRORMESSAGE errormessage;
		LIBPGSQL_EXEC         exec;
		LIBPGSQL_FINISH       finish;
		LIBPGSQL_FNAME        fname;
		LIBPGSQL_GETVALUE     getvalue;
		LIBPGSQL_NFIELDS      nfields;
		LIBPGSQL_NTUPLES      ntuples;
		LIBPGSQL_RESULTSTATUS resultstatus;
		LIBPGSQL_SETDBLOGIN   setdblogin;
		LIBPGSQL_STATUS       status;
	} libpgsql;
	static PGconn *pgconn=NULL;
	static PGresult *pgres=NULL;
#endif
#ifdef HAVE_SQLITE2
	#include "stubs/sqlite2.h"
	typedef	sqlite *(*SQLITE2_OPEN)(const char *, int, char **);
	typedef	int     (*SQLITE2_EXEC)(sqlite *, const char *, sqlite_callback, void *, char **);
	typedef	void    (*SQLITE2_CLOSE)(sqlite *);
	typedef int     (*SQLITE2_LASTID)(sqlite *);
	static struct {
		SQLITE2_OPEN   open;
		SQLITE2_CLOSE  close;
		SQLITE2_EXEC   exec;
		SQLITE2_LASTID lastid;
	} libsqlite2;
	static sqlite *db2;
#endif
#ifdef HAVE_SQLITE3
	#include "stubs/sqlite3.h"
	typedef	int  (*SQLITE3_OPEN)(const char *, sqlite3 **);
	typedef	int  (*SQLITE3_EXEC)(sqlite3 *, const char *, sqlite3_callback, void *, char **);
	typedef	void (*SQLITE3_CLOSE)(sqlite3 *);
	typedef sqlite_int64 (*SQLITE3_LASTID)(sqlite3 *);
	static struct {
		SQLITE3_OPEN   open;
		SQLITE3_CLOSE  close;
		SQLITE3_EXEC   exec;
		SQLITE3_LASTID lastid;
	} libsqlite3;
	static sqlite3 *db3;
#endif

static short int sql_is_connected=0;
#ifdef WIN32
#define LIBEXT "dll"
static HINSTANCE hinstLib=NULL;
#else
#define LIBEXT "so"
static void *hinstLib=NULL;
#endif

#ifdef HAVE_MYSQL
static int mysqlDLLInit(nes_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysql.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.15", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.14", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", LIBEXT);
	goto fail;
found:
	if ((libmysql.close              = lib_sym(hinstLib, "mysql_close"))==NULL) goto fail;
	if ((libmysql.data_seek          = lib_sym(hinstLib, "mysql_data_seek"))==NULL) goto fail;
	if ((libmysql.error              = lib_sym(hinstLib, "mysql_error"))==NULL) goto fail;
	if ((libmysql.fetch_field_direct = lib_sym(hinstLib, "mysql_fetch_field_direct"))==NULL) goto fail;
	if ((libmysql.fetch_row          = lib_sym(hinstLib, "mysql_fetch_row"))==NULL) goto fail;
	if ((libmysql.free_result        = lib_sym(hinstLib, "mysql_free_result"))==NULL) goto fail;
	if ((libmysql.init               = lib_sym(hinstLib, "mysql_init"))==NULL) goto fail;
	if ((libmysql.num_fields         = lib_sym(hinstLib, "mysql_num_fields"))==NULL) goto fail;
	if ((libmysql.num_rows           = lib_sym(hinstLib, "mysql_num_rows"))==NULL) goto fail;
	if ((libmysql.ping               = lib_sym(hinstLib, "mysql_ping"))==NULL) goto fail;
	if ((libmysql.query              = lib_sym(hinstLib, "mysql_query"))==NULL) goto fail;
	if ((libmysql.real_connect       = lib_sym(hinstLib, "mysql_real_connect"))==NULL) goto fail;
	if ((libmysql.use_result         = lib_sym(hinstLib, "mysql_use_result"))==NULL) goto fail;
	if ((libmysql.insert_id          = lib_sym(hinstLib, "mysql_insert_id"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libmysql, 0, sizeof(libmysql));
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

static void mysqlDisconnect()
{
	if (mysock!=NULL) {
		libmysql.close(mysock);
		mysock=NULL;
	}
	sql_is_connected=0;
	return;
}

static int mysqlConnect(nes_state *N)
{
	obj_t *confobj=nes_settable(N, &N->g, "CONFIG");
	char *libpath=nes_getstr(N, confobj, "lib_path");
	char *host, *user, *pass, *db;
	int port;

	if(mysqlDLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	host=nes_getstr(N, confobj, "sql_hostname");
	user=nes_getstr(N, confobj, "sql_username");
	pass=nes_getstr(N, confobj, "sql_password");
	db=nes_getstr(N, confobj, "sql_dbname");
	port=(int)nes_getnum(N, confobj, "sql_port");
	libmysql.init(&mysql);
	if (!(mysock=libmysql.real_connect(&mysql, host, user, pass, db, port, NULL, 0))) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int mysqlUpdate(nes_state *N, char *sqlquery)
{
	int rc;

	if (libmysql.query(mysock, sqlquery)) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		log_error(N, "sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	rc=(int)libmysql.insert_id(mysock);
	return rc;
}

static int mysqlQuery(nes_state *N, obj_t *qobj, char *sqlquery)
{
	MYSQL_RES *myres;
	MYSQL_ROW MYrow;
	MYSQL_FIELD *MYfield;
	obj_t *cobj, *robj, *tobj;
	char *p;
	char name[8];
	unsigned int field, numfields, numtuples;

	if (libmysql.ping(mysock)!=0) {
		return -1;
	}
	if (libmysql.query(mysock, sqlquery)) {
		cobj=nes_setstr(NULL, qobj, "_error", libmysql.error(&mysql), -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", nes_tostr(NULL, cobj));
		log_error(N, "sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.use_result(mysock))) {
		cobj=nes_setstr(NULL, qobj, "_error", libmysql.error(&mysql), -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", nes_tostr(NULL, cobj));
		return -1;
	}
	numfields=(int)libmysql.num_fields(myres);
	numtuples=0;
	nes_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nes_setnum(NULL, qobj, "_fields", numfields);
	nes_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nes_settable(NULL, qobj, "_rows");
	for (numtuples=0;;numtuples++) {
		if ((MYrow=libmysql.fetch_row(myres))==NULL) break;
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", numtuples);
		tobj=nes_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			p=MYrow[field]?MYrow[field]:"NULL";
			MYfield=libmysql.fetch_field_direct(myres, field);
			nes_setstr(NULL, tobj, MYfield->name, p, strlen(p));
		}
	}
	nes_setnum(NULL, qobj, "_tuples", numtuples);
	libmysql.free_result(myres);
	myres=NULL;
	return 0;
}
#endif /* HAVE_MYSQL */

#ifdef HAVE_ODBC
static int odbcDLLInit()
{
	return 0;
}

static void odbcDisconnect()
{
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
	return;
}

static int odbcConnect(nes_state *N)
{
	SQLCHAR szConnStr[255];
	SWORD cbConnStr;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];

	if (sql_is_connected) return 0;
	rc=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, cnf.sql_odbc_dsn, (short int)strlen(cnf.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int odbcUpdate(nes_state *N, char *sqlquery)
{
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Update - SQLExecDirect %s", buf);
		log_error(N, "sql", __FILE__, __LINE__, 2, "ODBC UPDATE: [%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		hStmt=NULL;
		return -1;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
}

static int odbcQuery(nes_state *N, obj_t *qobj, char *sqlquery)
{
	SQLSMALLINT pccol;
	SDWORD slen;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];
	char *column;
	unsigned int field;
	unsigned int rowsalloc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Query - SQLExecDirect %s", buf);
		log_error(N, "sql", __FILE__, __LINE__, 2, "ODBC SELECT: [%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		hStmt=NULL;
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	numfields=pccol;
	numtuples=0;
	nes_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nes_setnum(NULL, qobj, "_fields", numfields);
	nes_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nes_settable(NULL, qobj, "_rows");
	for (numtuples=0;;numtuples++) {
		rc=SQLFetch(conn->hSTMT);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) break;
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", numtuples);
		tobj=nes_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			rc=SQLDescribeCol(conn->hSTMT, (SQLSMALLINT)(field+1), (SQLPOINTER)colname, MAX_OBJNAMELEN, NULL, NULL, NULL, NULL, NULL);
			cobj=nes_setstr(NULL, tobj, colname, NULL, 0);
			/* isn't there a way to peek at the size instead of faking a get? */
			rc=SQLGetData(conn->hSTMT, (SQLUSMALLINT)(field+1), SQL_C_CHAR, NULL, 0, &slen);
			if (slen>0) {
				cobj->val->size=slen;
				cobj->val->d.str=n_alloc(N, cobj->val->size+1, 0);
				if (cobj->val->d.str==NULL) log_error(N, "sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				rc=SQLGetData(conn->hSTMT, (SQLUSMALLINT)(field+1), SQL_C_CHAR, cobj->val->d.str, cobj->val->size+1, &slen);
				cobj->val->d.str[cobj->val->size]=0;
			}
		}
	}
	nes_setnum(NULL, qobj, "_tuples", numtuples);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
}
#endif /* HAVE_ODBC */

#ifdef HAVE_PGSQL
static int pgsqlDLLInit(nes_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libpq.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libpq.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libpgsql.clear        = lib_sym(hinstLib, "PQclear"))==NULL) goto fail;
	if ((libpgsql.errormessage = lib_sym(hinstLib, "PQerrorMessage"))==NULL) goto fail;
	if ((libpgsql.exec         = lib_sym(hinstLib, "PQexec"))==NULL) goto fail;
	if ((libpgsql.finish       = lib_sym(hinstLib, "PQfinish"))==NULL) goto fail;
	if ((libpgsql.fname        = lib_sym(hinstLib, "PQfname"))==NULL) goto fail;
	if ((libpgsql.getvalue     = lib_sym(hinstLib, "PQgetvalue"))==NULL) goto fail;
	if ((libpgsql.nfields      = lib_sym(hinstLib, "PQnfields"))==NULL) goto fail;
	if ((libpgsql.ntuples      = lib_sym(hinstLib, "PQntuples"))==NULL) goto fail;
	if ((libpgsql.resultstatus = lib_sym(hinstLib, "PQresultStatus"))==NULL) goto fail;
	if ((libpgsql.setdblogin   = lib_sym(hinstLib, "PQsetdbLogin"))==NULL) goto fail;
	if ((libpgsql.status       = lib_sym(hinstLib, "PQstatus"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

static void pgsqlDisconnect()
{
	if (pgconn) {
		libpgsql.finish(pgconn);
		pgconn=NULL;
	}
	sql_is_connected=0;
	return;
}

static int pgsqlConnect(nes_state *N)
{
	obj_t *confobj=nes_settable(N, &N->g, "CONFIG");
	char *libpath=nes_getstr(N, confobj, "lib_path");
	char *host, *user, *pass, *db, *port;

	if(pgsqlDLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	host=nes_getstr(N, confobj, "sql_hostname");
	user=nes_getstr(N, confobj, "sql_username");
	pass=nes_getstr(N, confobj, "sql_password");
	db=nes_getstr(N, confobj, "sql_dbname");
	port=nes_getstr(N, confobj, "sql_port");
	pgconn=libpgsql.setdblogin(host, port, NULL, NULL, db, user, pass);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int pgsqlUpdate(nes_state *N, char *sqlquery)
{
	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL error: %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);
	pgres=NULL;
	return 0;
}

static int pgsqlQuery(nes_state *N, obj_t *qobj, char *sqlquery)
{
	char *q;
	obj_t *robj, *tobj;
	char *p;
	char name[8];
	unsigned int field, tuple;
	unsigned int numfields, numtuples;

	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		libpgsql.clear(pgres);
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);

	if ((q=calloc(1, strlen(sqlquery)+42))==NULL) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "calloc error");
		pgsqlDisconnect();
		return -1;
	}
	sprintf(q, "DECLARE myportal CURSOR FOR %s", sqlquery);
	pgres = libpgsql.exec(pgconn, q);
	free(q);

	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		libpgsql.clear(pgres);
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);

	pgres = libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		libpgsql.clear(pgres);
		pgsqlDisconnect();
		return -1;
	}
	numfields=libpgsql.nfields(pgres);
	numtuples=libpgsql.ntuples(pgres);
	nes_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nes_setnum(NULL, qobj, "_fields", numfields);
	nes_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nes_settable(NULL, qobj, "_rows");
	for (tuple=0;tuple<numtuples;tuple++) {
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", tuple);
		tobj=nes_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			p=libpgsql.getvalue(pgres, tuple, field);
			p=p?p:"NULL";
			nes_setstr(NULL, tobj, libpgsql.fname(pgres, field), p, strlen(p));
		}
	}
	nes_setnum(NULL, qobj, "_tuples", numtuples);
	libpgsql.clear(pgres);

	pgres = libpgsql.exec(pgconn, "CLOSE myportal");
	libpgsql.clear(pgres);

	pgres = libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);

	pgres=NULL;
	return 0;
}
#endif /* HAVE_PGSQL */

#ifdef HAVE_SQLITE2
static int SQLite2DLLInit(nes_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite2.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite2.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite2.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite2.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite2.open   = (SQLITE2_OPEN)  lib_sym(hinstLib, "sqlite_open"))==NULL) goto fail;
	if ((libsqlite2.exec   = (SQLITE2_EXEC)  lib_sym(hinstLib, "sqlite_exec"))==NULL) goto fail;
	if ((libsqlite2.close  = (SQLITE2_CLOSE) lib_sym(hinstLib, "sqlite_close"))==NULL) goto fail;
	if ((libsqlite2.lastid = (SQLITE2_LASTID)lib_sym(hinstLib, "sqlite_last_insert_rowid"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

static void sqlite2Disconnect()
{
	if (sql_is_connected==0) return;
	libsqlite2.close(db2);
	sql_is_connected=0;
	return;
}

static int sqlite2Connect(nes_state *N)
{
	char *libpath=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "lib_path");
	char *varpath=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "var_path");
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite2DLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/db/%s.db2", varpath, SERVER_BASENAME);
	fixslashes(dbname);
	if ((db2=libsqlite2.open(dbname, 0, &zErrMsg))==0) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite Connect - %s", zErrMsg);
		return -1;
	}
	sql_is_connected=1;
	libsqlite2.exec(db2, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
	libsqlite2.exec(db2, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqlite2Update(nes_state *N, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

retry:
	rc=libsqlite2.exec(db2, sqlquery, NULL, 0, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		rc=(int)libsqlite2.lastid(db2);
		return rc;
	case SQLITE_BUSY:
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite busy error?: %s", zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	case SQLITE_CORRUPT:
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite corrupt error?: %s", zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	if (rc!=SQLITE_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite2Callback(void *vptr, int argc, char **argv, char **azColName)
{
	obj_t *qobj=vptr;
	obj_t *tobj;
	char name[8];
	unsigned int field;
	unsigned int numfields;
	unsigned int numtuples;
	char *p;

	numfields=argc;
	numtuples=(int)nes_getnum(NULL, qobj, "_tuples");
	tobj=nes_getobj(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	memset(name, 0, sizeof(name));
	sprintf(name, "%d", numtuples);
	/* get pointer to this record table */
	tobj=nes_settable(NULL, tobj, name);
	if (numtuples==0) nes_setnum(NULL, qobj, "_fields", numfields);
	for (field=0;field<numfields;field++) {
		if (argv==NULL) continue;
		p=argv[field]?argv[field]:"NULL";
		nes_setstr(NULL, tobj, azColName[field], p, strlen(p));
	}
	if (argv!=NULL) nes_setnum(NULL, qobj, "_tuples", numtuples+1);
	return 0;
}

static int sqlite2Query(nes_state *N, obj_t *qobj, char *sqlquery)
{
	obj_t *tobj;
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	tobj=nes_settable(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	nes_setnum(NULL, qobj, "_fields", 0);
	nes_setnum(NULL, qobj, "_tuples", 0);
retry:
	rc=libsqlite2.exec(db2, sqlquery, sqlite2Callback, qobj, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: busy or corrupt %d %s", rc, zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: unknown error %d %s", rc, zErrMsg);
		break;
	}
	log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
	log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE2 */

#ifdef HAVE_SQLITE3
static int SQLite3DLLInit(nes_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite3.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite3.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite3.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite3.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite3.open   = (SQLITE3_OPEN)  lib_sym(hinstLib, "sqlite3_open"))==NULL) goto fail;
	if ((libsqlite3.exec   = (SQLITE3_EXEC)  lib_sym(hinstLib, "sqlite3_exec"))==NULL) goto fail;
	if ((libsqlite3.close  = (SQLITE3_CLOSE) lib_sym(hinstLib, "sqlite3_close"))==NULL) goto fail;
	if ((libsqlite3.lastid = (SQLITE3_LASTID)lib_sym(hinstLib, "sqlite3_last_insert_rowid"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libsqlite3, 0, sizeof(libsqlite3));
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

static void sqlite3Disconnect()
{
	if (sql_is_connected==0) return;
	libsqlite3.close(db3);
	sql_is_connected=0;
	return;
}

static int sqlite3Connect(nes_state *N)
{
	char *libpath=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "lib_path");
	char *varpath=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "var_path");
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite3DLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/db/%s.db3", varpath, SERVER_BASENAME);
	fixslashes(dbname);
	if (libsqlite3.open(dbname, &db3)!=SQLITE_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite Connect - %s", zErrMsg);
		return -1;
	}
	sql_is_connected=1;
	libsqlite3.exec(db3, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	/* PRAGMA [database.]synchronous=OFF|ON|NORMAL|FULL */
	libsqlite3.exec(db3, "PRAGMA synchronous = OFF;", NULL, 0, &zErrMsg);
	/* PRAGMA temp_store = "default"|"memory"|"file" */
	libsqlite3.exec(db3, "PRAGMA temp_store = memory;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqlite3Update(nes_state *N, char *sqlquery)
{
//	obj_t *cobj;
	char *zErrMsg=0;
	int rc;
	short int retries=10;

retry:
	rc=libsqlite3.exec(db3, sqlquery, NULL, 0, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		rc=(int)libsqlite3.lastid(db3);
		return rc;
	case SQLITE_BUSY:
//		cobj=nes_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite busy error?: %s", zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	case SQLITE_CORRUPT:
//		cobj=nes_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite corrupt error?: %s", zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	if (rc!=SQLITE_OK) {
//		cobj=nes_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite3Callback(void *vptr, int argc, char **argv, char **azColName)
{
	obj_t *qobj=vptr;
	obj_t *tobj;
	char name[8];
	unsigned int field;
	unsigned int numfields;
	unsigned int numtuples;
	char *p;

	numfields=argc;
	numtuples=(int)nes_getnum(NULL, qobj, "_tuples");
	tobj=nes_getobj(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	memset(name, 0, sizeof(name));
	sprintf(name, "%d", numtuples);
	/* get pointer to this record table */
	tobj=nes_settable(NULL, tobj, name);
	tobj->val->attr&=~NST_AUTOSORT;
	if (numtuples==0) nes_setnum(NULL, qobj, "_fields", numfields);
	for (field=0;field<numfields;field++) {
		if (argv==NULL) continue;
		p=argv[field]?argv[field]:"NULL";
		nes_setstr(NULL, tobj, azColName[field], p, strlen(p));
	}
	if (argv!=NULL) nes_setnum(NULL, qobj, "_tuples", numtuples+1);
	return 0;
}

static int sqlite3Query(nes_state *N, obj_t *qobj, char *sqlquery)
{
	obj_t *cobj, *tobj;
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	nes_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	tobj=nes_settable(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	nes_setnum(NULL, qobj, "_fields", 0);
	nes_setnum(NULL, qobj, "_tuples", 0);
retry:
	rc=libsqlite3.exec(db3, sqlquery, sqlite3Callback, qobj, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		cobj=nes_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: busy or corrupt %d %s", rc, nes_tostr(NULL, cobj));
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		cobj=nes_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: unknown error %d %s", rc, nes_tostr(NULL, cobj));
		break;
	}
	cobj=nes_setstr(NULL, qobj, "_error", zErrMsg, -1);
	log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, nes_tostr(NULL, cobj));
	log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE3 */

static int _sql_dll_unload()
{
#ifdef HAVE_MYSQL
	memset((char *)&libmysql, 0, sizeof(libmysql));
#endif
#ifdef HAVE_PGSQL
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
#endif
#ifdef HAVE_SQLITE2
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
#endif
#ifdef HAVE_SQLITE3
	memset((char *)&libsqlite3, 0, sizeof(libsqlite3));
#endif
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return 0;
}

/*
 * Everything above here is private, and everything below constitutes the
 * standard interface.  functions preceding with a '_' should probably be
 * protected by wrapper functions with mutexes of some kind.
 */

void _sql_disconnect(nes_state *N)
{
	char *sqltype=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "sql_server_type");

	log_error(N, "sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if (strcmp(sqltype, "MYSQL")==0) {
#ifdef HAVE_MYSQL
		mysqlDisconnect();
#endif
	} else if (strcmp(sqltype, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(sqltype, "PGSQL")==0) {
#ifdef HAVE_PGSQL
		pgsqlDisconnect();
#endif
	} else if (strcmp(sqltype, "SQLITE2")==0) {
#ifdef HAVE_SQLITE2
		sqlite2Disconnect();
#endif
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		sqlite3Disconnect();
#endif
	}
	_sql_dll_unload();
	return;
}

void _sql_freeresult(nes_state *N, obj_t **qobj)
{
	if (*qobj!=NULL) {
		nes_unlinkval(N, *qobj);
		free(*qobj);
		*qobj=NULL;
	} else {
		log_error(N, "sql", __FILE__, __LINE__, 1, "freeing an unused sql result?");
	}
	log_error(N, "sql", __FILE__, __LINE__, 4, "SQL query [0x%08X] freed", qobj);
	return;
}

int _sql_update(nes_state *N, char *sqlquery)
{
	char *sqltype=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "sql_server_type");
	int rc=-1;

	log_error(N, "sql", __FILE__, __LINE__, 2, "SQL update: %s", sqlquery);
	if (strcmp(sqltype, "MYSQL")==0) {
#ifdef HAVE_MYSQL
		if (mysqlConnect(N)<0) return -1;
		rc=mysqlUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect(N)<0) return -1;
		rc=odbcUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "PGSQL")==0) {
#ifdef HAVE_PGSQL
		if (pgsqlConnect(N)<0) return -1;
		rc=pgsqlUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "SQLITE2")==0) {
#ifdef HAVE_SQLITE2
		if (sqlite2Connect(N)<0) return -1;
		rc=sqlite2Update(N, sqlquery);
#endif
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		if (sqlite3Connect(N)<0) return -1;
		rc=sqlite3Update(N, sqlquery);
#endif
	}
/*	log_error(N, "sql", __FILE__, __LINE__, 1, "lastid = %d", rc); */
	return rc;
}

int _sql_query(nes_state *N, obj_t **qobj, char *query)
{
	char *sqltype=nes_getstr(N, nes_settable(N, &N->g, "CONFIG"), "sql_server_type");
	obj_t *tobj;
	int rc=-1;

	if (*qobj==NULL) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		nes_linkval(N, tobj, NULL);
		tobj->val->type=NT_TABLE;
		tobj->val->attr&=~NST_AUTOSORT;
	}
	log_error(N, "sql", __FILE__, __LINE__, 3, "SQL query: [0x%08X] %s", *qobj, query);
	if (strcmp(sqltype, "MYSQL")==0) {
#ifdef HAVE_MYSQL
		if (mysqlConnect(N)<0) 	return -1;
		if ((rc=mysqlQuery(N, *qobj, query))<0) mysqlDisconnect();
#endif
	} else if (strcmp(sqltype, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect(N)<0) return -1;
		if ((rc=odbcQuery(N, *qobj, query))<0) odbcDisconnect();
		}
#endif
	} else if (strcmp(sqltype, "PGSQL")==0) {
#ifdef HAVE_PGSQL
		if (pgsqlConnect(N)<0) return -1;
		if ((rc=pgsqlQuery(N, *qobj, query))<0) pgsqlDisconnect();
#endif
	} else if (strcmp(sqltype, "SQLITE2")==0) {
#ifdef HAVE_SQLITE2
		if (sqlite2Connect(N)<0) return -1;
		if ((rc=sqlite2Query(N, *qobj, query))<0) sqlite2Disconnect();
#endif
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		if (sqlite3Connect(N)<0) return -1;
		if ((rc=sqlite3Query(N, *qobj, query))<0) sqlite3Disconnect();
#endif
	}
	return rc;
}

int _sql_updatef(nes_state *N, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery=calloc(8192, sizeof(char)))==NULL) {
		log_error(N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc=_sql_update(N, sqlquery);
	free(sqlquery);
	return rc;
}

int _sql_queryf(nes_state *N, obj_t **qobj, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery=calloc(8192, sizeof(char)))==NULL) {
		log_error(N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc=_sql_query(N, qobj, sqlquery);
	free(sqlquery);
	return rc;
}

char *sql_getname(nes_state *N, obj_t **qobj, int fieldnumber)
{
	obj_t *cobj;
	int j=0;

	if ((fieldnumber<0)||(fieldnumber+1>nes_getnum(N, *qobj, "_fields"))) return NULL;
	cobj=nes_getobj(N, *qobj, "_rows");
	if (cobj->val->type!=NT_TABLE) return "";
	for (cobj=cobj->val->d.table; cobj; cobj=cobj->next) {
		if (cobj->val->type==NT_NULL) return "";
		if (cobj->val->type!=NT_TABLE) continue;
		for (cobj=cobj->val->d.table; cobj; cobj=cobj->next) {
			if (j!=fieldnumber) { j++; continue; }
			return cobj->name;
		}
	}
	return NULL;
}

char *sql_getvalue(nes_state *N, obj_t **qobj, int tuple, int field)
{
	int i=0;
	int j=0;
	obj_t *tobj;
	obj_t *cobj;

	if ((tuple<0)||(tuple+1>nes_getnum(N, *qobj, "_tuples"))) return NULL;
	if ((field<0)||(field+1>nes_getnum(N, *qobj, "_fields"))) return NULL;
	tobj=nes_getobj(N, *qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return "";
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (cobj->val->type==NT_NULL) return "";
		if (cobj->val->type!=NT_TABLE) continue;
		if (i!=tuple) { i++; continue; }
		for (cobj=cobj->val->d.table; cobj; cobj=cobj->next) {
			if (j!=field) { j++; continue; }
			if (cobj->val->type==NT_STRING) {
				return cobj->val->d.str?cobj->val->d.str:"";
			} else {
				return nes_tostr(N, cobj);
			}
		}
	}
	return NULL;
}

char *sql_getvaluebyname(nes_state *N, obj_t **qobj, int tuple, char *fieldname)
{
	int i=0;
	obj_t *tobj;
	obj_t *cobj;

	if ((tuple<0)||(tuple+1>nes_getnum(N, *qobj, "_tuples"))) return NULL;
	tobj=nes_getobj(N, *qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return "";
	for (cobj=tobj->val->d.table; cobj; cobj=cobj->next) {
		if (cobj->val->type==NT_NULL) return "";
		if (cobj->val->type!=NT_TABLE) continue;
		if (i!=tuple) { i++; continue; }
		cobj=nes_getobj(N, cobj, fieldname);
		if (cobj->val->type==NT_STRING) {
			return cobj->val->d.str?cobj->val->d.str:"";
		} else {
			return nes_tostr(N, cobj);
		}
	}
	return NULL;
}

int sql_numfields(nes_state *N, obj_t **qobj)
{
	return (int)nes_getnum(N, *qobj, "_fields");
}

int sql_numtuples(nes_state *N, obj_t **qobj)
{
	return (int)nes_getnum(N, *qobj, "_tuples");
}
/*
char *str2sql(char *instring)
{
	static char buffer[16384];
	unsigned char ch;
	int bufferlength=0;
	unsigned int i=0;

	memset(buffer, 0, sizeof(buffer));
	while ((instring[i])&&(i<sizeof(buffer)-1)) {
		ch=instring[i];
		if (ch==0) break;
		if (ch<32) { i++; continue; }
		if (ch=='\'') {
			if (strcmp(config.sql_type, "ODBC")==0) {
				buffer[bufferlength]='\'';
			} else {
				buffer[bufferlength]='\\';
			}
			buffer[bufferlength+1]=ch;
			bufferlength+=2;
			i++;
			continue;
		}
		buffer[bufferlength]=ch;
		bufferlength++;
		i++;
	}
	return buffer;
}
*/

char *str2sql(char *outstring, int outlen, char *instring)
{
	unsigned char ch;
	int srcindex=0;
	int dstindex=0;

	while ((dstindex<outlen)&&(instring[srcindex])) {
		ch=instring[srcindex];
		if (ch=='\'') {
			outstring[dstindex]='\'';
			outstring[dstindex+1]=ch;
			dstindex+=2;
		} else if (ch=='\\') {
			outstring[dstindex]='\\';
			outstring[dstindex+1]=ch;
			dstindex+=2;
		} else {
			outstring[dstindex]=ch;
			dstindex++;
		}
		srcindex++;
	}
	if (dstindex<outlen) {
		outstring[dstindex]='\0';
	}
	outstring[outlen-1]='\0';
	return outstring;
}
