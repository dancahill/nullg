/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

#ifdef HAVE_FBSQL
	#ifdef HAVE_FBSQL_FBSQL_H
	#include "<ibase.h>"
	#else
	#include "stubs/fbsql_ibase.h"
	#endif
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_ATTACH_DATABASE)(ISC_STATUS*, short, const ISC_SCHAR*, isc_db_handle*, short, const ISC_SCHAR*);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_ALLOCATE_STATEMENT)(ISC_STATUS *, isc_db_handle *, isc_stmt_handle *);
	typedef ISC_STATUS ISC_EXPORT_VARARG (*LIBFBSQL_ISC_START_TRANSACTION)(ISC_STATUS*, isc_tr_handle*, short, ...);
	typedef ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_PREPARE)(ISC_STATUS*, isc_tr_handle*, isc_stmt_handle*, unsigned short, const ISC_SCHAR*, unsigned short, XSQLDA*);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_SQL_INFO)(ISC_STATUS*, isc_stmt_handle*, short, const ISC_SCHAR*, short, ISC_SCHAR*);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_EXECUTE)(ISC_STATUS*, isc_tr_handle*, isc_stmt_handle*, unsigned short, const XSQLDA*);
	typedef	ISC_LONG   ISC_EXPORT (*LIBFBSQL_ISC_VAX_INTEGER)(const ISC_SCHAR*, short);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_DESCRIBE)(ISC_STATUS *, isc_stmt_handle *, unsigned short, XSQLDA *);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_COMMIT_TRANSACTION)(ISC_STATUS *, isc_tr_handle *);
	typedef	ISC_LONG   ISC_EXPORT (*LIBFBSQL_FB_INTERPRET)(ISC_SCHAR*, unsigned int, const ISC_STATUS**);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_FREE_STATEMENT)(ISC_STATUS *, isc_stmt_handle *, unsigned short);

	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DSQL_FETCH)(ISC_STATUS *, isc_stmt_handle *, unsigned short, const XSQLDA *);
	typedef	void       ISC_EXPORT (*LIBFBSQL_ISC_DECODE_SQL_TIME)(const ISC_TIME*, void*);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_OPEN_BLOB)(ISC_STATUS*, isc_db_handle*, isc_tr_handle*, isc_blob_handle*, ISC_QUAD*);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_GET_SEGMENT)(ISC_STATUS *, isc_blob_handle *, unsigned short *, unsigned short, ISC_SCHAR *);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_CLOSE_BLOB)(ISC_STATUS *, isc_blob_handle *);
	typedef	void       ISC_EXPORT (*LIBFBSQL_ISC_DECODE_TIMESTAMP)(const ISC_TIMESTAMP*, void*);
	typedef	void       ISC_EXPORT (*LIBFBSQL_ISC_DECODE_SQL_DATE)(const ISC_DATE*, void*);
	typedef	ISC_STATUS ISC_EXPORT (*LIBFBSQL_ISC_DETACH_DATABASE)(ISC_STATUS *, isc_db_handle *);
	struct {
		LIBFBSQL_ISC_ATTACH_DATABASE isc_attach_database;
		LIBFBSQL_ISC_DSQL_ALLOCATE_STATEMENT isc_dsql_allocate_statement;
		LIBFBSQL_ISC_START_TRANSACTION isc_start_transaction;
		LIBFBSQL_ISC_DSQL_PREPARE isc_dsql_prepare;
		LIBFBSQL_ISC_DSQL_SQL_INFO isc_dsql_sql_info;
		LIBFBSQL_ISC_DSQL_EXECUTE isc_dsql_execute;
		LIBFBSQL_ISC_VAX_INTEGER isc_vax_integer;
		LIBFBSQL_ISC_DSQL_DESCRIBE isc_dsql_describe;
		LIBFBSQL_ISC_COMMIT_TRANSACTION isc_commit_transaction;
		LIBFBSQL_FB_INTERPRET fb_interpret;
		LIBFBSQL_ISC_DSQL_FREE_STATEMENT isc_dsql_free_statement;
		LIBFBSQL_ISC_DSQL_FETCH isc_dsql_fetch;
		LIBFBSQL_ISC_DECODE_SQL_TIME isc_decode_sql_time;
		LIBFBSQL_ISC_OPEN_BLOB isc_open_blob;
		LIBFBSQL_ISC_GET_SEGMENT isc_get_segment;
		LIBFBSQL_ISC_CLOSE_BLOB isc_close_blob;
		LIBFBSQL_ISC_DECODE_TIMESTAMP isc_decode_timestamp;
		LIBFBSQL_ISC_DECODE_SQL_DATE isc_decode_sql_date;
		LIBFBSQL_ISC_DETACH_DATABASE isc_detach_database;
	} libfbsql;
	static isc_db_handle isc_db;
	static isc_tr_handle trans;
	static isc_stmt_handle stmt;
	static XSQLDA ISC_FAR *sqlda;
	static long fetch_stat;
	static short trans_block;
	static int numfields;
	static int numtuples;
	#define    MAXLEN    8192
	static long buffer[MAXLEN];
#endif
#ifdef HAVE_MYSQL
	#ifdef HAVE_MYSQL_MYSQL_H
	#include "mysql/mysql.h"
	#else
	#include "stubs/mysql.h"
	#endif
	typedef	void          (STDCALL *LIBMYSQL_CLOSE)(MYSQL *);
	typedef	void          (STDCALL *LIBMYSQL_DATA_SEEK)(MYSQL_RES *, my_ulonglong);
	typedef	char         *(STDCALL *LIBMYSQL_ERROR)(MYSQL *);
	typedef	MYSQL_FIELD  *(STDCALL *LIBMYSQL_FETCH_FIELD_DIRECT)(MYSQL_RES *, unsigned int);
	typedef	MYSQL_ROW     (STDCALL *LIBMYSQL_FETCH_ROW)(MYSQL_RES *);
	typedef	void          (STDCALL *LIBMYSQL_FREE_RESULT)(MYSQL_RES *);
	typedef	MYSQL        *(STDCALL *LIBMYSQL_INIT)(MYSQL *);
	typedef	unsigned int  (STDCALL *LIBMYSQL_NUM_FIELDS)(MYSQL_RES *);
	typedef	my_ulonglong  (STDCALL *LIBMYSQL_NUM_ROWS)(MYSQL_RES *);
	typedef	int           (STDCALL *LIBMYSQL_PING)(MYSQL *);
	typedef	int           (STDCALL *LIBMYSQL_QUERY)(MYSQL *, const char *);
	typedef	MYSQL        *(STDCALL *LIBMYSQL_REAL_CONNECT)(MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned int);
	typedef	MYSQL_RES    *(STDCALL *LIBMYSQL_USE_RESULT)(MYSQL *);
	typedef	my_ulonglong  (STDCALL *LIBMYSQL_INSERT_ID)(MYSQL *);
	static struct {
		LIBMYSQL_CLOSE              close;
		LIBMYSQL_DATA_SEEK          data_seek;
		LIBMYSQL_ERROR              error;
		LIBMYSQL_FETCH_FIELD_DIRECT fetch_field_direct;
		LIBMYSQL_FETCH_ROW          fetch_row;
		LIBMYSQL_FREE_RESULT        free_result;
		LIBMYSQL_INIT               init;
		LIBMYSQL_NUM_FIELDS         num_fields;
		LIBMYSQL_NUM_ROWS           num_rows;
		LIBMYSQL_PING               ping;
		LIBMYSQL_QUERY              query;
		LIBMYSQL_REAL_CONNECT       real_connect;
		LIBMYSQL_USE_RESULT         use_result;
		LIBMYSQL_INSERT_ID          insert_id;
	} libmysql;
	static MYSQL mysql;
	static MYSQL *mysock=NULL;
#endif
#ifdef HAVE_ODBC
	#include <sql.h>
	#include <sqlext.h>
	static SQLHENV hEnv=NULL;
	static SQLHDBC hDBC=NULL;
	static SQLHSTMT hStmt=NULL;
#endif
#ifdef HAVE_PGSQL
	#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	#include "postgresql/libpq-fe.h"
	#else
	#include "stubs/pgsql.h"
	#endif
	typedef	void           (*LIBPGSQL_CLEAR)(PGresult *);
	typedef	char          *(*LIBPGSQL_ERRORMESSAGE)(const PGconn *);
	typedef	PGresult      *(*LIBPGSQL_EXEC)(PGconn *, const char *);
	typedef	void           (*LIBPGSQL_FINISH)(PGconn *);
	typedef	char          *(*LIBPGSQL_FNAME)(const PGresult *, int);
	typedef	char          *(*LIBPGSQL_GETVALUE)(const PGresult *, int, int);
	typedef	int            (*LIBPGSQL_NFIELDS)(const PGresult *);
	typedef	int            (*LIBPGSQL_NTUPLES)(const PGresult *);
	typedef	ExecStatusType (*LIBPGSQL_RESULTSTATUS)(const PGresult *);
	typedef	PGconn        *(*LIBPGSQL_SETDBLOGIN)(const char *, const char *, const char *, const char *, const char *, const char *, const char *);
	typedef	ConnStatusType (*LIBPGSQL_STATUS)(const PGconn *);
	static struct {
		LIBPGSQL_CLEAR        clear;
		LIBPGSQL_ERRORMESSAGE errormessage;
		LIBPGSQL_EXEC         exec;
		LIBPGSQL_FINISH       finish;
		LIBPGSQL_FNAME        fname;
		LIBPGSQL_GETVALUE     getvalue;
		LIBPGSQL_NFIELDS      nfields;
		LIBPGSQL_NTUPLES      ntuples;
		LIBPGSQL_RESULTSTATUS resultstatus;
		LIBPGSQL_SETDBLOGIN   setdblogin;
		LIBPGSQL_STATUS       status;
	} libpgsql;
	static PGconn *pgconn=NULL;
	static PGresult *pgres=NULL;
#endif
#ifdef HAVE_SQLITE2
	#include "stubs/sqlite2.h"
	typedef	sqlite *(*SQLITE2_OPEN)(const char *, int, char **);
	typedef	int     (*SQLITE2_EXEC)(sqlite *, const char *, sqlite_callback, void *, char **);
	typedef	void    (*SQLITE2_CLOSE)(sqlite *);
	typedef int     (*SQLITE2_LASTID)(sqlite *);
	static struct {
		SQLITE2_OPEN   open;
		SQLITE2_CLOSE  close;
		SQLITE2_EXEC   exec;
		SQLITE2_LASTID lastid;
	} libsqlite2;
	static sqlite *db2;
#endif
#ifdef HAVE_SQLITE3
	#include "stubs/sqlite3.h"
	typedef	int  (*SQLITE3_OPEN)(const char *, sqlite3 **);
	typedef	int  (*SQLITE3_EXEC)(sqlite3 *, const char *, sqlite3_callback, void *, char **);
	typedef	void (*SQLITE3_CLOSE)(sqlite3 *);
	typedef sqlite_int64 (*SQLITE3_LASTID)(sqlite3 *);
	static struct {
		SQLITE3_OPEN   open;
		SQLITE3_CLOSE  close;
		SQLITE3_EXEC   exec;
		SQLITE3_LASTID lastid;
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













































#ifdef HAVE_FBSQL

#ifndef FB_ALIGN

//#include "align.h"
#ifdef VMS
#define FB_ALIGN(n,b)              (n)
#endif

#ifdef sun
#ifdef sparc
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif
#endif

#ifdef hpux
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#ifdef ultrix
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#ifdef sgi
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#ifdef _AIX
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif
 
#ifdef __m88k__
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif
 
#if (defined __osf__ && defined __alpha)
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#ifdef mpexl
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

#if (defined(_MSC_VER) && defined(WIN32)) || (defined(__BORLANDC__) && defined(__WIN32__))
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

//#ifndef ALIGN
#ifndef FB_ALIGN
#define FB_ALIGN(n,b)          ((n+1) & ~1)
#endif

#endif /* FB_ALIGN */

typedef struct vary {
    short          vary_length;
    char           vary_string [1];
} VARY;

/* Define a format string for printf.  Printing of 64-bit integers
   is not standard between platforms */
#ifndef ISC_INT64_FORMAT
#if (defined(_MSC_VER) && defined(WIN32)) || (defined(__BORLANDC__) && defined(__WIN32__))
#define	ISC_INT64_FORMAT	"I64"
#else
#define	ISC_INT64_FORMAT	"ll"
#endif
#endif

static int fbsqlDLLInit(nsp_state *N)
{
	obj_t *confobj=nsp_settable(N, &N->g, "CONFIG");
	char *libpath=nsp_getstr(N, confobj, "lib_path");
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libfbclient.%s", libpath, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libfbclient.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libfbsql.isc_attach_database=(LIBFBSQL_ISC_ATTACH_DATABASE)lib_sym(hinstLib, "isc_attach_database"))==NULL) goto fail;
	if ((libfbsql.isc_dsql_allocate_statement=(LIBFBSQL_ISC_DSQL_ALLOCATE_STATEMENT)lib_sym(hinstLib, "isc_dsql_allocate_statement"))==NULL) goto fail;
	if ((libfbsql.isc_start_transaction=(LIBFBSQL_ISC_START_TRANSACTION)lib_sym(hinstLib, "isc_start_transaction"))==NULL) goto fail;
	if ((libfbsql.isc_dsql_prepare=(LIBFBSQL_ISC_DSQL_PREPARE)lib_sym(hinstLib, "isc_dsql_prepare"))==NULL) goto fail;
	if ((libfbsql.isc_dsql_sql_info=(LIBFBSQL_ISC_DSQL_SQL_INFO)lib_sym(hinstLib, "isc_dsql_sql_info"))==NULL) goto fail;
	if ((libfbsql.isc_dsql_execute=(LIBFBSQL_ISC_DSQL_EXECUTE)lib_sym(hinstLib, "isc_dsql_execute"))==NULL) goto fail;
	if ((libfbsql.isc_vax_integer=(LIBFBSQL_ISC_VAX_INTEGER)lib_sym(hinstLib, "isc_vax_integer"))==NULL) goto fail;
	if ((libfbsql.isc_dsql_describe=(LIBFBSQL_ISC_DSQL_DESCRIBE)lib_sym(hinstLib, "isc_dsql_describe"))==NULL) goto fail;
	if ((libfbsql.isc_commit_transaction=(LIBFBSQL_ISC_COMMIT_TRANSACTION)lib_sym(hinstLib, "isc_commit_transaction"))==NULL) goto fail;
	if ((libfbsql.fb_interpret=(LIBFBSQL_FB_INTERPRET)lib_sym(hinstLib, "fb_interpret"))==NULL) goto fail;
	if ((libfbsql.isc_dsql_free_statement=(LIBFBSQL_ISC_DSQL_FREE_STATEMENT)lib_sym(hinstLib, "isc_dsql_free_statement"))==NULL) goto fail;

	if ((libfbsql.isc_dsql_fetch=(LIBFBSQL_ISC_DSQL_FETCH)lib_sym(hinstLib, "isc_dsql_fetch"))==NULL) goto fail;
	if ((libfbsql.isc_decode_sql_time=(LIBFBSQL_ISC_DECODE_SQL_TIME)lib_sym(hinstLib, "isc_decode_sql_time"))==NULL) goto fail;
	if ((libfbsql.isc_open_blob=(LIBFBSQL_ISC_OPEN_BLOB)lib_sym(hinstLib, "isc_open_blob"))==NULL) goto fail;
	if ((libfbsql.isc_get_segment=(LIBFBSQL_ISC_GET_SEGMENT)lib_sym(hinstLib, "isc_get_segment"))==NULL) goto fail;
	if ((libfbsql.isc_close_blob=(LIBFBSQL_ISC_CLOSE_BLOB)lib_sym(hinstLib, "isc_close_blob"))==NULL) goto fail;
	if ((libfbsql.isc_decode_timestamp=(LIBFBSQL_ISC_DECODE_TIMESTAMP)lib_sym(hinstLib, "isc_decode_timestamp"))==NULL) goto fail;
	if ((libfbsql.isc_decode_sql_date=(LIBFBSQL_ISC_DECODE_SQL_DATE)lib_sym(hinstLib, "isc_decode_sql_date"))==NULL) goto fail;
	if ((libfbsql.isc_detach_database=(LIBFBSQL_ISC_DETACH_DATABASE)lib_sym(hinstLib, "isc_detach_database"))==NULL) goto fail;

	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
	memset((char *)&libfbsql, 0, sizeof(libfbsql));
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return -1;
}

static void print_warning(nsp_state *N, const ISC_STATUS *pvector, const char *__FN__, const char *errstr)
{
	char errbuf[1024];
	int rc;
	int line=0;

	while ((rc=libfbsql.fb_interpret(errbuf, sizeof(errbuf), &pvector))>0) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "%s: %s\r\n", line++==0?"Firebird exception":"                  ", errbuf);
	}
	return;
}

static void fbsqlDisconnect(nsp_state *N)
{
	if (isc_db) {
		ISC_STATUS isc_status[20];

		if (libfbsql.isc_detach_database(isc_status, &isc_db)) {
			print_warning(N, isc_status, __FUNCTION__, "isc_detach_database");
		}
		if (isc_db!=0) {
			isc_db=0;
		}
	}
	sql_is_connected=0;
	return;
}

static int fbsqlConnect(nsp_state *N)
{
	obj_t *confobj=nsp_settable(N, &N->g, "CONFIG");
	char *host, *user, *pass, *db;
	ISC_STATUS isc_status[20];
	char buf[1024];
	char buf2[1024];
	short len=0;
	long rc;

	if(fbsqlDLLInit(N)!=0) return -1;
	if (sql_is_connected) return 0;
	host=nsp_getstr(N, confobj, "sql_hostname");
	user=nsp_getstr(N, confobj, "sql_username");
	pass=nsp_getstr(N, confobj, "sql_password");
	db=nsp_getstr(N, confobj, "sql_dbname");
	if (strlen(user)&&strlen(pass)) {
		len=snprintf(buf, sizeof(buf), "%c%c%c%s%c%c%s", isc_dpb_version1, isc_dpb_user_name, (int)strlen(user), user, isc_dpb_password, (int)strlen(pass), pass);
		snprintf(buf2, sizeof(buf2), "%s:%s", host, db);
		rc=libfbsql.isc_attach_database(isc_status, 0, buf2, &isc_db, len, buf);
	} else {
		snprintf(buf2, sizeof(buf2), "%s:%s", host, db);
		rc=libfbsql.isc_attach_database(isc_status, 0, buf2, &isc_db, 0, NULL);
	}
	if (rc) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "connection error %s - %s\r\n", db, (char *)isc_status);
		return -1;
	}

	sql_is_connected=1;
	return 0;
}

static int fbsql_startquery(nsp_state *N, char *sqlquery)
{
	ISC_STATUS isc_status[20];
	XSQLVAR ISC_FAR *var;
	unsigned short  i;
	short           length, alignment, type, offset;
	char            stmt_info[] = { isc_info_sql_stmt_type };
	char            info_buffer[20];
	short           l;
	long            statement_type=0;

	/* Allocate a global statement */
	if (libfbsql.isc_dsql_allocate_statement(isc_status, &isc_db, &stmt)) {
		print_warning(N, isc_status, __FUNCTION__, "isc_dsql_allocate_statement");
		//printf("isc_dsql_allocate_statement\r\n");
		return 1;
	}

	/* 
	 *    Allocate enough space for 20 fields.  
	 *    If more fields get selected, re-allocate SQLDA later.
	 */
	sqlda = (XSQLDA ISC_FAR *) malloc(XSQLDA_LENGTH (20));
	sqlda->sqln = 20;
	sqlda->version = 1;
	if (!trans) {
		if (libfbsql.isc_start_transaction(isc_status, &trans, 1, &isc_db, 0, NULL)) {
			print_warning(N, isc_status, __FUNCTION__, "isc_start_transaction");
			return 1;
		}
		trans_block=0;
	}
	if (libfbsql.isc_dsql_prepare(isc_status, &trans, &stmt, 0, sqlquery, SQL_DIALECT_V6, sqlda)) {
		print_warning(N, isc_status, __FUNCTION__, "isc_dsql_prepare");
		return 2;
	}
	/* What is the statement type of this statement? 
	**
	** stmt_info is a 1 byte info request.  info_buffer is a buffer
	** large enough to hold the returned info packet
	** The info_buffer returned contains a isc_info_sql_stmt_type in the first byte, 
	** two bytes of length, and a statement_type token.
	*/
	if (!libfbsql.isc_dsql_sql_info(isc_status, &stmt, sizeof (stmt_info), stmt_info, sizeof (info_buffer), info_buffer)) {
		l = (short) libfbsql.isc_vax_integer((char ISC_FAR *) info_buffer + 1, 2);
		statement_type = libfbsql.isc_vax_integer((char ISC_FAR *) info_buffer + 3, l);
	}
	/*
	 *    Execute a non-select statement.
	 */
	if (!sqlda->sqld) {
		if (libfbsql.isc_dsql_execute(isc_status, &trans, &stmt, SQL_DIALECT_V6, NULL)) {
			printf("isc_dsql_execute [%s]\r\n", sqlquery);
			print_warning(N, isc_status, __FUNCTION__, "isc_dsql_execute");
			return 2;
		}
		/* Commit DDL statements if that is what sql_info says */
		if (trans && trans_block==0 && (statement_type == isc_info_sql_stmt_ddl)) {
			if (libfbsql.isc_commit_transaction(isc_status, &trans)) {
				print_warning(N, isc_status, __FUNCTION__, "isc_commit_transaction");
				return 2;
			}
		}
		fetch_stat=100L;
		return 0;
	}
	/*
	 *    Process select statements.
	 */
	numfields = sqlda->sqld;
	/* Need more room. */
	if (sqlda->sqln < numfields) {
		sqlda = (XSQLDA ISC_FAR *) realloc(sqlda, XSQLDA_LENGTH (numfields));
		sqlda->sqln = numfields;
		sqlda->version = 1;
		if (libfbsql.isc_dsql_describe(isc_status, &stmt, SQL_DIALECT_V6, sqlda)) {
			print_warning(N, isc_status, __FUNCTION__, "isc_dsql_describe");
			return 2;
		}
		numfields=sqlda->sqld;
	}
	numtuples=0;
//	cobj=nsp_settable(N, thisobj, "last_query");
//	nsp_setstr(N, cobj, "_query", sqlquery, -1);
//	nsp_setnum(N, cobj, "_fields", conn->numfields);
//	nsp_setstr(N, cobj, "_tuples", "firebird does not support this", -1);
	/*
	 *     Set up SQLDA.
	 */
	for (var = sqlda->sqlvar, offset = 0, i = 0; i < numfields; var++, i++) {
		length = alignment = var->sqllen;
		type = var->sqltype & ~1;
		if (type == SQL_TEXT) {
			alignment = 1;
		} else if (type == SQL_VARYING) {   
			length += sizeof (short) + 1;
			alignment = sizeof (short);
		}
		/*  RISC machines are finicky about word alignment
		**  So the output buffer values must be placed on
		**  word boundaries where appropriate
		*/
		offset = FB_ALIGN(offset, alignment);
		var->sqldata = (char ISC_FAR *) buffer + offset;
		offset += length;
		offset = FB_ALIGN(offset, sizeof (short));
		var->sqlind = (short*) ((char ISC_FAR *) buffer + offset);
		offset += sizeof  (short);
	}
	if (libfbsql.isc_dsql_execute(isc_status, &trans, &stmt, SQL_DIALECT_V6, NULL)) {
		print_warning(N, isc_status, __FUNCTION__, "isc_dsql_execute");
		return 2;
	}
	//nsp_setnum  (N, &N->r, "", 0);
	return 0;
}

static int fbsql_endquery(nsp_state *N)
{
	ISC_STATUS isc_status[20];

	if (stmt) {
		if (libfbsql.isc_dsql_free_statement(isc_status, &stmt, DSQL_drop)) {
			print_warning(N, isc_status, __FUNCTION__, "isc_dsql_free_statement");
		}
	}
	if (fetch_stat != 100L) {
		/* print_warning(N, N, conn->status, __FUNCTION__, "conn->fetch_stat != 100L (are there unfetched rows?)"); */
	}
	fetch_stat=0;
	if (sqlda) {
		free(sqlda);
		sqlda=NULL;
	}
	if (trans && trans_block==0) {
		if (libfbsql.isc_commit_transaction(isc_status, &trans)) {
			print_warning(N, isc_status, __FUNCTION__, "isc_commit_transaction");
			return 0;
		}
		trans=0;
	}
	stmt=0;
	return 0;
}

static void fbsql_store_field(nsp_state *N, XSQLVAR ISC_FAR *var, obj_t *tobj)
{
	ISC_STATUS isc_status[20];

	char  buf[1024];
	char  blob_s[20];
	char  date_s[25];
	short dtype;
	struct tm times;

	char fieldname[64];
	char *ptemp=fieldname;

	strncpy(fieldname, var->aliasname, sizeof(fieldname)-1);
	while (*ptemp) { *ptemp=tolower(*ptemp); ptemp++; }

	dtype=var->sqltype&~1;
	/* Null handling.  If the column is nullable and null */
	if ((var->sqltype&1)&&(*var->sqlind<0)) {
		nsp_setstr(N, tobj, fieldname, NULL, 0);
		return;
	}
	buf[0]='\0';
	switch (dtype) {
	case SQL_TEXT:
		nsp_setstr(N, tobj, fieldname, var->sqldata, var->sqllen);
		return;
	case SQL_VARYING: {
		VARY *vary=(VARY *)var->sqldata;

		nsp_setstr(N, tobj, fieldname, vary->vary_string, vary->vary_length);
		return;
	}
	case SQL_SHORT:
	case SQL_LONG:
	case SQL_INT64: {
		ISC_INT64 value=0;
		short     field_width=0;
		short     dscale;

		switch (dtype) {
		case SQL_SHORT:
			value = (ISC_INT64) *(short ISC_FAR *) var->sqldata;
			field_width = 6;
			break;
		case SQL_LONG:
			// breaks on Linux x86_64 due to wrong assumption about size of long
			//value = (ISC_INT64) *(long ISC_FAR *) var->sqldata;
			value = (ISC_INT64) *(int ISC_FAR *) var->sqldata;
			field_width = 11;
			break;
		case SQL_INT64:
			value = (ISC_INT64) *(ISC_INT64 ISC_FAR *) var->sqldata;
			field_width = 21;
			break;
		}
		dscale = var->sqlscale;
		if (dscale < 0) {
			ISC_INT64 tens;
			short i;

			tens = 1;
			for (i = 0; i > dscale; i--)
				tens *= 10;
			if (value >= 0)
				sprintf (buf, "%*" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d", field_width - 1 + dscale, (ISC_INT64) value / tens, -dscale, (ISC_INT64) value % tens);
			else if ((value / tens) != 0)
				sprintf (buf, "%*" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d", field_width - 1 + dscale, (ISC_INT64) (value / tens), -dscale, (ISC_INT64) -(value % tens));
			else
				sprintf (buf, "%*s.%0*" ISC_INT64_FORMAT "d", field_width - 1 + dscale, "-0", -dscale, (ISC_INT64) -(value % tens));
		} else if (dscale) {
			sprintf (buf, "%*" ISC_INT64_FORMAT "d%0*d", field_width, (ISC_INT64) value, dscale, 0);
		} else {
			sprintf (buf, "%" ISC_INT64_FORMAT "d", (ISC_INT64) value);
		}
		nsp_setstr(N, tobj, fieldname, buf, -1);
		return;
	}
	case SQL_FLOAT:
		sprintf(buf, "%15g", *(float ISC_FAR *) (var->sqldata));
		nsp_setstr(N, tobj, fieldname, buf, -1);
		return;
	case SQL_DOUBLE:
		sprintf(buf, "%24f", *(double ISC_FAR *) (var->sqldata));
		nsp_setstr(N, tobj, fieldname, buf, -1);
		return;
	case SQL_TIMESTAMP:
		libfbsql.isc_decode_timestamp((ISC_TIMESTAMP ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%04d-%02d-%02d %02d:%02d:%02d",
			times.tm_year + 1900,
			times.tm_mon+1,
			times.tm_mday,
			times.tm_hour,
			times.tm_min,
			times.tm_sec
		);
		nsp_setstr(N, tobj, fieldname, date_s, -1);
		return;
	case SQL_TYPE_DATE:
		libfbsql.isc_decode_sql_date((ISC_DATE ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%04d-%02d-%02d",
			times.tm_year + 1900,
			times.tm_mon+1,
			times.tm_mday);
		nsp_setstr(N, tobj, fieldname, date_s, -1);
		return;
	case SQL_TYPE_TIME:
		libfbsql.isc_decode_sql_time((ISC_TIME ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%02d:%02d:%02d",
			times.tm_hour,
			times.tm_min,
			times.tm_sec
		);
		nsp_setstr(N, tobj, fieldname, date_s, -1);
		return;
	case SQL_BLOB: {
		//int bloblen=0;
		ISC_QUAD        blob_id=*(ISC_QUAD ISC_FAR *)var->sqldata;
		isc_blob_handle blob_handle = NULL;
		short           blob_seg_len;
		char            blob_segment[256];
		long            blob_stat;
		obj_t *cobj;

		cobj=nsp_setstr(N, tobj, fieldname, "NULL", 0);
		/* Open the blob with the fetched blob_id. */
		if (libfbsql.isc_open_blob(isc_status, &isc_db, &trans, &blob_handle, &blob_id)) {
			print_warning(N, isc_status, __FUNCTION__, "isc_open_blob");
		}
		/* Get blob segments and their lengths and print each segment. */
		blob_stat = libfbsql.isc_get_segment(isc_status, &blob_handle, (unsigned short *)&blob_seg_len, sizeof(blob_segment), blob_segment);
		while (blob_stat == 0 || isc_status[1] == isc_segment) {
			nsp_strcat(N, cobj, blob_segment, blob_seg_len);
			blob_stat = libfbsql.isc_get_segment(isc_status, &blob_handle, (unsigned short *)&blob_seg_len, sizeof(blob_segment), blob_segment);
		}
		/* Close the blob.  Should be blob_stat to check */
		if (isc_status[1] == isc_segstr_eof) {
			if (libfbsql.isc_close_blob(isc_status, &blob_handle)) {
				print_warning(N, isc_status, __FUNCTION__, "isc_close_blob");
			}
		}
		log_error(N, "sql", __FILE__, __LINE__, 6, "blob");
		return;
	}
	case SQL_ARRAY: {
		/* Print the blob id on blobs or arrays */
		ISC_QUAD  bid=*(ISC_QUAD ISC_FAR *)var->sqldata;

		sprintf(blob_s, "%08x:%08x", (unsigned int)bid.isc_quad_high, (unsigned int)bid.isc_quad_low);
		sprintf(buf, "%17s", blob_s);
		break;
	}
	default:
		printf("Unhandled type!");
		break;
	}
	nsp_setstr(N, tobj, fieldname, buf, -1);
	return;
}

static int fbsqlUpdate(nsp_state *N, char *sqlquery)
{
	fbsql_startquery(N, sqlquery);
	fbsql_endquery(N);
	return 0;
}

static int fbsqlQuery(nsp_state *N, obj_t *qobj, char *sqlquery)
{
	ISC_STATUS isc_status[20];
	obj_t *cobj, *robj, *tobj;
	unsigned int field, numtuples;
	char name[8];

	fbsql_startquery(N, sqlquery);
	numtuples=0;
	nsp_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nsp_setnum(NULL, qobj, "_fields", numfields);
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nsp_settable(NULL, qobj, "_rows");
	/* now to populate the cursor */
	for (numtuples=0;;numtuples++) {
		if ((fetch_stat=libfbsql.isc_dsql_fetch(isc_status, &stmt, SQL_DIALECT_V6, sqlda))!=0) break;
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", numtuples);
		tobj=nsp_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			fbsql_store_field(N, (XSQLVAR ISC_FAR *)&sqlda->sqlvar[field], tobj);
		}
	}
	fbsql_endquery(N);
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	return 0;
}
#endif /* HAVE_FBSQL */

























#ifdef HAVE_MYSQL
static int mysqlDLLInit(nsp_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysql.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.15", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.14", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", LIBEXT);
	goto fail;
found:
	if ((libmysql.close              = lib_sym(hinstLib, "mysql_close"))==NULL) goto fail;
	if ((libmysql.data_seek          = lib_sym(hinstLib, "mysql_data_seek"))==NULL) goto fail;
	if ((libmysql.error              = lib_sym(hinstLib, "mysql_error"))==NULL) goto fail;
	if ((libmysql.fetch_field_direct = lib_sym(hinstLib, "mysql_fetch_field_direct"))==NULL) goto fail;
	if ((libmysql.fetch_row          = lib_sym(hinstLib, "mysql_fetch_row"))==NULL) goto fail;
	if ((libmysql.free_result        = lib_sym(hinstLib, "mysql_free_result"))==NULL) goto fail;
	if ((libmysql.init               = lib_sym(hinstLib, "mysql_init"))==NULL) goto fail;
	if ((libmysql.num_fields         = lib_sym(hinstLib, "mysql_num_fields"))==NULL) goto fail;
	if ((libmysql.num_rows           = lib_sym(hinstLib, "mysql_num_rows"))==NULL) goto fail;
	if ((libmysql.ping               = lib_sym(hinstLib, "mysql_ping"))==NULL) goto fail;
	if ((libmysql.query              = lib_sym(hinstLib, "mysql_query"))==NULL) goto fail;
	if ((libmysql.real_connect       = lib_sym(hinstLib, "mysql_real_connect"))==NULL) goto fail;
	if ((libmysql.use_result         = lib_sym(hinstLib, "mysql_use_result"))==NULL) goto fail;
	if ((libmysql.insert_id          = lib_sym(hinstLib, "mysql_insert_id"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libmysql, 0, sizeof(libmysql));
	if (hinstLib!=NULL) lib_close(hinstLib);
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

static int mysqlConnect(nsp_state *N)
{
	obj_t *confobj=nsp_settable(N, &N->g, "CONFIG");
	char *libpath=nsp_getstr(N, confobj, "lib_path");
	char *host, *user, *pass, *db;
	int port;

	if(mysqlDLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	host=nsp_getstr(N, confobj, "sql_hostname");
	user=nsp_getstr(N, confobj, "sql_username");
	pass=nsp_getstr(N, confobj, "sql_password");
	db=nsp_getstr(N, confobj, "sql_dbname");
	port=(int)nsp_getnum(N, confobj, "sql_port");
	libmysql.init(&mysql);
	if (!(mysock=libmysql.real_connect(&mysql, host, user, pass, db, port, NULL, 0))) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int mysqlUpdate(nsp_state *N, char *sqlquery)
{
	int rc;

	if (libmysql.query(mysock, sqlquery)) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		log_error(N, "sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	rc=(int)libmysql.insert_id(mysock);
	return rc;
}

static int mysqlQuery(nsp_state *N, obj_t *qobj, char *sqlquery)
{
	MYSQL_RES *myres;
	MYSQL_ROW MYrow;
	MYSQL_FIELD *MYfield;
	obj_t *cobj, *robj, *tobj;
	char *p;
	char name[8];
	unsigned int field, numfields, numtuples;

	if (libmysql.ping(mysock)!=0) {
		return -1;
	}
	if (libmysql.query(mysock, sqlquery)) {
		cobj=nsp_setstr(NULL, qobj, "_error", libmysql.error(&mysql), -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", nsp_tostr(NULL, cobj));
		log_error(N, "sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.use_result(mysock))) {
		cobj=nsp_setstr(NULL, qobj, "_error", libmysql.error(&mysql), -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "MYSQL error: %s", nsp_tostr(NULL, cobj));
		return -1;
	}
	numfields=(int)libmysql.num_fields(myres);
	numtuples=0;
	nsp_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nsp_setnum(NULL, qobj, "_fields", numfields);
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nsp_settable(NULL, qobj, "_rows");
	for (numtuples=0;;numtuples++) {
		if ((MYrow=libmysql.fetch_row(myres))==NULL) break;
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", numtuples);
		tobj=nsp_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			p=MYrow[field]?MYrow[field]:"NULL";
			MYfield=libmysql.fetch_field_direct(myres, field);
			nsp_setstr(NULL, tobj, MYfield->name, p, strlen(p));
		}
	}
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
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

static int odbcConnect(nsp_state *N)
{
	SQLCHAR szConnStr[255];
	SWORD cbConnStr;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];

	if (sql_is_connected) return 0;
	rc=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, cnf.sql_odbc_dsn, (short int)strlen(cnf.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int odbcUpdate(nsp_state *N, char *sqlquery)
{
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Update - SQLExecDirect %s", buf);
		log_error(N, "sql", __FILE__, __LINE__, 2, "ODBC UPDATE: [%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		hStmt=NULL;
		return -1;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
}

static int odbcQuery(nsp_state *N, obj_t *qobj, char *sqlquery)
{
	SQLSMALLINT pccol;
	SDWORD slen;
	RETCODE rc;
	char sqlstate[15];
	char buf[250];
	char *column;
	unsigned int field;
	unsigned int rowsalloc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Query - SQLExecDirect %s", buf);
		log_error(N, "sql", __FILE__, __LINE__, 2, "ODBC SELECT: [%s]", sqlquery);
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		hStmt=NULL;
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error(N, "sql", __FILE__, __LINE__, 1, "ODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	numfields=pccol;
	numtuples=0;
	nsp_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nsp_setnum(NULL, qobj, "_fields", numfields);
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nsp_settable(NULL, qobj, "_rows");
	for (numtuples=0;;numtuples++) {
		rc=SQLFetch(conn->hSTMT);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) break;
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", numtuples);
		tobj=nsp_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			rc=SQLDescribeCol(conn->hSTMT, (SQLSMALLINT)(field+1), (SQLPOINTER)colname, MAX_OBJNAMELEN, NULL, NULL, NULL, NULL, NULL);
			cobj=nsp_setstr(NULL, tobj, colname, NULL, 0);
			/* isn't there a way to peek at the size instead of faking a get? */
			rc=SQLGetData(conn->hSTMT, (SQLUSMALLINT)(field+1), SQL_C_CHAR, NULL, 0, &slen);
			if (slen>0) {
				cobj->val->size=slen;
				cobj->val->d.str=n_alloc(N, cobj->val->size+1, 0);
				if (cobj->val->d.str==NULL) log_error(N, "sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				rc=SQLGetData(conn->hSTMT, (SQLUSMALLINT)(field+1), SQL_C_CHAR, cobj->val->d.str, cobj->val->size+1, &slen);
				cobj->val->d.str[cobj->val->size]=0;
			}
		}
	}
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	hStmt=NULL;
	return 0;
}
#endif /* HAVE_ODBC */

#ifdef HAVE_PGSQL
static int pgsqlDLLInit(nsp_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libpq.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libpq.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libpgsql.clear        = lib_sym(hinstLib, "PQclear"))==NULL) goto fail;
	if ((libpgsql.errormessage = lib_sym(hinstLib, "PQerrorMessage"))==NULL) goto fail;
	if ((libpgsql.exec         = lib_sym(hinstLib, "PQexec"))==NULL) goto fail;
	if ((libpgsql.finish       = lib_sym(hinstLib, "PQfinish"))==NULL) goto fail;
	if ((libpgsql.fname        = lib_sym(hinstLib, "PQfname"))==NULL) goto fail;
	if ((libpgsql.getvalue     = lib_sym(hinstLib, "PQgetvalue"))==NULL) goto fail;
	if ((libpgsql.nfields      = lib_sym(hinstLib, "PQnfields"))==NULL) goto fail;
	if ((libpgsql.ntuples      = lib_sym(hinstLib, "PQntuples"))==NULL) goto fail;
	if ((libpgsql.resultstatus = lib_sym(hinstLib, "PQresultStatus"))==NULL) goto fail;
	if ((libpgsql.setdblogin   = lib_sym(hinstLib, "PQsetdbLogin"))==NULL) goto fail;
	if ((libpgsql.status       = lib_sym(hinstLib, "PQstatus"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	if (hinstLib!=NULL) lib_close(hinstLib);
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

static int pgsqlConnect(nsp_state *N)
{
	obj_t *confobj=nsp_settable(N, &N->g, "CONFIG");
	char *libpath=nsp_getstr(N, confobj, "lib_path");
	char *host, *user, *pass, *db, *port;

	if(pgsqlDLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	host=nsp_getstr(N, confobj, "sql_hostname");
	user=nsp_getstr(N, confobj, "sql_username");
	pass=nsp_getstr(N, confobj, "sql_password");
	db=nsp_getstr(N, confobj, "sql_dbname");
	port=nsp_getstr(N, confobj, "sql_port");
	pgconn=libpgsql.setdblogin(host, port, NULL, NULL, db, user, pass);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int pgsqlUpdate(nsp_state *N, char *sqlquery)
{
	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL error: %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);
	pgres=NULL;
	return 0;
}

static int pgsqlQuery(nsp_state *N, obj_t *qobj, char *sqlquery)
{
	char *q;
	obj_t *robj, *tobj;
	char *p;
	char name[8];
	unsigned int field, tuple;
	unsigned int numfields, numtuples;

	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		libpgsql.clear(pgres);
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);

	if ((q=calloc(1, strlen(sqlquery)+42))==NULL) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "calloc error");
		pgsqlDisconnect();
		return -1;
	}
	sprintf(q, "DECLARE myportal CURSOR FOR %s", sqlquery);
	pgres = libpgsql.exec(pgconn, q);
	free(q);

	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		libpgsql.clear(pgres);
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);

	pgres = libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		libpgsql.clear(pgres);
		pgsqlDisconnect();
		return -1;
	}
	numfields=libpgsql.nfields(pgres);
	numtuples=libpgsql.ntuples(pgres);
	nsp_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	nsp_setnum(NULL, qobj, "_fields", numfields);
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	robj=nsp_settable(NULL, qobj, "_rows");
	for (tuple=0;tuple<numtuples;tuple++) {
		memset(name, 0, sizeof(name));
		sprintf(name, "%d", tuple);
		tobj=nsp_settable(NULL, robj, name);
		tobj->val->attr&=~NST_AUTOSORT;
		for (field=0;field<numfields;field++) {
			p=libpgsql.getvalue(pgres, tuple, field);
			p=p?p:"NULL";
			nsp_setstr(NULL, tobj, libpgsql.fname(pgres, field), p, strlen(p));
		}
	}
	nsp_setnum(NULL, qobj, "_tuples", numtuples);
	libpgsql.clear(pgres);

	pgres = libpgsql.exec(pgconn, "CLOSE myportal");
	libpgsql.clear(pgres);

	pgres = libpgsql.exec(pgconn, "END");
	libpgsql.clear(pgres);

	pgres=NULL;
	return 0;
}
#endif /* HAVE_PGSQL */

#ifdef HAVE_SQLITE2
static int SQLite2DLLInit(nsp_state *N, char *libdir)
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite2.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite2.%s", libdir, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libsqlite2.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "sqlite2.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libsqlite2.open   = (SQLITE2_OPEN)  lib_sym(hinstLib, "sqlite_open"))==NULL) goto fail;
	if ((libsqlite2.exec   = (SQLITE2_EXEC)  lib_sym(hinstLib, "sqlite_exec"))==NULL) goto fail;
	if ((libsqlite2.close  = (SQLITE2_CLOSE) lib_sym(hinstLib, "sqlite_close"))==NULL) goto fail;
	if ((libsqlite2.lastid = (SQLITE2_LASTID)lib_sym(hinstLib, "sqlite_last_insert_rowid"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
	if (hinstLib!=NULL) lib_close(hinstLib);
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

static int sqlite2Connect(nsp_state *N)
{
	char *libpath=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "lib_path");
	char *varpath=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "var_path");
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite2DLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/db/%s.db2", varpath, SERVER_BASENAME);
	fixslashes(dbname);
	if ((db2=libsqlite2.open(dbname, 0, &zErrMsg))==0) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite Connect - %s", zErrMsg);
		return -1;
	}
	sql_is_connected=1;
	libsqlite2.exec(db2, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
	libsqlite2.exec(db2, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqlite2Update(nsp_state *N, char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=10;

//retry:
	rc=libsqlite2.exec(db2, sqlquery, NULL, 0, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		rc=(int)libsqlite2.lastid(db2);
		return rc;
	case SQLITE_BUSY:
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite busy error?: %s", zErrMsg);
		//if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	case SQLITE_CORRUPT:
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite corrupt error?: %s", zErrMsg);
		//if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	if (rc!=SQLITE_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite2Callback(void *vptr, int argc, char **argv, char **azColName)
{
	obj_t *qobj=vptr;
	obj_t *tobj;
	char name[8];
	unsigned int field;
	unsigned int numfields;
	unsigned int numtuples;
	char *p;

	numfields=argc;
	numtuples=(int)nsp_getnum(NULL, qobj, "_tuples");
	tobj=nsp_getobj(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	memset(name, 0, sizeof(name));
	sprintf(name, "%d", numtuples);
	/* get pointer to this record table */
	tobj=nsp_settable(NULL, tobj, name);
	if (numtuples==0) nsp_setnum(NULL, qobj, "_fields", numfields);
	for (field=0;field<numfields;field++) {
		if (argv==NULL) continue;
		p=argv[field]?argv[field]:"NULL";
		nsp_setstr(NULL, tobj, azColName[field], p, strlen(p));
	}
	if (argv!=NULL) nsp_setnum(NULL, qobj, "_tuples", numtuples+1);
	return 0;
}

static int sqlite2Query(nsp_state *N, obj_t *qobj, char *sqlquery)
{
	obj_t *tobj;
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	tobj=nsp_settable(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	nsp_setnum(NULL, qobj, "_fields", 0);
	nsp_setnum(NULL, qobj, "_tuples", 0);
//retry:
	rc=libsqlite2.exec(db2, sqlquery, sqlite2Callback, qobj, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: busy or corrupt %d %s", rc, zErrMsg);
		//if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: unknown error %d %s", rc, zErrMsg);
		break;
	}
	log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
	log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE2 */

#ifdef HAVE_SQLITE3
static int SQLite3DLLInit(nsp_state *N, char *libdir)
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
	if ((libsqlite3.open   = (SQLITE3_OPEN)  lib_sym(hinstLib, "sqlite3_open"))==NULL) goto fail;
	if ((libsqlite3.exec   = (SQLITE3_EXEC)  lib_sym(hinstLib, "sqlite3_exec"))==NULL) goto fail;
	if ((libsqlite3.close  = (SQLITE3_CLOSE) lib_sym(hinstLib, "sqlite3_close"))==NULL) goto fail;
	if ((libsqlite3.lastid = (SQLITE3_LASTID)lib_sym(hinstLib, "sqlite3_last_insert_rowid"))==NULL) goto fail;
	return 0;
fail:
	log_error(N, "sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
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

static int sqlite3Connect(nsp_state *N)
{
	char *libpath=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "lib_path");
	char *varpath=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "var_path");
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite3DLLInit(N, libpath)!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/db/%s.db3", varpath, SERVER_BASENAME);
	fixslashes(dbname);
	if (libsqlite3.open(dbname, &db3)!=SQLITE_OK) {
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite Connect - %s", zErrMsg);
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

static int sqlite3Update(nsp_state *N, char *sqlquery)
{
//	obj_t *cobj;
	char *zErrMsg=0;
	int rc;
//	short int retries=10;

//retry:
	rc=libsqlite3.exec(db3, sqlquery, NULL, 0, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		rc=(int)libsqlite3.lastid(db3);
		return rc;
	case SQLITE_BUSY:
//		cobj=nsp_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite busy error?: %s", zErrMsg);
//		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	case SQLITE_CORRUPT:
//		cobj=nsp_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite corrupt error?: %s", zErrMsg);
//		if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		break;
	}
	if (rc!=SQLITE_OK) {
//		cobj=nsp_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
		log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite3Callback(void *vptr, int argc, char **argv, char **azColName)
{
	obj_t *qobj=vptr;
	obj_t *tobj;
	char name[8];
	unsigned int field;
	unsigned int numfields;
	unsigned int numtuples;
	char *p;

	numfields=argc;
	numtuples=(int)nsp_getnum(NULL, qobj, "_tuples");
	tobj=nsp_getobj(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	memset(name, 0, sizeof(name));
	sprintf(name, "%d", numtuples);
	/* get pointer to this record table */
	tobj=nsp_settable(NULL, tobj, name);
	tobj->val->attr&=~NST_AUTOSORT;
	if (numtuples==0) nsp_setnum(NULL, qobj, "_fields", numfields);
	for (field=0;field<numfields;field++) {
		if (argv==NULL) continue;
		p=argv[field]?argv[field]:"NULL";
		nsp_setstr(NULL, tobj, azColName[field], p, strlen(p));
	}
	if (argv!=NULL) nsp_setnum(NULL, qobj, "_tuples", numtuples+1);
	return 0;
}

static int sqlite3Query(nsp_state *N, obj_t *qobj, char *sqlquery)
{
	obj_t *cobj, *tobj;
	char *zErrMsg=0;
	int rc;
	short int retries=10;

	nsp_setstr(NULL, qobj, "_query", sqlquery, strlen(sqlquery));
	tobj=nsp_settable(NULL, qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return -1;
	nsp_setnum(NULL, qobj, "_fields", 0);
	nsp_setnum(NULL, qobj, "_tuples", 0);
//retry:
	rc=libsqlite3.exec(db3, sqlquery, sqlite3Callback, qobj, &zErrMsg);
	switch (rc) {
	case SQLITE_OK:
		return 0;
	case SQLITE_BUSY:
	case SQLITE_CORRUPT:
		cobj=nsp_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: busy or corrupt %d %s", rc, nsp_tostr(NULL, cobj));
		//if (retries>0) { retries--; msleep(5); goto retry; }
		break;
	default:
		cobj=nsp_setstr(NULL, qobj, "_error", zErrMsg, -1);
		log_error(N, "sql", __FILE__, __LINE__, 2, "SQLite: unknown error %d %s", rc, nsp_tostr(NULL, cobj));
		break;
	}
	cobj=nsp_setstr(NULL, qobj, "_error", zErrMsg, -1);
	log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, nsp_tostr(NULL, cobj));
	log_error(N, "sql", __FILE__, __LINE__, 1, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE3 */

static int _sql_dll_unload()
{
#ifdef HAVE_FBSQL
	memset((char *)&libfbsql, 0, sizeof(libfbsql));
#endif
#ifdef HAVE_MYSQL
	memset((char *)&libmysql, 0, sizeof(libmysql));
#endif
#ifdef HAVE_PGSQL
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
#endif
#ifdef HAVE_SQLITE2
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
#endif
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
	char *sqltype=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "sql_server_type");

	log_error(N, "sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if (strcmp(sqltype, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		fbsqlDisconnect(N);
#endif
	} else if (strcmp(sqltype, "MYSQL")==0) {
#ifdef HAVE_MYSQL
		mysqlDisconnect();
#endif
	} else if (strcmp(sqltype, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(sqltype, "PGSQL")==0) {
#ifdef HAVE_PGSQL
		pgsqlDisconnect();
#endif
	} else if (strcmp(sqltype, "SQLITE2")==0) {
#ifdef HAVE_SQLITE2
		sqlite2Disconnect();
#endif
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		sqlite3Disconnect();
#endif
	}
	_sql_dll_unload();
	return;
}

void _sql_freeresult(nsp_state *N, obj_t **qobj)
{
	if (*qobj!=NULL) {
		nsp_unlinkval(N, *qobj);
		free(*qobj);
		*qobj=NULL;
	} else {
		log_error(N, "sql", __FILE__, __LINE__, 1, "freeing an unused sql result?");
	}
	log_error(N, "sql", __FILE__, __LINE__, 4, "SQL query [0x%08X] freed", qobj);
	return;
}

int _sql_update(nsp_state *N, char *sqlquery)
{
	char *sqltype=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "sql_server_type");
	int rc=-1;

	log_error(N, "sql", __FILE__, __LINE__, 2, "SQL update: %s", sqlquery);
	if (strcmp(sqltype, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		if (fbsqlConnect(N)<0) return -1;
		rc=fbsqlUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "MYSQL")==0) {
#ifdef HAVE_MYSQL
		if (mysqlConnect(N)<0) return -1;
		rc=mysqlUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect(N)<0) return -1;
		rc=odbcUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "PGSQL")==0) {
#ifdef HAVE_PGSQL
		if (pgsqlConnect(N)<0) return -1;
		rc=pgsqlUpdate(N, sqlquery);
#endif
	} else if (strcmp(sqltype, "SQLITE2")==0) {
#ifdef HAVE_SQLITE2
		if (sqlite2Connect(N)<0) return -1;
		rc=sqlite2Update(N, sqlquery);
#endif
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		if (sqlite3Connect(N)<0) return -1;
		rc=sqlite3Update(N, sqlquery);
#endif
	}
/*	log_error(N, "sql", __FILE__, __LINE__, 1, "lastid = %d", rc); */
	return rc;
}

int _sql_query(nsp_state *N, obj_t **qobj, char *query)
{
	char *sqltype=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "sql_server_type");
	obj_t *tobj;
	int rc=-1;

	if (*qobj==NULL) {
		tobj=*qobj=calloc(1, sizeof(obj_t));
		nsp_linkval(N, tobj, NULL);
		tobj->val->type=NT_TABLE;
		tobj->val->attr&=~NST_AUTOSORT;
	}
	log_error(N, "sql", __FILE__, __LINE__, 3, "SQL query: [0x%08X] %s", *qobj, query);
	if (strcmp(sqltype, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		if (fbsqlConnect(N)<0) 	return -1;
		if ((rc=fbsqlQuery(N, *qobj, query))<0) fbsqlDisconnect(N);
#endif
	} else if (strcmp(sqltype, "MYSQL")==0) {
#ifdef HAVE_MYSQL
		if (mysqlConnect(N)<0) 	return -1;
		if ((rc=mysqlQuery(N, *qobj, query))<0) mysqlDisconnect();
#endif
	} else if (strcmp(sqltype, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect(N)<0) return -1;
		if ((rc=odbcQuery(N, *qobj, query))<0) odbcDisconnect();
		}
#endif
	} else if (strcmp(sqltype, "PGSQL")==0) {
#ifdef HAVE_PGSQL
		if (pgsqlConnect(N)<0) return -1;
		if ((rc=pgsqlQuery(N, *qobj, query))<0) pgsqlDisconnect();
#endif
	} else if (strcmp(sqltype, "SQLITE2")==0) {
#ifdef HAVE_SQLITE2
		if (sqlite2Connect(N)<0) return -1;
		if ((rc=sqlite2Query(N, *qobj, query))<0) sqlite2Disconnect();
#endif
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
#ifdef HAVE_SQLITE3
		if (sqlite3Connect(N)<0) return -1;
		if ((rc=sqlite3Query(N, *qobj, query))<0) sqlite3Disconnect();
#endif
	}
	return rc;
}

int _sql_updatef(nsp_state *N, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery=calloc(8192, sizeof(char)))==NULL) {
		log_error(N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
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
		log_error(N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc=_sql_query(N, qobj, sqlquery);
	free(sqlquery);
	return rc;
}

int _sql_getsequence(nsp_state *N, char *table)
{
	char *sqltype=nsp_getstr(N, nsp_settable(N, &N->g, "CONFIG"), "sql_server_type");
	obj_t *qobj=NULL;
	int rc=-1;

	if (strcmp(sqltype, "FBSQL")==0) {
//		if (_sql_queryf(N, &qobj, "SELECT GEN_ID(gen_%s, 1) FROM RDB$DATABASE;", table)<0) return -1;
		if (_sql_queryf(N, &qobj, "SELECT NEXT VALUE FOR gen_%s FROM RDB$DATABASE;", table)<0) return -1;
		if (sql_numtuples(N, &qobj)>0) rc=atoi(sql_getvalue(N, &qobj, 0, 0));
		_sql_freeresult(N, &qobj);
	} else if (strcmp(sqltype, "MYSQL")==0) {
	} else if (strcmp(sqltype, "ODBC")==0) {
	} else if (strcmp(sqltype, "PGSQL")==0) {
	} else if (strcmp(sqltype, "SQLITE2")==0) {
	} else if ((strcmp(sqltype, "SQLITE3")==0)||(strcmp(sqltype, "SQLITE")==0)) {
	}
	return rc;
}

char *sql_getname(nsp_state *N, obj_t **qobj, int fieldnumber)
{
	obj_t *cobj;
	int j=0;

	if ((fieldnumber<0)||(fieldnumber+1>nsp_getnum(N, *qobj, "_fields"))) return NULL;
	cobj=nsp_getobj(N, *qobj, "_rows");
	if (cobj->val->type!=NT_TABLE) return "";
	for (cobj=cobj->val->d.table.f; cobj; cobj=cobj->next) {
		if (cobj->val->type==NT_NULL) return "";
		if (cobj->val->type!=NT_TABLE) continue;
		for (cobj=cobj->val->d.table.f; cobj; cobj=cobj->next) {
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

	if ((tuple<0)||(tuple+1>nsp_getnum(N, *qobj, "_tuples"))) return NULL;
	if ((field<0)||(field+1>nsp_getnum(N, *qobj, "_fields"))) return NULL;
	tobj=nsp_getobj(N, *qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return "";
	for (cobj=tobj->val->d.table.f; cobj; cobj=cobj->next) {
		if (cobj->val->type==NT_NULL) return "";
		if (cobj->val->type!=NT_TABLE) continue;
		if (i!=tuple) { i++; continue; }
		for (cobj=cobj->val->d.table.f; cobj; cobj=cobj->next) {
			if (j!=field) { j++; continue; }
			if (cobj->val->type==NT_STRING) {
				return cobj->val->d.str?cobj->val->d.str:"";
			} else {
				return nsp_tostr(N, cobj);
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

	if ((tuple<0)||(tuple+1>nsp_getnum(N, *qobj, "_tuples"))) return NULL;
	tobj=nsp_getobj(N, *qobj, "_rows");
	if (tobj->val->type!=NT_TABLE) return "";
	for (cobj=tobj->val->d.table.f; cobj; cobj=cobj->next) {
		if (cobj->val->type==NT_NULL) return "";
		if (cobj->val->type!=NT_TABLE) continue;
		if (i!=tuple) { i++; continue; }
		cobj=nsp_getobj(N, cobj, fieldname);
		if (cobj->val->type==NT_STRING) {
			return cobj->val->d.str?cobj->val->d.str:"";
		} else {
			return nsp_tostr(N, cobj);
		}
	}
	return NULL;
}

int sql_numfields(nsp_state *N, obj_t **qobj)
{
	return (int)nsp_getnum(N, *qobj, "_fields");
}

int sql_numtuples(nsp_state *N, obj_t **qobj)
{
	return (int)nsp_getnum(N, *qobj, "_tuples");
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
