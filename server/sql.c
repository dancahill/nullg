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
#include "main.h"
#include "sql.h"

#ifdef WIN32
#define LIBEXT "dll"
#else
#define LIBEXT "so"
#endif

#ifdef HAVE_MYSQL
static int mysqlDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.12", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
#ifdef WIN32
	snprintf(libname, sizeof(libname)-1, "libmysql.%s", LIBEXT);
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
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libmysql, 0, sizeof(libmysql));
	if (hinstLib!=NULL) dlclose(hinstLib);
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

static int mysqlConnect()
{
	if(mysqlDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	libmysql.init(&mysql);
	if (!(mysock=libmysql.real_connect(&mysql, proc.config.sql_hostname, proc.config.sql_username, proc.config.sql_password, proc.config.sql_dbname, proc.config.sql_port, NULL, 0))) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int mysqlUpdate(char *sqlquery)
{
	if (libmysql.query(mysock, sqlquery)) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		log_error("sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int mysqlQuery(SQLRES *sqr, char *sqlquery)
{
	MYSQL_RES *myres;
	MYSQL_FIELD *MYfield;
	MYSQL_ROW MYrow;
	char *column;
	unsigned int field;
	unsigned int tuple;
	unsigned int rowsalloc;

	if (libmysql.ping(mysock)!=0) {
		if (mysqlConnect()<0) return -1;
	}
	if (libmysql.query(mysock, sqlquery)) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		log_error("sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.use_result(mysock))) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sqr->NumFields=(int)libmysql.num_fields(myres);
	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		MYfield=libmysql.fetch_field_direct(myres, field);
		snprintf(column, MAX_FIELD_SIZE, "%s", MYfield->name);
		column+=strlen(column)+1;
	}
	/* build our cursor and track the number of tuples */
	rowsalloc=50;
	sqr->cursor=(char **)calloc(rowsalloc, sizeof(char *));
	/* now to populate the cursor */
	for (tuple=0;;tuple++) {
		if ((MYrow=libmysql.fetch_row(myres))==NULL) break;
		sqr->cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqr->cursor[tuple]==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "malloc() error creating SQL cursor tuple.");
			exit(-1);
		}
		column=sqr->cursor[tuple];
		for (field=0;field<sqr->NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", MYrow[field]);
			column+=strlen(column)+1;
		}
		if (tuple+2>rowsalloc) {
			rowsalloc+=50;
			sqr->cursor=(char **)realloc(sqr->cursor, rowsalloc*sizeof(char *));
		}
	}
	sqr->NumTuples=tuple;
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

static int odbcConnect()
{
	SQLCHAR szConnStr[255];
	SWORD cbConnStr;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];

	if (sql_is_connected) return 0;
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)==SQL_ERROR) {
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, proc.config.sql_odbc_dsn, (short int)strlen(proc.config.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int odbcUpdate(char *sqlquery)
{
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Update - SQLExecDirect %s", buf);
		log_error("sql", __FILE__, __LINE__, 2, "ODBC UPDATE: [%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		hStmt=NULL;
		return -1;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
}

static int odbcQuery(SQLRES *sqr, char *sqlquery)
{
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
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLExecDirect %s", buf);
		log_error("sql", __FILE__, __LINE__, 2, "ODBC SELECT: [%s]", sqlquery);
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	sqr->NumFields=pccol;
	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		rc=SQLDescribeCol(hStmt, (SQLSMALLINT)(field+1), column, MAX_FIELD_SIZE, NULL, NULL, NULL, NULL, NULL);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLDescribeCol %s", buf);
			return -1;
		}
		column+=strlen(column)+1;
	}
	/* build our cursor and track the number of tuples */
	rowsalloc=50;
	sqr->cursor=(char **)calloc(rowsalloc, sizeof(char *));
	/* now to populate the cursor */
	for (tuple=0;;tuple++) {
		rc=SQLFetch(hStmt);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) break;
		sqr->cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqr->cursor[tuple]==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "malloc() error creating SQL cursor tuple.");
			exit(-1);
		}
		column=sqr->cursor[tuple];
		for (field=0;field<sqr->NumFields;field++) {
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, MAX_FIELD_SIZE, &collen);
			if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
				SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
				log_error("sql", __FILE__, __LINE__, 1, "ODBC Getvalue - SQLGetData %s", buf);
				return -1;
			}
			column+=strlen(column)+1;
		}
		if (tuple+2>rowsalloc) {
			rowsalloc+=50;
			sqr->cursor=(char **)realloc(sqr->cursor, rowsalloc*sizeof(char *));
		}
	}
	sqr->NumTuples=tuple;
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
}
#endif /* HAVE_ODBC */

#ifdef HAVE_PGSQL
static int pgsqlDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libpq.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libpq.%s", LIBEXT);
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
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	if (hinstLib!=NULL) dlclose(hinstLib);
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

static int pgsqlConnect()
{
	char port[12];

	if(pgsqlDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	memset (port, 0, sizeof(port));
	snprintf(port, sizeof(port)-1, "%d", proc.config.sql_port);
	pgconn=libpgsql.setdblogin(proc.config.sql_hostname, port, NULL, NULL, proc.config.sql_dbname, proc.config.sql_username, proc.config.sql_password);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int pgsqlUpdate(char *sqlquery)
{
	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL error: %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);
	pgres=NULL;
	return 0;
}

static int pgsqlQuery(SQLRES *sqr, char *sqlquery)
{
	char query[8192];
	char *column;
	unsigned int field;
	unsigned int tuple;

	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres = libpgsql.exec(pgconn, query);
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	sqr->NumFields=libpgsql.nfields(pgres);
	sqr->NumTuples=libpgsql.ntuples(pgres);
	/* k..  now we know how many tuples and fields, we can build our cursor */
	sqr->cursor=(char **)calloc(sqr->NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqr->NumTuples;tuple++) {
		sqr->cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqr->cursor[tuple]==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "Memory allocation error while creating SQL cursor tuple.");
			exit(-1);
		}
	}
	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		snprintf(column, MAX_FIELD_SIZE, "%s", libpgsql.fname(pgres, field));
		column+=strlen(column)+1;
	}
	/* now to populate the cursor */
	for (tuple=0;tuple<sqr->NumTuples;tuple++) {
		column=sqr->cursor[tuple];
		for (field=0;field<sqr->NumFields;field++) {
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
	return 0;
}
#endif /* HAVE_PGSQL */

#ifdef HAVE_SQLITE3
static int SQLiteDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite.open=(SQLITE_OPEN)dlsym(hinstLib, "sqlite3_open"))==NULL) goto fail;
	if ((libsqlite.exec=(SQLITE_EXEC)dlsym(hinstLib, "sqlite3_exec"))==NULL) goto fail;
	if ((libsqlite.close=(SQLITE_CLOSE)dlsym(hinstLib, "sqlite3_close"))==NULL) goto fail;
	return 0;
fail:
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libsqlite, 0, sizeof(libsqlite));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
}

static void sqliteDisconnect()
{
	if (sql_is_connected==0) return;
	libsqlite.close(db);
	sql_is_connected=0;
	return;
}

static int sqliteConnect()
{
	char dbname[255];
	char *zErrMsg=0;

	if (SQLiteDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/%s.db", proc.config.dir_var_db, SERVER_BASENAME);
	fixslashes(dbname);
	if (libsqlite.open(dbname, &db)!=SQLITE_OK) {
		printf("\nSQLite Connect error");
		return -1;
	}
	sql_is_connected=1;
	libsqlite.exec(db, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
	libsqlite.exec(db, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqliteUpdate(char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

retry:
	rc=libsqlite.exec(db, sqlquery, NULL, 0, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	if (rc!=SQLITE_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %s", zErrMsg);
		log_error("sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqliteCallback(void *vpsqr, int argc, char **argv, char **azColName)
{
	char *column;
	static unsigned int rowsalloc;
	unsigned int field;
	SQLRES *sqr=vpsqr;

	if (sqr->cursor==NULL) {
		sqr->NumFields=argc;
		/* retreive the field names */
		column=sqr->fields;
		for (field=0;field<sqr->NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", azColName[field]);
			column+=strlen(column)+1;
		}
		/* build our cursor and track the number of tuples */
		sqr->NumTuples=0;
		rowsalloc=50;
		if ((sqr->cursor=(char **)calloc(rowsalloc, sizeof(char *)))==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "Memory allocation error while creating SQL cursor.");
			exit(-1);
		}
	}
	/* now to populate the cursor */
	if (argv!=NULL) {
		if ((sqr->cursor[sqr->NumTuples]=calloc(MAX_TUPLE_SIZE, sizeof(char)))==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "Memory allocation error while creating SQL cursor tuple.");
			exit(-1);
		}
		column=sqr->cursor[sqr->NumTuples];
		for (field=0;field<sqr->NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", argv[field]?argv[field]:"NULL");
			column+=strlen(column)+1;
		}
		if (sqr->NumTuples+2>rowsalloc) {
			rowsalloc+=50;
			sqr->cursor=(char **)realloc(sqr->cursor, rowsalloc*sizeof(char *));
		}
		sqr->NumTuples++;
	}
	return 0;
}

static int sqliteQuery(SQLRES *sqr, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	sqr->NumFields=0;
	sqr->NumTuples=0;
retry:
	rc=libsqlite.exec(db, sqlquery, sqliteCallback, (void *)sqr, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
	log_error("sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE3 */

int sql_dll_unload()
{
	memset((char *)&libmysql, 0, sizeof(libmysql));
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	memset((char *)&libsqlite, 0, sizeof(libsqlite));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return 0;
}

void sql_disconnect()
{
	pthread_mutex_lock(&Lock.SQL);
	log_error("sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		sqliteDisconnect();
	}
	sql_dll_unload();
	pthread_mutex_unlock(&Lock.SQL);
	return;
}

void sql_unsafedisconnect()
{
	log_error("sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		sqliteDisconnect();
	}
	sql_dll_unload();
	return;
}

void sql_freeresult(SQLRES *sqr)
{
	unsigned int tuple;

	pthread_mutex_lock(&Lock.SQL);
	memset(sqr->fields, 0, sizeof(sqr->fields));
	if (sqr->cursor!=NULL) {
		for (tuple=0;tuple<sqr->NumTuples;tuple++) {
			if (sqr->cursor[tuple]!=NULL) {
				free(sqr->cursor[tuple]);
				sqr->cursor[tuple]=NULL;
			}
		}
		free(sqr->cursor);
		sqr->cursor=NULL;
	}
	sqr->NumFields=0;
	sqr->NumTuples=0;
	proc.stats.sql_handlecount--;
	log_error("sql", __FILE__, __LINE__, 4, "SQL query [%d] freed", sqr);
	pthread_mutex_unlock(&Lock.SQL);
	return;
}

int sql_update(char *sqlquery)
{
	int rc=-1;

	pthread_mutex_lock(&Lock.SQL);
	proc.stats.sql_updates++;
	log_error("sql", __FILE__, __LINE__, 2, "SQL update: %s", sqlquery);
	if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			return -1;
		}
		rc=mysqlUpdate(sqlquery);
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=odbcUpdate(sqlquery);
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=pgsqlUpdate(sqlquery);
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		if (sqliteConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=sqliteUpdate(sqlquery);
	}
	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_updatef(char *format, ...)
{
	unsigned char sqlquery[8192];
	va_list ap;

	memset(sqlquery, 0, sizeof(sqlquery));
	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	va_end(ap);
	return sql_update(sqlquery);
}

int sql_query(SQLRES *sqr, char *query)
{
	int rc=-1;

	pthread_mutex_lock(&Lock.SQL);
	memset((char *)sqr, 0, sizeof(SQLRES));
	log_error("sql", __FILE__, __LINE__, 3, "SQL query: %s", query);
	proc.stats.sql_queries++;
	if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=mysqlQuery(sqr, query))<0) {
			mysqlDisconnect();
			mysqlConnect();
		}
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=odbcQuery(sqr, query))<0) {
			odbcDisconnect();
			odbcConnect();
		}
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=pgsqlQuery(sqr, query))<0) {
			pgsqlDisconnect();
			pgsqlConnect();
		}
	} else if (strcmp(proc.config.sql_type, "SQLITE")==0) {
		if (sqliteConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=sqliteQuery(sqr, query))<0) {
			sqliteDisconnect();
			sqliteConnect();
		}
	}
	if (rc>-1) proc.stats.sql_handlecount++;
	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_queryf(SQLRES *sqr, char *format, ...)
{
	unsigned char sqlquery[8192];
	va_list ap;

	memset(sqlquery, 0, sizeof(sqlquery));
	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	va_end(ap);
	return sql_query(sqr, sqlquery);
}

char *sql_getname(SQLRES *sqr, int fieldnumber)
{
	char *column=NULL;
	int i;

	if ((fieldnumber<0)||(fieldnumber+1>(int)sqr->NumFields)) return NULL;
	column=sqr->fields;
	for (i=0;i<fieldnumber;i++) {
		column+=strlen(column)+1;
	}
	return column;
}

char *sql_getvalue(SQLRES *sqr, int tuple, int field)
{
	char *column=NULL;
	int i;

	if ((tuple<0)||(tuple+1>(int)sqr->NumTuples)) return NULL;
	if ((field<0)||(field+1>(int)sqr->NumFields)) return NULL;
	column=sqr->cursor[tuple];
	for (i=0;i<field;i++) {
		column+=strlen(column)+1;
	}
	return column;
}

char *sql_getvaluebyname(SQLRES *sqr, int tuple, char *fieldname)
{
	char *column=NULL;
	unsigned int i;

	if ((tuple<0)||(tuple+1>(int)sqr->NumTuples)) return NULL;
/*	if ((field<0)||(field+1>(int)sqr->NumFields)) return NULL; */
	column=sqr->cursor[tuple];
	for (i=0;i<sqr->NumFields;i++) {
		if (strcasecmp(fieldname, sql_getname(sqr, i))==0) {
			return column;
		} else {
			column+=strlen(column)+1;
		}
	}
	return NULL;
}

int sql_numfields(SQLRES *sqr)
{
	return sqr->NumFields;
}

int sql_numtuples(SQLRES *sqr)
{
	return sqr->NumTuples;
}
