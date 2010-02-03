/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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
#include "libnsd.h"

/*
 * This is LDAP CLIENT code.
 * bind / unbind / search / delete / modify / modifyRDN / add / abandon / extended
 */

#ifdef HAVE_LDAP
	#include "stubs/ldap.h"
	typedef	int  (*SQLITE3_OPEN)(const char *, sqlite3 **);
	typedef	int  (*SQLITE3_EXEC)(sqlite3 *, const char *, sqlite3_callback, void *, char **);
	typedef	void (*SQLITE3_CLOSE)(sqlite3 *);
	typedef sqlite_int64 (*SQLITE_LASTID)(sqlite3 *);
	static struct {
		SQLITE3_OPEN open;
		SQLITE3_CLOSE close;
		SQLITE3_EXEC exec;
		SQLITE_LASTID lastid;
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

typedef struct {
	char lib_path[255];
	char var_path[255];
	char sql_type[32];
	char sql_odbc_dsn[200];
	char sql_hostname[128];
	char sql_port[12];
	char sql_dbname[32];
	char sql_username[32];
	char sql_password[32];
} SQL_CONF;

#ifdef HAVE_SQLITE3
static int SQLite3DLLInit(char *libdir)
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
	if ((libsqlite3.open   = lib_sym(hinstLib, "sqlite3_open"))==NULL) goto fail;
	if ((libsqlite3.exec   = lib_sym(hinstLib, "sqlite3_exec"))==NULL) goto fail;
	if ((libsqlite3.close  = lib_sym(hinstLib, "sqlite3_close"))==NULL) goto fail;
	if ((libsqlite3.lastid = lib_sym(hinstLib, "sqlite3_last_insert_rowid"))==NULL) goto fail;
	return 0;
fail:
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
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

static int sqlite3Connect(SQL_CONF cnf)
{
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite3DLLInit(cnf.lib_path)!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/db/%s.db3", cnf.var_path, SERVER_BASENAME);
	fixslashes(dbname);
	if (libsqlite3.open(dbname, &db3)!=SQLITE_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "SQLite Connect - %s", zErrMsg);
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

static int sqlite3Update(char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

retry:
	rc=libsqlite3.exec(db3, sqlquery, NULL, 0, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		rc=libsqlite3.lastid(db3);
//		log_error("sql", __FILE__, __LINE__, 1, "lastid = %d", rc);
		return rc;
	case SQLITE_BUSY:
		log_error("sql", __FILE__, __LINE__, 1, "SQLite busy error?: %s", zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	case SQLITE_CORRUPT:
		log_error("sql", __FILE__, __LINE__, 1, "SQLite corrupt error?: %s", zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	if (rc!=SQLITE_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
		log_error("sql", __FILE__, __LINE__, 1, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite3Callback(void *vptr, int argc, char **argv, char **azColName)
{
	obj_t **qobj=vptr;
	obj_t *tobj;
	char name[8];
	unsigned int field;
	unsigned int numfields;
	unsigned int numtuples;

	numfields=argc;
	numtuples=nsp_getnum(NULL, qobj, "_tuples");
	tobj=nsp_getobject(NULL, qobj, "_rows");
	if (tobj->type!=NIL_TTABLE) return -1;
	memset(name, 0, sizeof(name));
	sprintf(name, "%d", numtuples);
	/* get pointer to this record table */
	tobj=nsp_settable(NULL, &tobj->d.table, name);
	if (numtuples==0) nsp_setnumber(NULL, qobj, "_fields", numfields);
	for (field=0;field<numfields;field++) {
		if (argv==NULL) continue;
		nsp_setstring(NULL, &tobj->d.table, azColName[field], (argv[field]!=NULL)?argv[field]:"NULL");
	}
	if (argv!=NULL) nsp_setnumber(NULL, qobj, "_tuples", numtuples+1);
	return 0;
}

static int sqlite3Query(obj_t **qobj, char *sqlquery)
{
	obj_t *tobj;
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	nsp_setstring(NULL, qobj, "_query", sqlquery);
	tobj=nsp_settable(NULL, qobj, "_rows");
	if (tobj->type!=NIL_TTABLE) return -1;
	nsp_setnumber(NULL, qobj, "_fields", 0);
	nsp_setnumber(NULL, qobj, "_tuples", 0);
retry:
	rc=libsqlite3.exec(db3, sqlquery, sqlite3Callback, (void *)qobj, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		log_error("sql", __FILE__, __LINE__, 2, "SQLite: busy or corrupt %d %s", rc, zErrMsg);
		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		log_error("sql", __FILE__, __LINE__, 2, "SQLite: unknown error %d %s", rc, zErrMsg);
		break;
	}
	log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
	log_error("sql", __FILE__, __LINE__, 1, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE3 */

static int _sql_dll_unload()
{
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

void _sql_disconnect(nsp_state *N)
{
	obj_t *tobj;
	char sql_type[32];

	tobj=nsp_getobject(N, &N->globals, "CONFIG");
	if (tobj->type!=NIL_TTABLE) return;
	strncpy(sql_type, nsp_tostring(N, nsp_getobject(N, &tobj->d.table, "sql_type")), sizeof(sql_type)-1);
	log_error("sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if ((strcmp(sql_type, "SQLITE3")==0)||(strcmp(sql_type, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		sqlite3Disconnect();
#endif
	}
	_sql_dll_unload();
	return;
}

void _sql_freeresult(nsp_state *N, obj_t **qobj)
{
	if (qobj!=NULL) {
		nsp_freetable(N, qobj);
		*qobj=NULL;
//		proc.stats.sql_handlecount--;
	} else {
		log_error("sql", __FILE__, __LINE__, 1, "freeing an unused sql result?");
	}
	log_error("sql", __FILE__, __LINE__, 4, "SQL query [0x%08X] freed", &qobj);
	return;
}

int _sql_update(nsp_state *N, char *sqlquery)
{
	SQL_CONF cnf;
	obj_t *tobj;
	int rc=-1;

	tobj=nsp_getobject(N, &N->globals, "CONFIG");
	if (tobj->type!=NIL_TTABLE) return -1;
	strncpy(cnf.sql_type,     nsp_getstring(N, &tobj->d.table, "sql_server_type"), sizeof(cnf.sql_type)-1);
	if (!sql_is_connected) {
		strncpy(cnf.lib_path,     nsp_getstring(N, &tobj->d.table, "lib_path"    ), sizeof(cnf.lib_path)-1);
//		strncpy(cnf.var_db_path,  nsp_getstring(N, &tobj->d.table, "var_db_path" ), sizeof(cnf.var_db_path)-1);
		strncpy(cnf.var_path,     nsp_getstring(N, &tobj->d.table, "var_path"    ), sizeof(cnf.var_path)-1);
		strncpy(cnf.sql_odbc_dsn, nsp_getstring(N, &tobj->d.table, "sql_odbc_dsn"), sizeof(cnf.sql_odbc_dsn)-1);
		strncpy(cnf.sql_hostname, nsp_getstring(N, &tobj->d.table, "sql_hostname"), sizeof(cnf.sql_hostname)-1);
		strncpy(cnf.sql_port,     nsp_getstring(N, &tobj->d.table, "sql_port"    ), sizeof(cnf.sql_port)-1);
		strncpy(cnf.sql_dbname,   nsp_getstring(N, &tobj->d.table, "sql_dbname"  ), sizeof(cnf.sql_dbname)-1);
		strncpy(cnf.sql_username, nsp_getstring(N, &tobj->d.table, "sql_username"), sizeof(cnf.sql_username)-1);
		strncpy(cnf.sql_password, nsp_getstring(N, &tobj->d.table, "sql_password"), sizeof(cnf.sql_password)-1);
	}
	log_error("sql", __FILE__, __LINE__, 2, "SQL update: %s", sqlquery);
//	proc.stats.sql_updates++;
	if ((strcmp(cnf.sql_type, "SQLITE3")==0)||(strcmp(cnf.sql_type, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		if (sqlite3Connect(cnf)<0) {
			return -1;
		}
		rc=sqlite3Update(sqlquery);
#endif
	}
	return rc;
}

int _sql_query(nsp_state *N, obj_t **qobj, char *query)
{
	SQL_CONF cnf;
	obj_t *tobj;
	int rc=-1;

	tobj=nsp_getobject(N, &N->globals, "CONFIG");
	if (tobj->type!=NIL_TTABLE) return -1;
	strncpy(cnf.sql_type,     nsp_getstring(N, &tobj->d.table, "sql_server_type"), sizeof(cnf.sql_type)-1);
	if (!sql_is_connected) {
		strncpy(cnf.lib_path,     nsp_getstring(N, &tobj->d.table, "lib_path"    ), sizeof(cnf.lib_path)-1);
//		strncpy(cnf.var_db_path,  nsp_getstring(N, &tobj->d.table, "var_db_path" ), sizeof(cnf.var_db_path)-1);
		strncpy(cnf.var_path,     nsp_getstring(N, &tobj->d.table, "var_path"    ), sizeof(cnf.var_path)-1);
		strncpy(cnf.sql_odbc_dsn, nsp_getstring(N, &tobj->d.table, "sql_odbc_dsn"), sizeof(cnf.sql_odbc_dsn)-1);
		strncpy(cnf.sql_hostname, nsp_getstring(N, &tobj->d.table, "sql_hostname"), sizeof(cnf.sql_hostname)-1);
		strncpy(cnf.sql_port,     nsp_getstring(N, &tobj->d.table, "sql_port"    ), sizeof(cnf.sql_port)-1);
		strncpy(cnf.sql_dbname,   nsp_getstring(N, &tobj->d.table, "sql_dbname"  ), sizeof(cnf.sql_dbname)-1);
		strncpy(cnf.sql_username, nsp_getstring(N, &tobj->d.table, "sql_username"), sizeof(cnf.sql_username)-1);
		strncpy(cnf.sql_password, nsp_getstring(N, &tobj->d.table, "sql_password"), sizeof(cnf.sql_password)-1);
	}
	log_error("sql", __FILE__, __LINE__, 3, "SQL query: [0x%08X] %s", &qobj, query);
//	proc.stats.sql_queries++;
	if ((strcmp(cnf.sql_type, "SQLITE3")==0)||(strcmp(cnf.sql_type, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		if (sqlite3Connect(cnf)<0) {
			return -1;
		}
		if ((rc=sqlite3Query(qobj, query))<0) {
			sqlite3Disconnect();
		}
#endif
	}
//	if (rc>-1) proc.stats.sql_handlecount++;
	return rc;
}

int _sql_updatef(nsp_state *N, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery=calloc(8192, sizeof(char)))==NULL) {
		log_error("sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc=_sql_update(N, sqlquery);
	free(sqlquery);
	return rc;
}

int _sql_queryf(nsp_state *N, obj_t **qobj, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery=calloc(8192, sizeof(char)))==NULL) {
		log_error("sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc=_sql_query(N, qobj, sqlquery);
	free(sqlquery);
	return rc;
}

char *sql_getname(nsp_state *N, obj_t **qobj, int fieldnumber)
{
	obj_t *cobj;
	int j=0;

	if ((fieldnumber<0)||(fieldnumber+1>nsp_getnum(N, qobj, "_fields"))) return NULL;
	cobj=nsp_getobject(N, qobj, "_rows");
	if (cobj->type!=NIL_TTABLE) return "";
	for (cobj=cobj->d.table; cobj; cobj=cobj->next) {
		if (cobj->type==NIL_TNIL) return "";
		if (cobj->type!=NIL_TTABLE) continue;
		for (cobj=cobj->d.table; cobj; cobj=cobj->next) {
			if (j!=fieldnumber) { j++; continue; }
			return cobj->name;
		}
	}
	return NULL;
}

char *sql_getvalue(nsp_state *N, obj_t **qobj, int tuple, int field)
{
	int i=0;
	int j=0;
	obj_t *tobj;
	obj_t *cobj;

	if ((tuple<0)||(tuple+1>nsp_getnum(N, qobj, "_tuples"))) return NULL;
	if ((field<0)||(field+1>nsp_getnum(N, qobj, "_fields"))) return NULL;
	tobj=nsp_getobject(N, qobj, "_rows");
	if (tobj->type!=NIL_TTABLE) return "";
	for (cobj=tobj->d.table; cobj; cobj=cobj->next) {
		if (cobj->type==NIL_TNIL) return "";
		if (cobj->type!=NIL_TTABLE) continue;
		if (i!=tuple) { i++; continue; }
		for (cobj=cobj->d.table; cobj; cobj=cobj->next) {
			if (j!=field) { j++; continue; }
			if (cobj->type==NIL_TSTRING) {
				return cobj->d.string;
			} else {
				return nsp_tostring(N, cobj);
			}
		}
	}
	return NULL;
}

char *sql_getvaluebyname(nsp_state *N, obj_t **qobj, int tuple, char *fieldname)
{
	int i=0;
	obj_t *tobj;
	obj_t *cobj;

	if ((tuple<0)||(tuple+1>nsp_getnum(N, qobj, "_tuples"))) return NULL;
	tobj=nsp_getobject(N, qobj, "_rows");
	if (tobj->type!=NIL_TTABLE) return "";
	for (cobj=tobj->d.table; cobj; cobj=cobj->next) {
		if (cobj->type==NIL_TNIL) return "";
		if (cobj->type!=NIL_TTABLE) continue;
		if (i!=tuple) { i++; continue; }
		return nsp_getstring(N, &cobj->d.table, fieldname);
	}
	return NULL;
}

int sql_numfields(nsp_state *N, obj_t **qobj)
{
	return nsp_getnum(N, qobj, "_fields");
}

int sql_numtuples(nsp_state *N, obj_t **qobj)
{
	return nsp_getnum(N, qobj, "_tuples");
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
//		} else if ((ch=='\\')&&(strcasecmp(proc.config.sql_type, "SQLITE")!=0)) {
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
