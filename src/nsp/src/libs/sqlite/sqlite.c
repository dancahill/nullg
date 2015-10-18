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

#ifdef HAVE_SQLITE

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

typedef struct SQLITE_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's socket-specific */
	sqlite3 *db3;
	sqlite3_stmt *stm;

	//	unsigned long numfields;
	//	unsigned long numtuples;
} SQLITE_CONN;

static SQLITE_CONN *getconn(nsp_state *N)
{
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SQLITE_CONN *conn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sqlite-conn") != 0))
		n_error(N, NE_SYNTAX, "getconn", "expected a sqlite-conn");
	conn = (SQLITE_CONN *)cobj->val->d.str;
	return conn;
}

static char *sqlite3Error(int err)
{
	switch (err) {
	case SQLITE_OK:         return "Successful result";
	case SQLITE_ERROR:      return "SQL error or missing database";
	case SQLITE_INTERNAL:   return "Internal logic error in SQLite";
	case SQLITE_PERM:       return "Access permission denied";
	case SQLITE_ABORT:      return "Callback routine requested an abort";
	case SQLITE_BUSY:       return "The database file is locked";
	case SQLITE_LOCKED:     return "A table in the database is locked";
	case SQLITE_NOMEM:      return "A malloc() failed";
	case SQLITE_READONLY:   return "Attempt to write a readonly database";
	case SQLITE_INTERRUPT:  return "Operation terminated by sqlite3_interrupt()";
	case SQLITE_IOERR:      return "Some kind of disk I/O error occurred";
	case SQLITE_CORRUPT:    return "The database disk image is malformed";
	case SQLITE_NOTFOUND:   return "NOT USED. Table or record not found";
	case SQLITE_FULL:       return "Insertion failed because database is full";
	case SQLITE_CANTOPEN:   return "Unable to open the database file";
	case SQLITE_PROTOCOL:   return "NOT USED. Database lock protocol error";
	case SQLITE_EMPTY:      return "Database is empty";
	case SQLITE_SCHEMA:     return "The database schema changed";
	case SQLITE_TOOBIG:     return "String or BLOB exceeds size limit";
	case SQLITE_CONSTRAINT: return "Abort due to constraint violation";
	case SQLITE_MISMATCH:   return "Data type mismatch";
	case SQLITE_MISUSE:     return "Library used incorrectly";
	case SQLITE_NOLFS:      return "Uses OS features not supported on host";
	case SQLITE_AUTH:       return "Authorization denied";
	case SQLITE_FORMAT:     return "Auxiliary database format error";
	case SQLITE_RANGE:      return "2nd parameter to sqlite3_bind out of range";
	case SQLITE_NOTADB:     return "File opened that is not a database file";
	case SQLITE_ROW:        return "sqlite3_step() has another row ready";
	case SQLITE_DONE:       return "sqlite3_step() has finished executing";
	}
	return "Unknown error";
}

static void sqlite3Disconnect(nsp_state *N, SQLITE_CONN *conn)
{
	if (conn->db3 != NULL) {
		sqlite3_close(conn->db3);
		conn->db3 = NULL;
	}
	return;
}

static int sqlite3Connect(nsp_state *N, SQLITE_CONN *conn, char *dbname)
{
#define __FN__ __FILE__ ":sqlite3Connect()"
	int rc;

	if ((rc = sqlite3_open(dbname, &conn->db3)) != SQLITE_OK) {
		n_warn(N, __FN__, "SQLite connect error: '%s'", sqlite3Error(rc));
		return -1;
	}
	//sqlite3_exec(conn->db3, "PRAGMA empty_result_callbacks = ON;", NULL, NULL, NULL);
	/* PRAGMA [database.]synchronous=OFF|ON|NORMAL|FULL */
	//sqlite3_exec(conn->db3, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);
	/* PRAGMA temp_store = "default"|"memory"|"file" */
	sqlite3_exec(conn->db3, "PRAGMA temp_store = memory;", NULL, NULL, NULL);
	return 0;
#undef __FN__
}

void sqlite3_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":sqlite3_murder()"
	SQLITE_CONN *conn;

	//	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "sqlite-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a sqlite-conn");
	conn = (SQLITE_CONN *)cobj->val->d.str;
	sqlite3Disconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(SQLITE_CONN) + 1);
	cobj->val->size = 0;
	return;
#undef __FN__
}

static void store_field(nsp_state *N, sqlite3_stmt *stm, int column, obj_t *tobj)
{
#define __FN__ __FILE__ ":sqlite store_field()"
	//char buf[64];
	char *name;

	name = (char *)sqlite3_column_name(stm, column);
	switch (sqlite3_column_type(stm, column)) {
	case SQLITE_INTEGER:
		//snprintf(buf, sizeof(buf) - 1, "%lld", sqlite3_column_int64(stm, column));
		//nsp_setstr(N, tobj, name, buf, -1);
		nsp_setnum(N, tobj, name, sqlite3_column_int64(stm, column));
		break;
	case SQLITE_FLOAT:
		//snprintf(buf, sizeof(buf) - 1, "%f", sqlite3_column_double(stm, column));
		//nsp_setstr(N, tobj, name, buf, -1);
		nsp_setnum(N, tobj, name, sqlite3_column_double(stm, column));
		break;
	case SQLITE_TEXT:
	case SQLITE_BLOB:
		nsp_setstr(N, tobj, name, (char *)sqlite3_column_text(stm, column), -1);
		break;
	case SQLITE_NULL:
		nsp_setstr(N, tobj, name, "NULL", -1);
		break;
	default:
		n_warn(N, __FN__, "Unhandled type!");
		break;
	}
	return;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sqlite_open)
{
#define __FN__ __FILE__ ":libnsp_data_sqlite_open()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SQLITE_CONN *conn;
	int rc;
	char *db = NULL;

	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "1")))) {
		db = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "database")))) {
		db = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for database");
	}
	conn = n_alloc(N, sizeof(SQLITE_CONN) + 1, 1);
	strcpy(conn->obj_type, "sqlite-conn");
	conn->obj_term = (NSP_CFREE)sqlite3_murder;
	rc = sqlite3Connect(N, conn, db);
	if (rc < 0) {
		nsp_setstr(N, &N->r, "", "sqlite connection error", -1);
		n_free(N, (void *)&conn, sizeof(SQLITE_CONN) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
	cobj->val->d.str = (void *)conn;
	cobj->val->size = sizeof(SQLITE_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sqlite_close)
{
#define __FN__ __FILE__ ":libnsp_data_sqlite_close()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SQLITE_CONN *conn = getconn(N);

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "sqlite-conn") != 0))
		return 0;
	// n_error(N, NE_SYNTAX, __FN__, "expected a sqlite-conn");
	conn = (SQLITE_CONN *)cobj->val->d.str;
	sqlite3Disconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(SQLITE_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "connection", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sqlite_query)
{
#define __FN__ __FILE__ ":libnsp_data_sqlite_query()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SQLITE_CONN *conn = getconn(N);
	int rc;
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
	rc = sqlite3_prepare(conn->db3, sqlquery, -1, &conn->stm, NULL);
	if (rc != SQLITE_OK) {
		n_warn(N, __FN__, "SQLite query error: '%s' '%s'", sqlite3Error(rc), sqlquery);
		return -1;
	}
	if (!expect_results) {
		int rc;

		rc = sqlite3_step(conn->stm);
		if (rc != SQLITE_DONE && rc != SQLITE_MISUSE && rc != SQLITE_ROW) {
			n_warn(N, __FN__, "SQLite endquery error: '%s'", sqlite3Error(rc));
		}
		rc = sqlite3_finalize(conn->stm);
		if (rc != SQLITE_OK) {
			n_warn(N, __FN__, "SQLite endquery finalize error: '%s'", sqlite3Error(rc));
		}
	}
//	cobj = nsp_settable(N, thisobj, "last_query");
//	nsp_setstr(N, cobj, "query", sqlquery, -1);
//	nsp_setnum(N, cobj, "_fields", conn->numfields);
	nsp_setstr(N, thisobj, "last_query", sqlquery, -1);
	nsp_setnum(N, thisobj, "changes", sqlite3_changes(conn->db3));
	nsp_setnum(N, thisobj, "totalchanges", sqlite3_total_changes(conn->db3));
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sqlite_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_sqlite_getnext()"
	obj_t tobj;
	SQLITE_CONN *conn = getconn(N);
	int rc;
	unsigned int i;
	unsigned long numfields;

	rc = sqlite3_step(conn->stm);
	if (rc == SQLITE_DONE) return 0;
	if (rc != SQLITE_ROW) {
		n_warn(N, __FN__, "SQLite getnext error: '%s'", sqlite3Error(rc));
		return 0;
	}
	numfields = sqlite3_data_count(conn->stm);
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	for (i = 0;i < numfields;i++) {
		store_field(N, conn->stm, i, &tobj);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sqlite_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_sqlite_endquery()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	SQLITE_CONN *conn = getconn(N);
	int rc;

	rc = sqlite3_step(conn->stm);
	if (rc != SQLITE_DONE && rc != SQLITE_MISUSE && rc != SQLITE_ROW) {
		n_warn(N, __FN__, "SQLite endquery error: '%s'", sqlite3Error(rc));
	}
	rc = sqlite3_finalize(conn->stm);
	if (rc != SQLITE_OK) {
		n_warn(N, __FN__, "SQLite endquery finalize error: '%s'", sqlite3Error(rc));
	}
	nsp_setnum(N, thisobj, "changes", sqlite3_changes(conn->db3));
	nsp_setnum(N, thisobj, "totalchanges", sqlite3_total_changes(conn->db3));
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sqlite_begin)
{
	SQLITE_CONN *conn = getconn(N);

	sqlite3_exec(conn->db3, "BEGIN;", NULL, NULL, NULL);
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_sqlite_commit)
{
	SQLITE_CONN *conn = getconn(N);

	sqlite3_exec(conn->db3, "COMMIT;", NULL, NULL, NULL);
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_sqlite_rollback)
{
	SQLITE_CONN *conn = getconn(N);

	sqlite3_exec(conn->db3, "ROLLBACK;", NULL, NULL, NULL);
	return 0;
}

NSP_CLASSMETHOD(libnsp_data_sqlite_version)
{
	char buf[64];

	snprintf(buf, sizeof(buf) - 1, "%s / %s", SQLITE_VERSION, sqlite3_libversion());
	nsp_setstr(N, &N->r, "", buf, -1);
	return 0;
}

NSP_CLASS(libnsp_data_sqlite_client)
{
#define __FN__ __FILE__ ":libnsp_data_sqlite_client()"
	obj_t *tobj, *cobj;

	nsp_setstr(N, &N->l, "database", "", 0);
	nsp_setbool(N, &N->l, "connection", 0);
	if (nsp_istable((tobj = nsp_getobj(N, &N->l, "1")))) {
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "database")))) {
			nsp_setstr(N, &N->l, "database", cobj->val->d.str, cobj->val->size);
		}
	}
	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "sqlite");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->l, cobj);
	else n_warn(N, __FN__, "data.sqlite not found");
	cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "sql"), "common");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->l, cobj);
	else n_warn(N, __FN__, "data.sql.common not found");
	return 0;
#undef __FN__
}

int nspsqlite_register_all(nsp_state *N)
{
	obj_t *tobj;

	tobj = nsp_settable(N, &N->g, "data");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "sqlite");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "client", (NSP_CFUNC)libnsp_data_sqlite_client);
	nsp_setcfunc(N, tobj, "open", (NSP_CFUNC)libnsp_data_sqlite_open);
	nsp_setcfunc(N, tobj, "close", (NSP_CFUNC)libnsp_data_sqlite_close);
	nsp_setcfunc(N, tobj, "query", (NSP_CFUNC)libnsp_data_sqlite_query);
	nsp_setcfunc(N, tobj, "getnext", (NSP_CFUNC)libnsp_data_sqlite_getnext);
	nsp_setcfunc(N, tobj, "endquery", (NSP_CFUNC)libnsp_data_sqlite_endquery);
	nsp_setcfunc(N, tobj, "begin", (NSP_CFUNC)libnsp_data_sqlite_begin);
	nsp_setcfunc(N, tobj, "commit", (NSP_CFUNC)libnsp_data_sqlite_commit);
	nsp_setcfunc(N, tobj, "rollback", (NSP_CFUNC)libnsp_data_sqlite_rollback);

	nsp_setcfunc(N, tobj, "version", (NSP_CFUNC)libnsp_data_sqlite_version);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspsqlite_register_all(N);
	return 0;
}
#endif

#endif /* HAVE_SQLITE */
