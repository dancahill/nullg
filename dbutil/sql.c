/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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

/****************************************************************************
 *	*DLLInit()
 *
 *	Purpose	: MySQL specific function to disconnect from the SQL server
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
int mysqlDLLInit()
{
#ifdef HAVE_MYSQL
	static int isloaded=0;
#ifdef WIN32
	HINSTANCE hinstLib;
#else
	void *hinstLib;
#endif

	if (isloaded) return 0;
#ifdef WIN32
	if ((hinstLib=dlopen("libmysql", RTLD_NOW))==NULL) goto fail;
#else
	if ((hinstLib=dlopen("libmysqlclient.so", RTLD_NOW))==NULL) goto fail;
#endif
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
	isloaded=1;
	return 0;
fail:
	printf("\r\nERROR: Failed to load libmysql\r\n");
	memset((char *)&libmysql, 0, sizeof(libmysql));
	if (hinstLib!=NULL) dlclose(hinstLib);
	return -1;
#else
	return -1;
#endif
}

int odbcDLLInit()
{
#ifdef HAVE_ODBC
	static int isloaded=0;

	if (isloaded) return 0;
	return 0;
#else
	return -1;
#endif
}

int pgsqlDLLInit()
{
#ifdef HAVE_PGSQL
	static int isloaded=0;
#ifdef WIN32
	HINSTANCE hinstLib;
#else
	void *hinstLib;
#endif

	if (isloaded) return 0;
#ifdef WIN32
	if ((hinstLib=dlopen("libpq", RTLD_NOW))==NULL) goto fail;
#else
	if ((hinstLib=dlopen("libpq.so", RTLD_NOW))==NULL) goto fail;
#endif
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
	isloaded=1;
	return 0;
fail:
	printf("\r\nERROR: Failed to load libpq\r\n");
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	if (hinstLib!=NULL) dlclose(hinstLib);
	return -1;
#else
	return -1;
#endif
}

int SQLiteDLLInit()
{
	char libname[255];
#ifdef HAVE_SQLITE
	static int isloaded=0;
#ifdef WIN32
	HINSTANCE hinstLib;
#else
	void *hinstLib;
#endif

	if (isloaded) return 0;
	memset(libname, 0, sizeof(libname));
#ifdef WIN32
	snprintf(libname, sizeof(libname)-1, "..\\lib\\sqlite.dll");
//	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite.dll");
//	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
#else
	snprintf(libname, sizeof(libname)-1, "../lib/sqlite.so");
//	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite.so");
//	fixslashes(libname);
	if ((hinstLib=dlopen(libname, RTLD_NOW))!=NULL) goto found;
	goto fail;
#endif
found:
	if ((libsqlite.open=(SQLITE_OPEN)dlsym(hinstLib, "sqlite_open"))==NULL) goto fail;
	if ((libsqlite.exec=(SQLITE_EXEC)dlsym(hinstLib, "sqlite_exec"))==NULL) goto fail;
	if ((libsqlite.close=(SQLITE_CLOSE)dlsym(hinstLib, "sqlite_close"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s", libname);
	memset((char *)&libsqlite, 0, sizeof(libsqlite));
	if (hinstLib!=NULL) dlclose(hinstLib);
	hinstLib=NULL;
	return -1;
#else
	return -1;
#endif
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
	Connected=0;
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
	Connected=0;
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
	Connected=0;
#endif
	return;
}

void sqliteDisconnect()
{
#ifdef HAVE_SQLITE
	libsqlite.close(db);
	Connected=0;
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
int mysqlConnect()
{
#ifdef HAVE_MYSQL
	if(mysqlDLLInit()!=0) return -1;
	if (Connected) return 0;
	libmysql.init(&mysql);
	if (!(mysock=libmysql.real_connect(&mysql, sql.sql_hostname, sql.sql_username, sql.sql_password, sql.sql_dbname, atoi(sql.sql_port), NULL, 0))) {
		printf("\nMYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	Connected=1;
	return 0;
#else
	return -1;
#endif
}

int odbcConnect()
{
#ifdef HAVE_ODBC
	SQLCHAR szConnStr[255];
	SWORD cbConnStr;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];

	if (Connected) return 0;
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)==SQL_ERROR) {
		printf("\nODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, sql.sql_odbc_dsn, (short int)strlen(sql.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	Connected=1;
	return 0;
#else
	return -1;
#endif
}

int pgsqlConnect()
{
#ifdef HAVE_PGSQL
	char port[12];

	if(pgsqlDLLInit()!=0) return -1;
	if (Connected) return 0;
	memset (port, 0, sizeof(port));
	snprintf(port, sizeof(port)-1, "%d", atoi(sql.sql_port));
	pgconn=libpgsql.setdblogin(sql.sql_hostname, port, NULL, NULL, sql.sql_dbname, sql.sql_username, sql.sql_password);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		printf("\nPGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=NULL;
	Connected=1;
	return 0;
#else
	return -1;
#endif
}

int sqliteConnect()
{
#ifdef HAVE_SQLITE
	char *zErrMsg=0;

	if(SQLiteDLLInit()!=0) return -1;
	if (Connected) return 0;
	db=libsqlite.open("../var/db/groupware.db", 0, &zErrMsg);
	if (db==0) {
		printf("\nSQLite Connect - %s", zErrMsg);
		return -1;
	}
	Connected=1;
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
int mysqlUpdate(int verbose, char *sqlquery)
{
#ifdef HAVE_MYSQL
	if (libmysql.query(mysock, sqlquery)) {
		if (verbose) printf("\nMYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	return 0;
#else
	return -1;
#endif
}

int odbcUpdate(int verbose, char *sqlquery)
{
#ifdef HAVE_ODBC
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		if (verbose) printf("\nODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		if (verbose) printf("\nODBC Update - SQLExecDirect %s", buf);
		if (verbose) printf("\nODBC UPDATE: [%s]", sqlquery);
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

int pgsqlUpdate(int verbose, char *sqlquery)
{
#ifdef HAVE_PGSQL
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		if (verbose) printf("\nPGSQL error: %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=NULL;
	return 0;
#else
	return -1;
#endif
}

int sqliteUpdate(int verbose, char *sqlquery)
{
#ifdef HAVE_SQLITE
	char *zErrMsg=0;
	int rc;

	rc=libsqlite.exec(db, sqlquery, NULL, 0, &zErrMsg);
	if (rc!=SQLITE_OK) {
		if (verbose) printf("\nSQL error: %s", zErrMsg);
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
int mysqlQuery(int sqr, char *sqlquery)
{
#ifdef HAVE_MYSQL
	MYSQL_FIELD *MYfield;
	MYSQL_ROW MYrow;
	char *column;
	unsigned int field;
	unsigned int tuple;

	if (libmysql.query(mysock, sqlquery)) {
		printf("\nMYSQL error: %s", libmysql.error(&mysql));
		printf("\nMYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.store_result(mysock))) {
		printf("\nMYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sqlreply[sqr].NumFields=(int)libmysql.num_fields(myres);
	sqlreply[sqr].NumTuples=(int)libmysql.num_rows(myres);
	// k..  now we know how many tuples and fields, we can build our static cursor
	sqlreply[sqr].cursor=(char **)calloc(sqlreply[sqr].NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			printf("\nMemory allocation error while creating SQL cursor tuple.");
			exit(0);
		}
	}
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		MYfield=libmysql.fetch_field_direct(myres, field);
		snprintf(column, MAX_FIELD_SIZE, "%s", MYfield->name);
		column+=strlen(column)+1;
	}
	// now to populate the cursor
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			libmysql.data_seek(myres, tuple);
			MYrow=libmysql.fetch_row(myres);
			snprintf(column, MAX_FIELD_SIZE, "%s", MYrow[field]);
			column+=strlen(column)+1;
		}
	}
	if (myres!=NULL) {
		libmysql.free_result(myres);
		myres=NULL;
	}
	return sqr;
#else
	return -1;
#endif
}

int odbcQuery(int sqr, char *sqlquery)
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

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Query - SQLExecDirect %s", buf);
		printf("\nODBC SELECT: [%s]", sqlquery);
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	sqlreply[sqr].NumFields=pccol;
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		rc=SQLDescribeCol(hStmt, (SQLSMALLINT)(field+1), column, MAX_FIELD_SIZE, NULL, NULL, NULL, NULL, NULL);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			printf("\nODBC Query - SQLDescribeCol %s", buf);
			return -1;
		}
		column+=strlen(column)+1;
	}
	// now to figure out how many tuples we have
	while (1) {
		rc=SQLFetch(hStmt);
		if ((rc==SQL_SUCCESS)||(rc==SQL_SUCCESS_WITH_INFO)) {
			sqlreply[sqr].NumTuples++;
		} else {
			break;
		}
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	// k..  now we know how many tuples and fields, we can build our static cursor
	sqlreply[sqr].cursor=(char **)calloc(sqlreply[sqr].NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			printf("\nMemory allocation error while creating SQL cursor tuple.");
			exit(0);
		}
	}
	if (SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt)==SQL_ERROR) {
		printf("\nODBC Getvalue - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		printf("\nODBC Getvalue - SQLExecDirect() %s", buf);
		return -1;
	}
	// now to populate the cursor
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		rc=SQLFetch(hStmt);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			printf("\nODBC Getvalue - SQLFetch %s", buf);
			return -1;
		}
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, MAX_FIELD_SIZE, &collen);
			if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
				SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
				printf("\nODBC Getvalue - SQLGetData %s", buf);
				return -1;
			}
			column+=strlen(column)+1;
		}
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return sqr;
#else
	return -1;
#endif
}

int pgsqlQuery(int sqr, char *sqlquery)
{
#ifdef HAVE_PGSQL
	char query[8192];
	char *column;
	unsigned int field;
	unsigned int tuple;

	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		printf("\nPGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres=libpgsql.exec(pgconn, query);
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		printf("\nPGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		printf("\nPGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	sqlreply[sqr].NumFields=libpgsql.nfields(pgres);
	sqlreply[sqr].NumTuples=libpgsql.ntuples(pgres);
	// k..  now we know how many tuples and fields, we can build our static cursor
	sqlreply[sqr].cursor=(char **)calloc(sqlreply[sqr].NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			printf("\nMemory allocation error while creating SQL cursor tuple.");
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
//	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, "CLOSE myportal");
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);
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

	if (sqlreply[sqr].cursor==NULL) {
		sqlreply[sqr].NumFields=argc;
		// retreive the field names
		column=sqlreply[sqr].fields;
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", azColName[field]);
			column+=strlen(column)+1;
		}
		// build our cursor and track the number of tuples
		sqlreply[sqr].NumTuples=0;
		rowsalloc=50;
		sqlreply[sqr].cursor=(char **)calloc(rowsalloc, sizeof(char *));
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

int sqliteQuery(int sqr, char *sqlquery)
{
#ifdef HAVE_SQLITE
	char *zErrMsg=0;
	int rc;

	rc=libsqlite.exec(db, sqlquery, sqliteCallback, (void *)sqr, &zErrMsg);
	if (rc!=SQLITE_OK) {
		printf("SQL error: %s\n", zErrMsg);
		return -1;
	}
	return sqr;
#else
	return -1;
#endif
}

void sqlDisconnect()
{
	if (strcmp(sql.sql_type, "ODBC")==0) {
		odbcDisconnect();
	} else if (strcmp(sql.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(sql.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(sql.sql_type, "SQLITE")==0) {
		sqliteDisconnect();
	}
	return;
}

void sqlFreeconnect(int sqr)
{
	unsigned int tuple;

	memset(sqlreply[sqr].fields, 0, sizeof(sqlreply[sqr].fields));
	if (sqlreply[sqr].cursor!=NULL) {
		for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
			free(sqlreply[sqr].cursor[tuple]);
		}
		free(sqlreply[sqr].cursor);
		sqlreply[sqr].cursor=NULL;
	}
	sqlreply[sqr].NumFields=0;
	sqlreply[sqr].NumTuples=0;
	return;
}

int sqlUpdate(int verbose, char *sqlquery)
{
	int rc=-1;

	if (strcmp(sql.sql_type, "ODBC")==0) {
		if (odbcConnect()<0) return -1;
		rc=odbcUpdate(verbose, sqlquery);
	} else if (strcmp(sql.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) return -1;
		rc=mysqlUpdate(verbose, sqlquery);
	} else if (strcmp(sql.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) return -1;
		rc=pgsqlUpdate(verbose, sqlquery);
	} else if (strcmp(sql.sql_type, "SQLITE")==0) {
		if (sqliteConnect()<0) return -1;
		rc=sqliteUpdate(verbose, sqlquery);
	}
	return rc;
}

int sqlUpdatef(int verbose, char *format, ...)
{
	unsigned char sqlquery[8192];
	int rc=-1;
	va_list ap;

	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	return sqlUpdate(verbose, sqlquery);
	return rc;
}

int sqlQuery(char *query)
{
	int i;
	int rc=-1;

	for (i=0;i<MAX_QUERIES;i++) {
		if (sqlreply[i].cursor==NULL) break;
	}
	if (i==MAX_QUERIES) {
		printf("\ndropping sql connection - max %d", MAX_QUERIES);
		return -1;
	}
	if (strcmp(sql.sql_type, "ODBC")==0) {
		if (odbcConnect()<0) return -1;
		rc=odbcQuery(i, query);
	} else if (strcmp(sql.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) return -1;
		rc=mysqlQuery(i, query);
	} else if (strcmp(sql.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) return -1;
		rc=pgsqlQuery(i, query);
	} else if (strcmp(sql.sql_type, "SQLITE")==0) {
		if (sqliteConnect()<0) return -1;
		rc=sqliteQuery(i, query);
	}
	return rc;
}

int sqlQueryf(char *format, ...)
{
	unsigned char sqlquery[8192];
	va_list ap;

	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	return sqlQuery(sqlquery);
}

char *sqlGetfieldname(int sqr, int fieldnumber)
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

char *sqlGetvalue(int sqr, int tuple, int field)
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

int sqlNumfields(int sqr)
{
	return sqlreply[sqr].NumFields;
}

int sqlNumtuples(int sqr)
{
	return sqlreply[sqr].NumTuples;
}

char *str2sql(char *instring)
{
	static unsigned char buffer[8192];
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	memset(buffer, 0, sizeof(buffer));
	while ((instring[i])&&(i<sizeof(buffer)-1)) {
		ch=instring[i];
		if (ch==0) break;
		if ((ch<32)||(ch>255)) { i++; continue; }
		if (ch=='\'') {
			if (strcmp(sql.sql_type, "ODBC")==0) {
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
