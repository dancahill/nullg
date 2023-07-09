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

#ifdef HAVE_PGSQL

#include <libpq-fe.h>
#include <string.h>

void pgsql_murder(nsp_state *N, obj_t *cobj);

typedef struct PGSQL_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's socket-specific */
	PGconn   *pgconn;
	PGresult *pgresult;
	unsigned int last_row;
} PGSQL_CONN;

static PGSQL_CONN *getconn(nsp_state *N)
{
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	PGSQL_CONN *conn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "pgsql-conn") != 0))
		n_error(N, NE_SYNTAX, "getconn", "expected a pgsql-conn");
	conn = (PGSQL_CONN *)cobj->val->d.str;
	return conn;
}

#include "pgsql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void pgsqlDisconnect(nsp_state *N, PGSQL_CONN *conn)
{
	if (conn->pgconn != NULL) {
		PQfinish(conn->pgconn);
		conn->pgconn = NULL;
	}
	return;
}

static int pgsqlConnect(nsp_state *N, PGSQL_CONN *conn, char *host, char *user, char *pass, char *db, char *port)
{
#define __FN__ __FILE__ ":pgsqlConnect()"
	conn->pgconn = PQsetdbLogin(host, port, NULL, NULL, db, user, pass);
	if (PQstatus(conn->pgconn) != CONNECTION_OK) {
		n_warn(N, __FN__, "%s", PQerrorMessage(conn->pgconn));
		return -1;
	}
	PQclear(conn->pgresult);
	return 0;
#undef __FN__
}

void pgsql_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":pgsql_murder()"
	PGSQL_CONN *conn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if (cobj->val->type != NT_CDATA || cobj->val->d.str == NULL || nc_strcmp(cobj->val->d.str, "pgsql-conn") != 0)
		n_error(N, NE_SYNTAX, __FN__, "expected a pgsql conn");
	conn = (PGSQL_CONN *)cobj->val->d.str;
	pgsqlDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(PGSQL_CONN) + 1);
	return;
#undef __FN__
}
/*
static int pgsqlUpdate(nsp_state *N, PGSQL_CONN *conn, char *sqlquery)
{
	return 0;
}
*/
/*
static int pgsqlQuery(nsp_state *N, PGSQL_CONN *conn, char *sqlquery, obj_t *qobj)
{
#define __FN__ __FILE__ ":pgsqlQuery()"
	char *q;
	obj_t *robj, *tobj;
	char *p;
	char name[8];
	unsigned int sqllen = strlen(sqlquery);
	unsigned int plen;
	unsigned int field, tuple;
	unsigned int numfields, numtuples;

	conn->pgresult = PQexec(conn->pgconn, "BEGIN");
	if (PQresultStatus(conn->pgresult) != PGRES_COMMAND_OK) {
		n_warn(N, __FN__, "BEGIN failed: %s", PQerrorMessage(conn->pgconn));
		PQclear(conn->pgresult);
		pgsqlDisconnect(N, conn);
		return -1;
	}
	PQclear(conn->pgresult);

	if ((q = n_alloc(N, sqllen + 42, 1)) == NULL) {
		n_warn(N, __FN__, "calloc error");
		pgsqlDisconnect(N, conn);
		return -1;
	}
	sprintf(q, "DECLARE myportal CURSOR FOR %s", sqlquery);
	conn->pgresult = PQexec(conn->pgconn, q);
	n_free(N, (void *)&q, sqllen + 42);

	if (PQresultStatus(conn->pgresult) != PGRES_COMMAND_OK) {
		n_warn(N, __FN__, "QUERY failed: %s", PQerrorMessage(conn->pgconn));
		PQclear(conn->pgresult);
		pgsqlDisconnect(N, conn);
		return -1;
	}
	PQclear(conn->pgresult);

	conn->pgresult = PQexec(conn->pgconn, "FETCH ALL IN myportal");
	if (PQresultStatus(conn->pgresult) != PGRES_TUPLES_OK) {
		n_warn(N, __FN__, "FETCH failed: %s", PQerrorMessage(conn->pgconn));
		PQclear(conn->pgresult);
		pgsqlDisconnect(N, conn);
		return -1;
	}
	numfields = (int)PQnfields(conn->pgresult);
	numtuples = (int)PQntuples(conn->pgresult);
	nsp_setstr(NULL, qobj, "_query", sqlquery, sqllen);
	nsp_setnum(NULL, qobj, "_fields", numfields);
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	robj = nsp_settable(NULL, qobj, "_rows");
	for (tuple = 0;tuple < numtuples;tuple++) {
		memset(name, 0, sizeof(name));
		n_ntoa(N, name, tuple, 10, 0);
		tobj = nsp_settable(NULL, robj, name);
		tobj->val->attr &= ~NST_AUTOSORT;
		for (field = 0;field < numfields;field++) {
			p = PQgetvalue(conn->pgresult, tuple, field);
			if (p == NULL) {
				p = "NULL";
				plen = 4;
			}
			else {
				plen = PQgetlength(conn->pgresult, tuple, field);
			}
			nsp_setstr(NULL, tobj, PQfname(conn->pgresult, field), p, plen);
		}
	}
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	PQclear(conn->pgresult);

	conn->pgresult = PQexec(conn->pgconn, "CLOSE myportal");
	PQclear(conn->pgresult);

	conn->pgresult = PQexec(conn->pgconn, "END");
	PQclear(conn->pgresult);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_data_pgsql_query)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_query()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2");
	obj_t tobj;
	PGSQL_CONN *conn;
	int rc;

	if (cobj1->val->type != NT_TABLE) n_error(N, NE_SYNTAX, __FN__, "expected a table for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	conn = calloc(1, sizeof(PGSQL_CONN) + 1);
	strcpy(conn->obj_type, "pgsql-conn");
	rc = pgsqlConnect(N, conn, nsp_getstr(N, cobj1, "host"), nsp_getstr(N, cobj1, "user"), nsp_getstr(N, cobj1, "pass"), nsp_getstr(N, cobj1, "db"), nsp_getstr(N, cobj1, "port"));
	if (rc < 0) {
		nsp_setstr(N, &N->r, "", "pgsql connection error", -1);
		n_free(N, (void *)&conn, sizeof(PGSQL_CONN) + 1);
		return -1;
	}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr &= ~NST_AUTOSORT;
	pgsqlQuery(N, conn, cobj2->val->d.str, &tobj);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	pgsqlDisconnect(N, conn);
	n_free(N, (void *)&conn, sizeof(PGSQL_CONN) + 1);
	return 0;
#undef __FN__
}
*/

NSP_CLASSMETHOD(libnsp_data_pgsql_open)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_open()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	PGSQL_CONN *conn;
	unsigned short port = 0;
	char port_s[8];
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
	conn = n_alloc(N, sizeof(PGSQL_CONN) + 1, 1);
	strcpy(conn->obj_type, "pgsql-conn");
	snprintf(port_s, sizeof(port_s) - 1, "%d", port);
	rc = pgsqlConnect(N, conn, host, user, pass, db, port_s);
	if (rc < 0) {
		nsp_setstr(N, &N->r, "", "pgsql connection error", -1);
		n_free(N, (void *)&conn, sizeof(PGSQL_CONN) + 1);
		return -1;
	}
	conn->last_row = 0;
	cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
	cobj->val->d.str = (void *)conn;
	cobj->val->size = sizeof(PGSQL_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_pgsql_close)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_close()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	PGSQL_CONN *conn = getconn(N);

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sqlite-conn") != 0))
		return 0;
	// n_error(N, NE_SYNTAX, __FN__, "expected a sqlite-conn");
	conn = (PGSQL_CONN *)cobj->val->d.str;
	pgsqlDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(PGSQL_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "connection", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_pgsql_query)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_query()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	PGSQL_CONN *conn = getconn(N);
	char *sqlquery = NULL;
	short expect_results = 1;

	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "1")))) {
		sqlquery = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "sqlquery")))) {
		sqlquery = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for sqlquery");
	}
	if (nsp_isbool((cobj = nsp_getobj(N, &N->context->l, "2")))) {
		expect_results = nsp_tobool(N, cobj);
	}
	conn->pgresult = PQexec(conn->pgconn, sqlquery);
	if (PQresultStatus(conn->pgresult) != PGRES_TUPLES_OK && PQresultStatus(conn->pgresult) != PGRES_COMMAND_OK) {
		n_warn(N, __FN__, "PQexec failed: '%s'\r\n'%s'", PQresultErrorMessage(conn->pgresult), sqlquery);
		PQclear(conn->pgresult);
		return -1;
	}
	if (!expect_results) {
		PQclear(conn->pgresult);
	}
	/*
		create or replace function x returns integer is $$
		declare
			number_of_rows integer := 0;
		begin
			update table_y set col_x = default where col_z is null;
			GET DIAGNOSTICS number_of_rows = ROW_COUNT;
			return number_of_rows;
		end;
		$$ language plpgsql;
	*/
	nsp_setstr(N, thisobj, "last_query", sqlquery, -1);
	nsp_setnum(N, thisobj, "changes", -1);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_pgsql_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_getnext()"
	obj_t tobj;
	PGSQL_CONN *conn = getconn(N);
	unsigned int field;
	unsigned int plen;
	char *p;

	int numfields = (int)PQnfields(conn->pgresult);
	int numtuples = (int)PQntuples(conn->pgresult);
	if (conn->last_row == numtuples) return 0;
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	for (field = 0; field < numfields; field++) {
		p = PQgetvalue(conn->pgresult, conn->last_row, field);
		if (p == NULL) {
			p = "NULL";
			plen = 4;
		} else {
			plen = PQgetlength(conn->pgresult, conn->last_row, field);
		}
		nsp_setstr(N, &tobj, PQfname(conn->pgresult, field), p, plen);
	}
	//PQclear(conn->pgresult);
	conn->last_row++;
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_pgsql_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_endquery()"
	PGSQL_CONN *conn = getconn(N);

	PQclear(conn->pgresult);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_pgsql_begin)
{
	PGSQL_CONN *conn = getconn(N);

	conn->pgresult = PQexec(conn->pgconn, "BEGIN");
	PQclear(conn->pgresult);
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_pgsql_commit)
{
	PGSQL_CONN *conn = getconn(N);

	conn->pgresult = PQexec(conn->pgconn, "COMMIT");
	PQclear(conn->pgresult);
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_pgsql_rollback)
{
	PGSQL_CONN *conn = getconn(N);

	conn->pgresult = PQexec(conn->pgconn, "ROLLBACK");
	PQclear(conn->pgresult);
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_pgsql_version)
{
	return 0;
}

NSP_CLASS(libnsp_data_pgsql_client)
{
#define __FN__ __FILE__ ":libnsp_data_pgsql_client()"
	obj_t *tobj, *cobj;

	nsp_setstr(N, &N->context->l, "host", "localhost", 9);
	nsp_setnum(N, &N->context->l, "port", 5432);
	nsp_setstr(N, &N->context->l, "username", "postgres", 8);
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
	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "pgsql");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->context->l, cobj);
	else n_warn(N, __FN__, "data.pgsql not found");
	cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "sql"), "common");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->context->l, cobj);
	else n_warn(N, __FN__, "data.sql.common not found");
	return 0;
#undef __FN__
}

int nsppgsql_register_all(nsp_state *N)
{
	obj_t *tobj;

	tobj = nsp_settable(N, nsp_settable(N, &N->g, "lib"), "data");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "pgsql");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "client", (NSP_CFUNC)libnsp_data_pgsql_client);
	nsp_setcfunc(N, tobj, "open", (NSP_CFUNC)libnsp_data_pgsql_open);
	nsp_setcfunc(N, tobj, "close", (NSP_CFUNC)libnsp_data_pgsql_close);
	nsp_setcfunc(N, tobj, "query", (NSP_CFUNC)libnsp_data_pgsql_query);
	nsp_setcfunc(N, tobj, "getnext", (NSP_CFUNC)libnsp_data_pgsql_getnext);
	nsp_setcfunc(N, tobj, "endquery", (NSP_CFUNC)libnsp_data_pgsql_endquery);
	nsp_setcfunc(N, tobj, "begin", (NSP_CFUNC)libnsp_data_pgsql_begin);
	nsp_setcfunc(N, tobj, "commit", (NSP_CFUNC)libnsp_data_pgsql_commit);
	nsp_setcfunc(N, tobj, "rollback", (NSP_CFUNC)libnsp_data_pgsql_rollback);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nsppgsql_register_all(N);
	return 0;
}
#endif

#endif /* HAVE_PGSQL */
