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
#include "main.h"
/* MySQL headers, libs, and global vars */
#ifdef HAVE_MYSQL
#include "mysql.h"
static MYSQL mysql;
static MYSQL *mysock=NULL;
#endif
/* PGSQL headers, libs, and global vars */
#ifdef HAVE_PGSQL
#include "libpq-fe.h"
static PGconn *pgconn=NULL;
static PGresult *pgres=NULL;
#endif
/* WinODBC headers, libs, and global vars */
#ifdef HAVE_ODBC
#include <sql.h>
#include <sqlext.h>
static SQLHENV hEnv=NULL;
static SQLHDBC hDBC=NULL;
static SQLHSTMT hStmt=NULL;
#endif
/* shared vars */
static unsigned int sqlIsConnected=0;

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
		mysql_close(mysock);
		mysock=NULL;
	}
	sqlIsConnected=0;
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
	sqlIsConnected=0;
#endif
	return;
}

void pgsqlDisconnect()
{
#ifdef HAVE_PGSQL
	if (pgconn) {
		PQfinish(pgconn);
		pgconn=NULL;
	}
	sqlIsConnected=0;
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
	if (sqlIsConnected) return 0;
	mysql_init(&mysql);
	if (!(mysock=mysql_real_connect(&mysql, config.sql_hostname, config.sql_username, config.sql_password, config.sql_dbname, config.sql_port, NULL, 0))) {
		logerror("MYSQL error: %s", mysql_error(&mysql));
		return -1;
	}
	sqlIsConnected=1;
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

	if (sqlIsConnected) return 0;
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv)==SQL_ERROR) {
		logerror("ODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, config.sql_odbc_dsn, (short int)strlen(config.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	sqlIsConnected=1;
	return 0;
#else
	return -1;
#endif
}

int pgsqlConnect()
{
#ifdef HAVE_PGSQL
	char port[12];

	if (sqlIsConnected) return 0;
	memset (port, 0, sizeof(port));
	snprintf(port, sizeof(port)-1, "%d", config.sql_port);
	pgconn=PQsetdbLogin(config.sql_hostname, port, NULL, NULL, config.sql_dbname, config.sql_username, config.sql_password);
	if (PQstatus(pgconn)==CONNECTION_BAD) {
		logerror("PGSQL Connect - %s", PQerrorMessage(pgconn));
		return -1;
	}
	sqlIsConnected=1;
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
int mysqlUpdate(char *sqlquery)
{
#ifdef HAVE_MYSQL
	if (mysql_query(mysock, sqlquery)) {
		logerror("MYSQL error: %s", mysql_error(&mysql));
		return -1;
	}
	return 0;
#else
	return -1;
#endif
}

int odbcUpdate(char *sqlquery)
{
#ifdef HAVE_ODBC
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Update - SQLExecDirect %s", buf);
		logerror("ODBC UPDATE: [%s]", sqlquery);
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

int pgsqlUpdate(char *sqlquery)
{
#ifdef HAVE_PGSQL
	pgres=PQexec(pgconn, "BEGIN");
	if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
		logerror("PGSQL Connect - %s", PQerrorMessage(pgconn));
		return -1;
	}
	PQclear(pgres);
	pgres=PQexec(pgconn, sqlquery);
	if (PQresultStatus(pgres)!=PGRES_COMMAND_OK) {
		logerror("PGSQL error: %s", PQerrorMessage(pgconn));
		return -1;
	}
	PQclear(pgres);
	pgres = PQexec(pgconn, "END");
	PQclear(pgres);
	pgres=NULL;
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
	MYSQL_RES *myres;
	MYSQL_FIELD *MYfield;
	MYSQL_ROW MYrow;
	char *column;
	unsigned int field;
	unsigned int tuple;

	if (mysql_ping(mysock)!=0) {
		if (mysqlConnect()<0) return -1;
	}
	if (mysql_query(mysock, sqlquery)) {
		logerror("MYSQL error: %s", mysql_error(&mysql));
		logerror("MYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=mysql_store_result(mysock))) {
		logerror("MYSQL error: %s", mysql_error(&mysql));
		return -1;
	}
	sqlreply[sqr].NumFields=(int)mysql_num_fields(myres);
	sqlreply[sqr].NumTuples=(int)mysql_num_rows(myres);
	// k..  now we know how many tuples and fields, we can build our cursor
	sqlreply[sqr].cursor=(char **)calloc(sqlreply[sqr].NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			logerror("Memory allocation error while creating SQL cursor tuple.");
//			closeconnect(1);
			exit(0);
		}
	}
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		MYfield=mysql_fetch_field_direct(myres, field);
		snprintf(column, MAX_FIELD_SIZE, "%s", MYfield->name);
		column+=strlen(column)+1;
	}
	// now to populate the cursor
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			mysql_data_seek(myres, tuple);
			MYrow=mysql_fetch_row(myres);
			snprintf(column, MAX_FIELD_SIZE, "%s", MYrow[field]);
			column+=strlen(column)+1;
		}
	}
	if (myres!=NULL) {
		mysql_free_result(myres);
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
	unsigned int rowsalloc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Query - SQLExecDirect %s", buf);
		logerror("ODBC SELECT: [%s]", sqlquery);
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		logerror("ODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	sqlreply[sqr].NumFields=pccol;
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		rc=SQLDescribeCol(hStmt, (SQLSMALLINT)(field+1), column, MAX_FIELD_SIZE, NULL, NULL, NULL, NULL, NULL);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			logerror("ODBC Query - SQLDescribeCol %s", buf);
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
			logerror("malloc() error creating SQL cursor tuple.");
			exit(0);
		}
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, MAX_FIELD_SIZE, &collen);
			if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
				SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
				logerror("ODBC Getvalue - SQLGetData %s", buf);
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

int pgsqlQuery(int sqr, char *sqlquery)
{
#ifdef HAVE_PGSQL
	char query[8192];
	char *column;
	unsigned int field;
	unsigned int tuple;

	pgres=PQexec(pgconn, "BEGIN");
	if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
		logerror("PGSQL Connect - %s", PQerrorMessage(pgconn));
		return -1;
	}
	PQclear(pgres);
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres = PQexec(pgconn, query);
	if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
		logerror("PGSQL Query - %s", PQerrorMessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	PQclear(pgres);
	pgres = PQexec(pgconn, "FETCH ALL IN myportal");
	if (PQresultStatus(pgres) != PGRES_TUPLES_OK) {
		logerror("PGSQL Query - %s", PQerrorMessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	sqlreply[sqr].NumFields=PQnfields(pgres);
	sqlreply[sqr].NumTuples=PQntuples(pgres);
	// k..  now we know how many tuples and fields, we can build our cursor
	sqlreply[sqr].cursor=(char **)calloc(sqlreply[sqr].NumTuples, sizeof(char *));
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		sqlreply[sqr].cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqlreply[sqr].cursor[tuple]==NULL) {
			logerror("Memory allocation error while creating SQL cursor tuple.");
//			closeconnect(1);
			exit(0);
		}
	}
	// retreive the field names
	column=sqlreply[sqr].fields;
	for (field=0;field<sqlreply[sqr].NumFields;field++) {
		snprintf(column, MAX_FIELD_SIZE, "%s", PQfname(pgres, field));
		column+=strlen(column)+1;
	}
	// now to populate the cursor
	for (tuple=0;tuple<sqlreply[sqr].NumTuples;tuple++) {
		column=sqlreply[sqr].cursor[tuple];
		for (field=0;field<sqlreply[sqr].NumFields;field++) {
			snprintf(column, MAX_FIELD_SIZE, "%s", PQgetvalue(pgres, tuple, field));
			column+=strlen(column)+1;
		}
	}
	if (pgres) {
		PQclear(pgres);
		pgres = PQexec(pgconn, "CLOSE myportal");
		PQclear(pgres);
		pgres = PQexec(pgconn, "END");
		PQclear(pgres);
		pgres=NULL;
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
void sqlDisconnect()
{
	if (!RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	logaccess(4, "SQL Disconnection");
	if (strcmp(config.sql_type, "ODBC")==0) {
		odbcDisconnect();
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	}
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return;
}

void sqlFreeconnect(int sqr)
{
	unsigned int tuple;

	if (!RunAsCGI) pthread_mutex_lock(&Lock.SQL);
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
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return;
}

int sqlUpdate(char *sqlquery)
{
	int rc=-1;
	int sid=getsid();

	if (!RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	if (sid==-1) {
		logaccess(2, "SQL update by system: %s", sqlquery);
	} else {
		logaccess(2, "SQL update by %s: %s", conn[sid].dat->in_username, sqlquery);
	}
	if (strcmp(config.sql_type, "ODBC")==0) {
		if (odbcConnect()<0) {
			if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=odbcUpdate(sqlquery);
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			return -1;
		}
		rc=mysqlUpdate(sqlquery);
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
			if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=pgsqlUpdate(sqlquery);
	}
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sqlUpdatef(char *format, ...)
{
	unsigned char sqlquery[8192];
	int rc=-1;
	va_list ap;

	va_start(ap, format);
	vsnprintf(sqlquery, sizeof(sqlquery)-1, format, ap);
	return sqlUpdate(sqlquery);
	return rc;
}

int sqlQuery(char *query)
{
	int i;
	int rc=-1;
	int sid=getsid();

	if (!RunAsCGI) pthread_mutex_lock(&Lock.SQL);
	if (sid	==-1) {
		logaccess(3, "SQL query by system: %s", query);
	} else {
		logaccess(3, "SQL query by %s: %s", conn[sid].dat->in_username, query);
	}
	for (i=0;;i++) {
		if (i>=config.sql_maxconn) {
			sleep(1);
			i=0;
			continue;
		}
		if (sqlreply[i].cursor==NULL) break;
	}
	if (strcmp(config.sql_type, "ODBC")==0) {
		if (odbcConnect()<0) {
			if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=odbcQuery(i, query);
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=mysqlQuery(i, query);
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
			if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=pgsqlQuery(i, query);
	}
	if (!RunAsCGI) pthread_mutex_unlock(&Lock.SQL);
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
	int sid=getsid();
	char *buffer=conn[sid].dat->sqlbuf;
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	memset(conn[sid].dat->sqlbuf, 0, sizeof(conn[sid].dat->sqlbuf));
	while ((instring[i])&&(i<sizeof(conn[sid].dat->sqlbuf)-1)) {
		ch=instring[i];
		if (ch==0) break;
		if (ch=='\'') {
			if (strcmp(config.sql_type, "ODBC")==0) {
				buffer[bufferlength]='\'';
			} else if (strcmp(config.sql_type, "MYSQL")==0) {
				buffer[bufferlength]='\'';
			} else {
				buffer[bufferlength]='\'';
//				buffer[bufferlength]='\\';
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

char *field(int numfields, char *fieldname)
{
	int sid=getsid();
	int i;
	
	for (i=0;i<numfields;i++) {
		if (strcmp(conn[sid].dat->tuple[i].name, fieldname)==0) {
			return conn[sid].dat->tuple[i].value;
		}
	}
	return "Missing Field";
}
