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

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

static void fixslashes(char *string)
{
 	while (*string) {
#ifdef WIN32
 		if (*string=='/') *string='\\';
#else
 		if (*string=='\\') *string='/';
#endif
		string++;
	}
}

#ifdef HAVE_MYSQL
static int mysqlDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", config.dir_lib, LIBEXT);
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
	if ((libmysql.store_result=(LIBMYSQL_STORE_RESULT)dlsym(hinstLib, "mysql_store_result"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
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
	if (!(mysock=libmysql.real_connect(&mysql, config.sql_hostname, config.sql_username, config.sql_password, config.sql_dbname, config.sql_port, NULL, 0))) {
		printf("MYSQL error: %s\r\n", libmysql.error(&mysql));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int mysqlUpdate(int verbose, char *sqlquery)
{
	if (libmysql.query(mysock, sqlquery)) {
		if (verbose) printf("MYSQL error: %s\r\n", libmysql.error(&mysql));
		if (verbose) printf("MYSQL: [%s]\r\n", sqlquery);
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
		printf("MYSQL error: %s\r\n", libmysql.error(&mysql));
		printf("MYSQL: [%s]\r\n", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.store_result(mysock))) {
		printf("MYSQL error: %s\r\n", libmysql.error(&mysql));
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
			printf("malloc() error creating SQL cursor tuple.\r\n");
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
		printf("ODBC Connect - Unable to allocate an environment handle.\r\n");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("ODBC Connect - SQLSetEnvAttr %s\r\n", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("ODBC Connect - SQLAllocHandle %s\r\n", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, config.sql_odbc_dsn, (short int)strlen(config.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("ODBC Connect - SQLDriverConnect %s\r\n", buf);
		odbcDisconnect();
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int odbcUpdate(int verbose, char *sqlquery)
{
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		if (verbose) printf("ODBC Update - SQLAllocHandle %s\r\n", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		if (verbose) printf("ODBC Update - SQLExecDirect %s\r\n", buf);
		if (verbose) printf("ODBC UPDATE: [%s]\r\n", sqlquery);
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
		printf("ODBC Query - SQLAllocHandle %s\r\n", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("ODBC Query - SQLExecDirect %s\r\n", buf);
		printf("ODBC SELECT: [%s]\r\n", sqlquery);
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("ODBC Query - SQLNumResultCols %s\r\n", buf);
		return -1;
	}
	sqr->NumFields=pccol;
	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		rc=SQLDescribeCol(hStmt, (SQLSMALLINT)(field+1), column, MAX_FIELD_SIZE, NULL, NULL, NULL, NULL, NULL);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			printf("ODBC Query - SQLDescribeCol %s\r\n", buf);
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
			printf("malloc() error creating SQL cursor tuple.\r\n");
			exit(-1);
		}
		column=sqr->cursor[tuple];
		for (field=0;field<sqr->NumFields;field++) {
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, MAX_FIELD_SIZE, &collen);
			if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
				SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
				printf("ODBC Getvalue - SQLGetData %s\r\n", buf);
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
	snprintf(libname, sizeof(libname)-1, "%s/libpq.%s", config.dir_lib, LIBEXT);
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
	printf("ERROR: Failed to load %s\r\n", libname);
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
	snprintf(port, sizeof(port)-1, "%d", config.sql_port);
	pgconn=libpgsql.setdblogin(config.sql_hostname, port, NULL, NULL, config.sql_dbname, config.sql_username, config.sql_password);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		printf("PGSQL Connect - %s\r\n", libpgsql.errormessage(pgconn));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int pgsqlUpdate(int verbose, char *sqlquery)
{
	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		if (verbose) printf("PGSQL Connect - %s\r\n", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		if (verbose) printf("PGSQL error: %s\r\n", libpgsql.errormessage(pgconn));
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
		printf("PGSQL Connect - %s\r\n", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres = libpgsql.exec(pgconn, query);
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		printf("PGSQL Query - %s\r\n", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		printf("PGSQL Query - %s\r\n", libpgsql.errormessage(pgconn));
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
			printf("Memory allocation error while creating SQL cursor tuple.\r\n");
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

#ifdef HAVE_SQLITE2
static int SQLite2DLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite2.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite2.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite2.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite2.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite2.open=(SQLITE2_OPEN)dlsym(hinstLib, "sqlite_open"))==NULL) goto fail;
	if ((libsqlite2.exec=(SQLITE2_EXEC)dlsym(hinstLib, "sqlite_exec"))==NULL) goto fail;
	if ((libsqlite2.close=(SQLITE2_CLOSE)dlsym(hinstLib, "sqlite_close"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
	if (hinstLib!=NULL) dlclose(hinstLib);
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

static int sqlite2Connect()
{
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite2DLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/%s.db", config.dir_var_db, SERVER_BASENAME);
	fixslashes(dbname);
	db2=libsqlite2.open(dbname, 0, &zErrMsg);
	if (db2==0) {
		printf("SQLite Connect - %s\r\n", zErrMsg);
		return -1;
	}
	sql_is_connected=1;
//	libsqlite2.exec(db2, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
//	libsqlite2.exec(db2, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	libsqlite2.exec(db2, "PRAGMA default_synchronous = OFF;", NULL, 0, &zErrMsg);
	libsqlite2.exec(db2, "PRAGMA synchronous = OFF;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqlite2Update(int verbose, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

retry:
	rc=libsqlite2.exec(db2, sqlquery, NULL, 0, &zErrMsg);
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
		if (verbose) printf("SQLite error: %s\r\n", zErrMsg);
		if (verbose) printf("SQLite: [%s]\r\n", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite2Callback(void *vpsqr, int argc, char **argv, char **azColName)
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
			printf("Memory allocation error while creating SQL cursor.\r\n");
			exit(-1);
		}
	}
	/* now to populate the cursor */
	if (argv!=NULL) {
		if ((sqr->cursor[sqr->NumTuples]=calloc(MAX_TUPLE_SIZE, sizeof(char)))==NULL) {
			printf("Memory allocation error while creating SQL cursor tuple.\r\n");
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

static int sqlite2Query(SQLRES *sqr, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	sqr->NumFields=0;
	sqr->NumTuples=0;
retry:
	rc=libsqlite2.exec(db2, sqlquery, sqlite2Callback, (void *)sqr, &zErrMsg);
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
	printf("SQLite error: %d %s\r\n", rc, zErrMsg);
	printf("SQLite: [%s]\r\n", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE2 */

#ifdef HAVE_SQLITE3
static int SQLite3DLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite3.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite3.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite3.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite3.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite3.open=(SQLITE3_OPEN)dlsym(hinstLib, "sqlite3_open"))==NULL) goto fail;
	if ((libsqlite3.exec=(SQLITE3_EXEC)dlsym(hinstLib, "sqlite3_exec"))==NULL) goto fail;
	if ((libsqlite3.close=(SQLITE3_CLOSE)dlsym(hinstLib, "sqlite3_close"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
	memset((char *)&libsqlite3, 0, sizeof(libsqlite3));
	if (hinstLib!=NULL) dlclose(hinstLib);
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

static int sqlite3Connect()
{
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite3DLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/%s.db", config.dir_var_db, SERVER_BASENAME);
	fixslashes(dbname);
	if (libsqlite3.open(dbname, &db3)!=SQLITE_OK) {
		printf("\nSQLite Connect error");
		return -1;
	}
	sql_is_connected=1;
//	libsqlite3.exec(db3, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
//	libsqlite3.exec(db3, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	libsqlite3.exec(db3, "PRAGMA default_synchronous = OFF;", NULL, 0, &zErrMsg);
	libsqlite3.exec(db3, "PRAGMA synchronous = OFF;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqlite3Update(int verbose, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

retry:
	rc=libsqlite3.exec(db3, sqlquery, NULL, 0, &zErrMsg);
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
		if (verbose) printf("SQLite error: %s\r\n", zErrMsg);
		if (verbose) printf("SQLite: [%s]\r\n", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite3Callback(void *vpsqr, int argc, char **argv, char **azColName)
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
			printf("Memory allocation error while creating SQL cursor.\r\n");
			exit(-1);
		}
	}
	/* now to populate the cursor */
	if (argv!=NULL) {
		if ((sqr->cursor[sqr->NumTuples]=calloc(MAX_TUPLE_SIZE, sizeof(char)))==NULL) {
			printf("Memory allocation error while creating SQL cursor tuple.\r\n");
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

static int sqlite3Query(SQLRES *sqr, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	sqr->NumFields=0;
	sqr->NumTuples=0;
retry:
	rc=libsqlite3.exec(db3, sqlquery, sqlite3Callback, (void *)sqr, &zErrMsg);
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
	printf("SQLite error: %d %s\r\n", rc, zErrMsg);
	printf("SQLite: [%s]\r\n", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE3 */

int sql_dll_unload()
{
	memset((char *)&libmysql, 0, sizeof(libmysql));
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
	memset((char *)&libsqlite3, 0, sizeof(libsqlite3));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return 0;
}

void sql_disconnect()
{
//	pthread_mutex_lock(&Lock.SQL);
	if (strcmp(config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(config.sql_type, "SQLITE2")==0) {
		sqlite2Disconnect();
	} else if ((strcmp(config.sql_type, "SQLITE3")==0)||(strcmp(config.sql_type, "SQLITE")==0)) {
		sqlite3Disconnect();
	}
	sql_dll_unload();
//	pthread_mutex_unlock(&Lock.SQL);
	return;
}
/*
void sql_unsafedisconnect()
{
	printf("SQL Disconnection\r\n");
	if (strcmp(config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(config.sql_type, "SQLITE2")==0) {
		sqlite2Disconnect();
	} else if ((strcmp(config.sql_type, "SQLITE3")==0)||(strcmp(config.sql_type, "SQLITE")==0)) {
		sqlite3Disconnect();
	}
	sql_dll_unload();
	return;
}
*/
void sql_freeresult(SQLRES *sqr)
{
	unsigned int tuple;

//	pthread_mutex_lock(&Lock.SQL);
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
//	printf("SQL query [%d] freed\r\n", sqr);
//	pthread_mutex_unlock(&Lock.SQL);
	return;
}

int sql_update(int verbose, char *sqlquery)
{
	int rc=-1;

//	pthread_mutex_lock(&Lock.SQL);
	if (strcmp(config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			return -1;
		}
		rc=mysqlUpdate(verbose, sqlquery);
	} else if (strcmp(config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=odbcUpdate(verbose, sqlquery);
#endif
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=pgsqlUpdate(verbose, sqlquery);
	} else if (strcmp(config.sql_type, "SQLITE2")==0) {
		if (sqlite2Connect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=sqlite2Update(verbose, sqlquery);
	} else if ((strcmp(config.sql_type, "SQLITE3")==0)||(strcmp(config.sql_type, "SQLITE")==0)) {
		if (sqlite3Connect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=sqlite3Update(verbose, sqlquery);
	}
//	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_updatef(int verbose, char *format, ...)
{
	unsigned char sqlquery[8192];
	va_list ap;

	memset(sqlquery, 0, sizeof(sqlquery));
	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	va_end(ap);
	return sql_update(verbose, sqlquery);
}

int sql_query(SQLRES *sqr, char *query)
{
	int rc=-1;

//	pthread_mutex_lock(&Lock.SQL);
	memset((char *)sqr, 0, sizeof(SQLRES));
	if (strcmp(config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=mysqlQuery(sqr, query))<0) {
			mysqlDisconnect();
			mysqlConnect();
		}
	} else if (strcmp(config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=odbcQuery(sqr, query))<0) {
			odbcDisconnect();
			odbcConnect();
		}
#endif
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=pgsqlQuery(sqr, query))<0) {
			pgsqlDisconnect();
			pgsqlConnect();
		}
	} else if (strcmp(config.sql_type, "SQLITE2")==0) {
		if (sqlite2Connect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=sqlite2Query(sqr, query))<0) {
			sqlite2Disconnect();
			sqlite2Connect();
		}
	} else if ((strcmp(config.sql_type, "SQLITE3")==0)||(strcmp(config.sql_type, "SQLITE")==0)) {
		if (sqlite3Connect()<0) {
//			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=sqlite3Query(sqr, query))<0) {
			sqlite3Disconnect();
			sqlite3Connect();
		}
	}
//	pthread_mutex_unlock(&Lock.SQL);
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

char *str2sql(char *instring)
{
	static unsigned char buffer[8192];
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

int sqlfprintf(FILE *fp, const char *format, ...)
{
	unsigned char buffer[8192];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	while (buffer[offset]) {
		if (buffer[offset]==13) {
//			fprintf(fp, "\\r");
		} else if (buffer[offset]==10) {
			fprintf(fp, "\\n");
		} else if (buffer[offset]=='\'') {
			fprintf(fp, "''");
		} else {
			fprintf(fp, "%c", buffer[offset]);
		}
		offset++;
	}
	return 0;
}
