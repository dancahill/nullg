/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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
#include "nsp/nsplib.h"
#include "libodbc.h"

#ifdef HAVE_ODBC

#ifdef WIN32
#  pragma comment(lib, "odbc32.lib")
#endif
#include <sql.h>
#include <sqlext.h>

void odbc_murder(nsp_state *N, obj_t *cobj);

typedef struct ODBC_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's socket-specific */
	SQLHENV  hENV;
	SQLHDBC  hDBC;
	SQLHSTMT hSTMT;

	unsigned long numfields;
	unsigned long numtuples;
} ODBC_CONN;

static ODBC_CONN *getconn(nsp_state *N)
{
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	ODBC_CONN *conn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "odbc-conn") != 0))
		n_error(N, NE_SYNTAX, "getconn", "expected a odbc-conn");
	conn = (ODBC_CONN *)cobj->val->d.str;
	return conn;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void odbcDisconnect(nsp_state *N, ODBC_CONN *conn)
{
	SQLDisconnect(conn->hDBC);
	if (conn->hDBC) {
		SQLFreeHandle(SQL_HANDLE_DBC, conn->hDBC);
		conn->hDBC = NULL;
	}
	if (conn->hENV) {
		SQLFreeHandle(SQL_HANDLE_ENV, conn->hENV);
		conn->hENV = NULL;
	}
	return;
}

static int odbcConnect(nsp_state *N, ODBC_CONN *conn, char *dsn)
{
#define __FN__ __FILE__ ":odbcConnect()"
	RETCODE rc;
	char sqlstate[15];
	char buf[250];

	rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &conn->hENV);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		n_warn(N, __FN__, "Unable to allocate an environment handle.");
		return -1;
	}
	rc = SQLSetEnvAttr(conn->hENV, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
		n_warn(N, __FN__, "SQLSetEnvAttr %s", buf);
		odbcDisconnect(N, conn);
		return -1;
	}
	rc = SQLAllocHandle(SQL_HANDLE_DBC, conn->hENV, &conn->hDBC);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
		n_warn(N, __FN__, "SQLAllocHandle %s", buf);
		odbcDisconnect(N, conn);
		return -1;
	}
	rc = SQLDriverConnect(conn->hDBC, NULL, (SQLPOINTER)dsn, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
		n_warn(N, __FN__, "SQLDriverConnect %s", buf);
		odbcDisconnect(N, conn);
		return -1;
	}
	return 0;
#undef __FN__
}

void odbc_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":odbc_murder()"
	ODBC_CONN *conn;

	//	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "odbc-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected an odbc-conn");
	conn = (ODBC_CONN *)cobj->val->d.str;
	odbcDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(ODBC_CONN) + 1);
	cobj->val->size = 0;
	return;
#undef __FN__
}


NSP_CLASSMETHOD(libnsp_data_odbc_open)
{
#define __FN__ __FILE__ ":libnsp_data_odbc_open()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	ODBC_CONN *conn;
	RETCODE rc;
	char *DSN = NULL;
	char *sqlquery = NULL;

	if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "dsn")))) {
		DSN = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for DSN");
	}
	conn = n_alloc(N, sizeof(ODBC_CONN) + 1, 1);
	strcpy(conn->obj_type, "odbc-conn");
	conn->obj_term = (NSP_CFREE)odbc_murder;
	rc = odbcConnect(N, conn, DSN);
	if (rc < 0) {
		nsp_setstr(N, &N->r, "", "odbc connection error", -1);
		n_free(N, (void *)&conn, sizeof(ODBC_CONN) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
	cobj->val->d.str = (void *)conn;
	cobj->val->size = sizeof(ODBC_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_odbc_close)
{
#define __FN__ __FILE__ ":libnsp_data_odbc_close()"
	ODBC_CONN *conn = getconn(N);
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "odbc-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a odbc-conn");
	conn = (ODBC_CONN *)cobj->val->d.str;
	if (conn->hSTMT != NULL) {
		SQLFreeHandle(SQL_HANDLE_STMT, conn->hSTMT);
		conn->hSTMT = NULL;
	}
	odbcDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(ODBC_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "connection", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_odbc_query)
{
#define __FN__ __FILE__ ":libnsp_data_odbc_query()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	ODBC_CONN *conn = getconn(N);
	SQLSMALLINT pccol;
	SQLINTEGER rowcount;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];
	char *DSN = NULL;
	char *sqlquery = NULL;
	short expect_results = 1;

	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "1")))) {
		sqlquery = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "sqlquery")))) {
		sqlquery = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for sqlquery");
	}
	if (nsp_isbool((cobj = nsp_getobj(N, &N->l, "2")))) {
		expect_results = nsp_tobool(N, cobj);
	}
	if (conn->hSTMT == NULL) {
		rc = SQLAllocHandle(SQL_HANDLE_STMT, conn->hDBC, &conn->hSTMT);
		if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
			SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
			n_warn(N, __FN__, "SQLAllocHandle %s", buf);
			return -1;
		}
	}
	rc = SQLExecDirect(conn->hSTMT, (SQLPOINTER)sqlquery, SQL_NTS);
	if (rc == SQL_NO_DATA) {
		n_warn(N, __FN__, "SQLExecDirect SQL_NO_DATA [%s]", sqlquery);
		conn->numfields = 0;
		conn->numtuples = 0;
		cobj = nsp_settable(N, thisobj, "last_query");
		nsp_setstr(N, cobj, "_query", sqlquery, -1);
		nsp_setnum(N, cobj, "_fields", conn->numfields);
		cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
		cobj->val->d.str = (void *)conn;
		cobj->val->size = sizeof(ODBC_CONN) + 1;
		return 0;
	}
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
		n_warn(N, __FN__, "SQLExecDirect %s", buf);
		n_warn(N, __FN__, "[%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, conn->hSTMT);
		conn->hSTMT = NULL;
		return -1;
	}
	rc = SQLRowCount(conn->hSTMT, &rowcount);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
		n_warn(N, __FN__, "SQLRowCount %s", buf);
		return -1;
	}
	nsp_setnum(N, thisobj, "changes", rowcount);
	rc = SQLNumResultCols(conn->hSTMT, &pccol);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
		SQLError(conn->hENV, conn->hDBC, conn->hSTMT, (SQLPOINTER)sqlstate, NULL, (SQLPOINTER)buf, sizeof(buf), NULL);
		n_warn(N, __FN__, "SQLNumResultCols %s", buf);
		return -1;
	}
	if (!expect_results) {
		if (conn->hSTMT != NULL) {
			SQLFreeHandle(SQL_HANDLE_STMT, conn->hSTMT);
			conn->hSTMT = NULL;
		}
	}
	conn->numfields = pccol;
	conn->numtuples = 0;
	cobj = nsp_settable(N, thisobj, "last_query");
	nsp_setstr(N, cobj, "_query", sqlquery, -1);
	nsp_setnum(N, cobj, "_fields", conn->numfields);
	return 0;
#undef __FN__
}

#define FBUFSIZE 1024

NSP_CLASSMETHOD(libnsp_data_odbc_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_odbc_getnext()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t tobj;
	ODBC_CONN *conn;
	char fbuf[FBUFSIZE];
	char colname[MAX_OBJNAMELEN + 1];
	RETCODE rc;
	SDWORD slen;
	unsigned long field;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "odbc-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected an odbc-conn");
	conn = (ODBC_CONN *)cobj->val->d.str;
	rc = SQLFetch(conn->hSTMT);
	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) return 0;
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	for (field = 0;field < conn->numfields;field++) {
		rc = SQLDescribeCol(conn->hSTMT, (SQLSMALLINT)(field + 1), (SQLPOINTER)colname, MAX_OBJNAMELEN, NULL, NULL, NULL, NULL, NULL);
		cobj = nsp_setstr(N, &tobj, colname, NULL, 0);
		do {
			rc = SQLGetData(conn->hSTMT, (SQLUSMALLINT)(field + 1), SQL_C_CHAR, fbuf, FBUFSIZE, &slen);
			if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) {
				n_warn(N, __FN__, "Error %d retrieving field %d:%d", rc, conn->numtuples, field);
				nsp_unlinkval(N, &tobj);
				return 0;
			}
			if (slen > 0) nsp_strcat(N, cobj, fbuf, slen < FBUFSIZE ? slen : FBUFSIZE - 1);
		} while (rc == SQL_SUCCESS_WITH_INFO);
	}
	conn->numtuples++;
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_odbc_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_odbc_endquery()"
	ODBC_CONN *conn = getconn(N);

	if (conn->hSTMT != NULL) {
		SQLFreeHandle(SQL_HANDLE_STMT, conn->hSTMT);
		conn->hSTMT = NULL;
	}
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_data_odbc_client)
{
#define __FN__ __FILE__ ":libnsp_data_odbc_client()"
	obj_t *tobj, *cobj;

	nsp_setstr(N, &N->l, "dsn", "", 0);
	nsp_setbool(N, &N->l, "connection", 0);
	if (nsp_istable((tobj = nsp_getobj(N, &N->l, "1")))) {
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "dsn")))) {
			nsp_setstr(N, &N->l, "dsn", cobj->val->d.str, cobj->val->size);
		}
	}
	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "odbc");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->l, cobj);
	else n_warn(N, __FN__, "data.odbc not found");
	cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "sql"), "common");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->l, cobj);
	else n_warn(N, __FN__, "data.sql.common not found");
	return 0;
#undef __FN__
}

int nspodbc_register_all(nsp_state *N)
{
	obj_t *tobj;

	tobj = nsp_settable(N, &N->g, "data");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "odbc");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "client", (NSP_CFUNC)libnsp_data_odbc_client);
	nsp_setcfunc(N, tobj, "open", (NSP_CFUNC)libnsp_data_odbc_open);
	nsp_setcfunc(N, tobj, "close", (NSP_CFUNC)libnsp_data_odbc_close);
	nsp_setcfunc(N, tobj, "query", (NSP_CFUNC)libnsp_data_odbc_query);
	nsp_setcfunc(N, tobj, "getnext", (NSP_CFUNC)libnsp_data_odbc_getnext);
	nsp_setcfunc(N, tobj, "endquery", (NSP_CFUNC)libnsp_data_odbc_endquery);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspodbc_register_all(N);
	return 0;
}
#endif

#endif /* HAVE_ODBC */
