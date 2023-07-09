/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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

#ifdef HAVE_MYSQL

#include <mysql/mysql.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mysql_murder(nsp_state *N, obj_t *cobj);

typedef struct MYSQL_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's socket-specific */
	MYSQL mysql;
	MYSQL *mysock;
	MYSQL_RES *myres;
} MYSQL_CONN;

static MYSQL_CONN *getconn(nsp_state *N)
{
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	MYSQL_CONN *conn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "mysql-conn") != 0))
		n_error(N, NE_SYNTAX, "getconn", "expected a mysql-conn");
	conn = (MYSQL_CONN *)cobj->val->d.str;
	return conn;
}

#include <stdio.h>
#include <string.h>

static void mysqlDisconnect(nsp_state *N, MYSQL_CONN *conn)
{
	if (conn->myres != NULL) {
		mysql_free_result(conn->myres);
		conn->myres = NULL;
	}
	if (conn->mysock != NULL) {
		mysql_close(conn->mysock);
		conn->mysock = NULL;
	}
	return;
}

static int mysqlConnect(nsp_state *N, MYSQL_CONN *conn, char *host, char *user, char *pass, char *db, int port)
{
#define __FN__ __FILE__ ":mysqlConnect()"
	mysql_init(&conn->mysql);
	conn->mysock = mysql_real_connect(&conn->mysql, host, user, pass, db, port, NULL, 0);
	if (conn->mysock == NULL) {
		n_warn(N, __FN__, "connection error");
		return -1;
	}
	return 0;
#undef __FN__
}

void mysql_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":mysql_murder()"
	MYSQL_CONN *conn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "mysql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a mysql conn");
	conn = (MYSQL_CONN *)cobj->val->d.str;
	mysqlDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(MYSQL_CONN) + 1);
	cobj->val->size = 0;
	return;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_mysql_open)
{
#define __FN__ __FILE__ ":libnsp_data_mysql_open()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
//	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	//obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	//obj_t tobj;
	MYSQL_CONN *conn;
	unsigned short port = 0;
	char *host = NULL;
	char *user = NULL;
	char *pass = NULL;
	char *db = NULL;
	int rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "1")))) {
		host = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "hostname")))) {
		host = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "host")))) {
		host = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for host");
	}
	if (nsp_isnum((cobj = nsp_getobj(N, &N->context->l, "2")))) {
		port = (unsigned short)nsp_tonum(N, cobj);
	} else if (nsp_isnum((cobj = nsp_getobj(N, thisobj, "port")))) {
		port = (unsigned short)nsp_tonum(N, cobj);
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a number for port");
	}
	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "3")))) {
		user = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "username")))) {
		user = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for username");
	}
	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "4")))) {
		pass = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "password")))) {
		pass = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for password");
	}
	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "5")))) {
		db = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "database")))) {
		db = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for password");
	}

	//if (cobj1->val->type != NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	// if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	conn = n_alloc(N, sizeof(MYSQL_CONN) + 1, 1);
	strcpy(conn->obj_type, "mysql-conn");
	//rc = mysqlConnect(N, conn, nsp_getstr(N, cobj1, "host"), nsp_getstr(N, cobj1, "user"), nsp_getstr(N, cobj1, "pass"), nsp_getstr(N, cobj1, "db"), (int)nsp_getnum(N, cobj1, "port"));
	rc = mysqlConnect(N, conn, host, user, pass, db, port);
	if (rc < 0) {
		nsp_setstr(N, &N->r, "", "mysql connection error", -1);
		n_free(N, (void *)&conn, sizeof(MYSQL_CONN) + 1);
		return -1;
	}

	cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
	cobj->val->d.str = (void *)conn;
	cobj->val->size = sizeof(MYSQL_CONN) + 1;

	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_mysql_close)
{
#define __FN__ __FILE__ ":libnsp_data_mysql_close()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	MYSQL_CONN *conn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "mysql-conn") != 0))
		return 0;
	/// n_error(N, NE_SYNTAX, __FN__, "expected a mysql-conn");
	conn = (MYSQL_CONN *)cobj->val->d.str;
	mysqlDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(MYSQL_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "connection", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_mysql_query)
{
#define __FN__ __FILE__ ":libnsp_data_mysql_query()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	MYSQL_CONN *conn = getconn(N);
	obj_t *cobj;
	char *sqlquery = NULL;
	//short expect_results = 1;

	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "1")))) {
		sqlquery = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "sqlquery")))) {
		sqlquery = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for sqlquery");
	}
	//if (nsp_isbool((cobj = nsp_getobj(N, &N->context->l, "2")))) {
	//	expect_results = nsp_tobool(N, cobj);
	//}
	if (mysql_ping(conn->mysock) != 0) {
		n_warn(N, __FN__, "connection lost");
		return -1;
	}
	if (mysql_query(conn->mysock, sqlquery)) {
		n_warn(N, __FN__, "query error '%s'\r\n%s", mysql_error(conn->mysock), sqlquery);
		return -1;
	}
	nsp_setstr(N, thisobj, "last_query", sqlquery, -1);
	nsp_setnum(N, thisobj, "changes", (long)mysql_affected_rows(&conn->mysql));
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_mysql_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_mysql_getnext()"
	MYSQL_CONN *conn = getconn(N);
	MYSQL_ROW MYrow;
	MYSQL_FIELD *MYfield;
	obj_t tobj;
	char *p;
	unsigned int field, numfields;

	if (conn->myres == NULL) {
		if (!(conn->myres = mysql_use_result(conn->mysock))) {
			n_warn(N, __FN__, "mysql_use_result error '%s'", mysql_error(conn->mysock));
		}
	}
	if ((MYrow = mysql_fetch_row(conn->myres)) == NULL) {
		if (conn->myres != NULL) {
			mysql_free_result(conn->myres);
			conn->myres = NULL;
		}
		return 0;
	}
	numfields = (int)mysql_num_fields(conn->myres);
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	for (field = 0; field < numfields; field++) {
		p = MYrow[field] ? MYrow[field] : "NULL";
		MYfield = mysql_fetch_field_direct(conn->myres, field);
/*
		Type Value		Type Description
		MYSQL_TYPE_TINY		TINYINT field
		MYSQL_TYPE_SHORT	SMALLINT field
		MYSQL_TYPE_LONG	INTEGER field
		MYSQL_TYPE_INT24	MEDIUMINT field
		MYSQL_TYPE_LONGLONG	BIGINT field
		MYSQL_TYPE_DECIMAL	DECIMAL or NUMERIC field
		MYSQL_TYPE_NEWDECIMAL	Precision math DECIMAL or NUMERIC field (MySQL 5.0.3 and up)
		MYSQL_TYPE_FLOAT	FLOAT field
		MYSQL_TYPE_DOUBLE	DOUBLE or REAL field
		MYSQL_TYPE_BIT		BIT field (MySQL 5.0.3 and up)
		MYSQL_TYPE_TIMESTAMP	TIMESTAMP field
		MYSQL_TYPE_DATE		DATE field
		MYSQL_TYPE_TIME		TIME field
		MYSQL_TYPE_DATETIME	DATETIME field
		MYSQL_TYPE_YEAR		YEAR field
		MYSQL_TYPE_STRING	CHAR or BINARY field
		MYSQL_TYPE_VAR_STRING	VARCHAR or VARBINARY field
		MYSQL_TYPE_BLOB		BLOB or TEXT field (use max_length to determine the maximum length)
		MYSQL_TYPE_SET		SET field
		MYSQL_TYPE_ENUM		ENUM field
		MYSQL_TYPE_GEOMETRY	Spatial field
		MYSQL_TYPE_NULL		NULL-type field
		----
		enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
		MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
		MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
		MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
		MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
		MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
		MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
		MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
		MYSQL_TYPE_BIT,
		MYSQL_TYPE_NEWDECIMAL=246,
		MYSQL_TYPE_ENUM=247,
		MYSQL_TYPE_SET=248,
		MYSQL_TYPE_TINY_BLOB=249,
		MYSQL_TYPE_MEDIUM_BLOB=250,
		MYSQL_TYPE_LONG_BLOB=251,
		MYSQL_TYPE_BLOB=252,
		MYSQL_TYPE_VAR_STRING=253,
		MYSQL_TYPE_STRING=254,
		MYSQL_TYPE_GEOMETRY=255
		};
*/
		switch (MYfield->type) {
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_LONG:
			nsp_setnum(N, &tobj, MYfield->name, atoi(p));
			break;
		case MYSQL_TYPE_FLOAT:
			nsp_setstr(N, &tobj, MYfield->name, p, -1);
			break;
		case MYSQL_TYPE_VAR_STRING:
			nsp_setstr(N, &tobj, MYfield->name, p, -1);
			break;
		case MYSQL_TYPE_NULL:
			nsp_setstr(N, &tobj, MYfield->name, "NULL", -1);
			break;
		default:
			nsp_setstr(N, &tobj, MYfield->name, p, -1);
			n_warn(N, __FN__, "Unhandled type %d for %s!", MYfield->type, MYfield->name);
			break;
		}
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_mysql_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_mysql_endquery()"
	MYSQL_CONN *conn = getconn(N);

	if (conn->myres != NULL) {
		mysql_free_result(conn->myres);
		conn->myres = NULL;
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_mysql_begin)
{
	MYSQL_CONN *conn = getconn(N);

	if (mysql_query(conn->mysock, "BEGIN;")) {
		n_warn(N, "", "libnsp_data_mysql_begin error '%s'\r\n%s", mysql_error(conn->mysock));
		return -1;
	}
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_mysql_commit)
{
	MYSQL_CONN *conn = getconn(N);

	if (mysql_query(conn->mysock, "COMMIT;")) {
		n_warn(N, "", "libnsp_data_mysql_commit error '%s'\r\n%s", mysql_error(conn->mysock));
		return -1;
	}
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_mysql_rollback)
{
	MYSQL_CONN *conn = getconn(N);

	if (mysql_query(conn->mysock, "ROLLBACK;")) {
		n_warn(N, "", "libnsp_data_mysql_rollback error '%s'\r\n%s", mysql_error(conn->mysock));
		return -1;
	}
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_mysql_version)
{
	char buf[64];

	snprintf(buf, sizeof(buf) - 1, "%s / %s", "SQLITE_VERSION", mysql_get_client_info());
	nsp_setstr(N, &N->r, "", buf, -1);
	return 0;
}

NSP_CLASS(libnsp_data_mysql_client)
{
#define __FN__ __FILE__ ":libnsp_data_mysql_client()"
	obj_t *tobj, *cobj;

	nsp_setstr(N, &N->context->l, "host", "localhost", 9);
	nsp_setnum(N, &N->context->l, "port", 3306);
	nsp_setstr(N, &N->context->l, "username", "root", 4);
	nsp_setstr(N, &N->context->l, "password", "", 0);
	nsp_setstr(N, &N->context->l, "database", "", 0);
	nsp_setbool(N, &N->context->l, "connection", 0);
	if (nsp_istable((tobj = nsp_getobj(N, &N->context->l, "1")))) {
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "username")))) {
			nsp_setstr(N, &N->context->l, "username", cobj->val->d.str, cobj->val->size);
		}
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "password")))) {
			nsp_setstr(N, &N->context->l, "password", cobj->val->d.str, cobj->val->size);
		}
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "hostname")))) {
			nsp_setstr(N, &N->context->l, "host", cobj->val->d.str, cobj->val->size);
		}
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "host")))) {
			nsp_setstr(N, &N->context->l, "host", cobj->val->d.str, cobj->val->size);
		}
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "database")))) {
			nsp_setstr(N, &N->context->l, "database", cobj->val->d.str, cobj->val->size);
		}
	}

	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "mysql");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->context->l, cobj);
	else n_warn(N, __FN__, "data.mysql not found");
	cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "sql"), "common");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->context->l, cobj);
	else n_warn(N, __FN__, "data.sql.common not found");
	return 0;
#undef __FN__
}

int nspmysql_register_all(nsp_state *N)
{
	obj_t *tobj;

	tobj = nsp_settable(N, nsp_settable(N, &N->g, "lib"), "data");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "mysql");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "client", (NSP_CFUNC)libnsp_data_mysql_client);
	nsp_setcfunc(N, tobj, "open", (NSP_CFUNC)libnsp_data_mysql_open);
	nsp_setcfunc(N, tobj, "close", (NSP_CFUNC)libnsp_data_mysql_close);
	nsp_setcfunc(N, tobj, "query", (NSP_CFUNC)libnsp_data_mysql_query);
	nsp_setcfunc(N, tobj, "getnext", (NSP_CFUNC)libnsp_data_mysql_getnext);
	nsp_setcfunc(N, tobj, "endquery", (NSP_CFUNC)libnsp_data_mysql_endquery);
	nsp_setcfunc(N, tobj, "begin", (NSP_CFUNC)libnsp_data_mysql_begin);
	nsp_setcfunc(N, tobj, "commit", (NSP_CFUNC)libnsp_data_mysql_commit);
	nsp_setcfunc(N, tobj, "rollback", (NSP_CFUNC)libnsp_data_mysql_rollback);

	nsp_setcfunc(N, tobj, "version", (NSP_CFUNC)libnsp_data_mysql_version);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspmysql_register_all(N);
	return 0;
}
#endif

#endif /* HAVE_MYSQL */
