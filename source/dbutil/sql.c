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
#ifdef WIN32
#pragma comment(lib, "libmysql.lib")
#endif
static MYSQL mysql;
static MYSQL *mysock=NULL;
static MYSQL_RES *myres=NULL;
#endif
/* PGSQL headers, libs, and global vars */
#ifdef HAVE_PGSQL
#include "libpq-fe.h"
#ifdef WIN32
#pragma comment(lib, "libpq.a")
#endif
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
#define MAX_TUPLE_SIZE 8192
#define MAX_FIELD_SIZE 1024
#define MAX_QUERIES 2
typedef struct {
	unsigned int ctime;
	unsigned int NumFields;
	unsigned int NumTuples;
	char fields[1024];
	char **cursor;
} sql_reply;
static sql_reply sqlreply[MAX_QUERIES];
static unsigned int Connected=0;

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
	if (pgres) {
		PQclear(pgres);
		pgres = PQexec(pgconn, "CLOSE myportal");
		PQclear(pgres);
		pgres = PQexec(pgconn, "END");
		PQclear(pgres);
		pgres=NULL;
	}
	if (pgconn) {
		PQfinish(pgconn);
		pgconn=NULL;
	}
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
	if (Connected) return 0;
	mysql_init(&mysql);
	if (!(mysock=mysql_real_connect(&mysql, sql.sql_hostname, sql.sql_username, sql.sql_password, sql.sql_dbname, atoi(sql.sql_port), NULL, 0))) {
		printf("\nMYSQL error: %s", mysql_error(&mysql));
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

	if (Connected) return 0;
	memset (port, 0, sizeof(port));
	snprintf(port, sizeof(port)-1, "%d", atoi(sql.sql_port));
	pgconn=PQsetdbLogin(sql.sql_hostname, port, NULL, NULL, sql.sql_dbname, sql.sql_username, sql.sql_password);
	if (PQstatus(pgconn)==CONNECTION_BAD) {
		printf("\nPGSQL Connect - %s", PQerrorMessage(pgconn));
		return -1;
	}
	pgres=PQexec(pgconn, "BEGIN");
	if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
		printf("\nPGSQL Connect - %s", PQerrorMessage(pgconn));
		return -1;
	}
	PQclear(pgres);
	pgres=NULL;
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
	if (mysql_query(mysock, sqlquery)) {
		if (verbose) printf("\nMYSQL error: %s", mysql_error(&mysql));
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
	pgres=PQexec(pgconn, sqlquery);
	if (PQresultStatus(pgres)!=PGRES_COMMAND_OK) {
		if (verbose) printf("\nPGSQL error: %s", PQerrorMessage(pgconn));
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
	MYSQL_FIELD *MYfield;
	MYSQL_ROW MYrow;
	char *column;
	unsigned int field;
	unsigned int tuple;

	if (mysql_query(mysock, sqlquery)) {
		printf("\nMYSQL error: %s", mysql_error(&mysql));
		printf("\nMYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=mysql_store_result(mysock))) {
		printf("\nMYSQL error: %s", mysql_error(&mysql));
		return -1;
	}
	sqlreply[sqr].NumFields=(int)mysql_num_fields(myres);
	sqlreply[sqr].NumTuples=(int)mysql_num_rows(myres);
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

	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres = PQexec(pgconn, query);
	if (PQresultStatus(pgres) != PGRES_COMMAND_OK) {
		printf("\nPGSQL Query - %s", PQerrorMessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	PQclear(pgres);
	pgres = PQexec(pgconn, "FETCH ALL IN myportal");
	if (PQresultStatus(pgres) != PGRES_TUPLES_OK) {
		printf("\nPGSQL Query - %s", PQerrorMessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	sqlreply[sqr].NumFields=PQnfields(pgres);
	sqlreply[sqr].NumTuples=PQntuples(pgres);
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
/*
int sqlConnect()
{
	if (strcmp(config.sql_type, "ODBC")==0) {
		return odbcConnect();
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
		return mysqlConnect();
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		return pgsqlConnect();
	}
	return -1;
}
*/

void sqlDisconnect()
{
//	logaccess(4, "SQL Disconnection by %s", request.username);
	if (strcmp(sql.sql_type, "ODBC")==0) {
		odbcDisconnect();
	} else if (strcmp(sql.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(sql.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
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

//	logaccess(2, "SQL update by %s: %s", request.username, sqlquery);
	if (strcmp(sql.sql_type, "ODBC")==0) {
		if (odbcConnect()<0) return -1;
		rc=odbcUpdate(verbose, sqlquery);
	} else if (strcmp(sql.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) return -1;
		rc=mysqlUpdate(verbose, sqlquery);
	} else if (strcmp(sql.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) return -1;
		rc=pgsqlUpdate(verbose, sqlquery);
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

//	logaccess(3, "SQL query by %s: %s", request.username, query);
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
