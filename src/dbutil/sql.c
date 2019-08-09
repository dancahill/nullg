/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#else
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif

void *lib_open(const char *file)
{
#ifdef WIN32
	return LoadLibrary(file);
#else
#ifdef HAVE_DLFCN_H
	return dlopen(file, RTLD_NOW);
#else
	return NULL;
#endif
#endif
}

void *lib_sym(void *handle, const char *name)
{
#ifdef WIN32
	return GetProcAddress(handle, name);
#else
#ifdef HAVE_DLFCN_H
	return dlsym(handle, name);
#else
	return NULL;
#endif
#endif
}

int lib_close(void *handle)
{
#ifdef WIN32
	return FreeLibrary(handle);
#else
#ifdef HAVE_DLFCN_H
	return dlclose(handle);
#else
	return 0;
#endif
#endif
}

#ifdef WIN32
#define sleep(x) Sleep(x*1000)
#define msleep(x) Sleep(x)
#define LIBEXT "dll"
#else
#define msleep(x) usleep(x*1000)
#define LIBEXT "so"
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

static int fbsqlDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libfbclient.%s", config.dir_lib, LIBEXT);
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

static void print_warning(const ISC_STATUS *pvector, const char *__FN__, const char *errstr)
{
	char errbuf[1024];
	int rc;
	int line=0;

	while ((rc=libfbsql.fb_interpret(errbuf, sizeof(errbuf), &pvector))>0) {
		printf("%s: %s\r\n", line++==0?"Firebird exception":"                  ", errbuf);
	}
	return;
}

static void fbsqlDisconnect()
{
	if (isc_db) {
		ISC_STATUS isc_status[20];

		if (libfbsql.isc_detach_database(isc_status, &isc_db)) {
			print_warning(isc_status, __FUNCTION__, "isc_detach_database");
		}
		if (isc_db!=0) {
			isc_db=0;
		}
	}
	sql_is_connected=0;
	return;
}

static int fbsqlConnect()
{
	ISC_STATUS isc_status[20];
	char port[12];

	char buf[1024];
	char buf2[1024];
	short len=0;
	long rc;

	if(fbsqlDLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	memset (port, 0, sizeof(port));
	snprintf(port, sizeof(port)-1, "%d", config.sql_port);

	if (strlen(config.sql_username)&&strlen(config.sql_password)) {
		len=snprintf(buf, sizeof(buf), "%c%c%c%s%c%c%s", isc_dpb_version1, isc_dpb_user_name, (int)strlen(config.sql_username), config.sql_username, isc_dpb_password, (int)strlen(config.sql_password), config.sql_password);
		snprintf(buf2, sizeof(buf2), "%s:%s", config.sql_hostname, config.sql_dbname);
		rc=libfbsql.isc_attach_database(isc_status, 0, buf2, &isc_db, len, buf);
	} else {
		snprintf(buf2, sizeof(buf2), "%s:%s", config.sql_hostname, config.sql_dbname);
		rc=libfbsql.isc_attach_database(isc_status, 0, buf2, &isc_db, 0, NULL);
	}
	if (rc) {
		printf("connection error %s - %s\r\n", config.sql_dbname, (char *)isc_status);
		return -1;
	}

	sql_is_connected=1;
	return 0;
}

static int fbsql_startquery(char *sqlquery)
{
	ISC_STATUS isc_status[20];
//#define __FN__ __FILE__ ":libnsp_data_sql_firebird_query()"
//	obj_t *thisobj=nsp_getobj(N, &N->l, "this");
//	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
//	obj_t *cobj;
//	FBSQL_CONN *conn;
//	char *sqlquery;

	XSQLVAR ISC_FAR *var;
	unsigned short  i;
	short           length, alignment, type, offset;
	char            stmt_info[] = { isc_info_sql_stmt_type };
	char            info_buffer[20];
	short           l;
	long            statement_type=0;

//	cobj=nsp_getobj(N, thisobj, "connection");
//	if ((cobj->val->type!=NT_CDATA)||(cobj->val->d.str==NULL)||(strcmp(cobj->val->d.str, "fbsql-conn")!=0))
//		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
//	conn=(FBSQL_CONN *)cobj->val->d.str;

//	if (cobj1->val->type!=NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
//	sqlquery=cobj1->val->d.str;

	/* Allocate a global statement */
	if (libfbsql.isc_dsql_allocate_statement(isc_status, &isc_db, &stmt)) {
		print_warning(isc_status, __FUNCTION__, "isc_dsql_allocate_statement");
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
			//printf("isc_start_transaction %s", (char *)isc_status);
			print_warning(isc_status, __FUNCTION__, "isc_start_transaction");
			return 1;
		}
		trans_block=0;
	}
	if (libfbsql.isc_dsql_prepare(isc_status, &trans, &stmt, 0, sqlquery, SQL_DIALECT_V6, sqlda)) {
		print_warning(isc_status, __FUNCTION__, "isc_dsql_prepare");
		//printf("isc_dsql_prepare %s\r\n", (char *)isc_status);
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
			print_warning(isc_status, __FUNCTION__, "isc_dsql_execute");
			return 2;
		}
		/* Commit DDL statements if that is what sql_info says */
		if (trans && trans_block==0 && (statement_type == isc_info_sql_stmt_ddl)) {
			//printf ("\tCommitting...\n");
			if (libfbsql.isc_commit_transaction(isc_status, &trans)) {
				print_warning(isc_status, __FUNCTION__, "isc_commit_transaction");
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
			print_warning(isc_status, __FUNCTION__, "isc_dsql_describe");
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
		print_warning(isc_status, __FUNCTION__, "isc_dsql_execute");
		return 2;
	}
	//nsp_setnum  (N, &N->r, "", 0);
	return 0;
//#undef __FN__
}

static int fbsql_endquery()
{
	ISC_STATUS isc_status[20];

	if (stmt) {
		if (libfbsql.isc_dsql_free_statement(isc_status, &stmt, DSQL_drop)) {
			print_warning(isc_status, __FUNCTION__, "isc_dsql_free_statement");
		}
	}
	if (fetch_stat != 100L) {
		/* print_warning(N, conn->status, __FUNCTION__, "conn->fetch_stat != 100L (are there unfetched rows?)"); */
	}
	fetch_stat=0;
	if (sqlda) {
		free(sqlda);
		sqlda=NULL;
	}
	if (trans && trans_block==0) {
		if (libfbsql.isc_commit_transaction(isc_status, &trans)) {
			print_warning(isc_status, __FUNCTION__, "isc_commit_transaction");
			return 0;
		}
		trans=0;
	}
	stmt=0;
	return 0;
}

static int fbsql_store_field(XSQLVAR ISC_FAR *var, char *cursor, int max)
{
	ISC_STATUS isc_status[20];

	char  buf[1024];
	char  blob_s[20];
	char  date_s[25];
	short dtype;
	struct tm times;

	dtype=var->sqltype&~1;
	/* Null handling.  If the column is nullable and null */
	if ((var->sqltype&1)&&(*var->sqlind<0)) {
		//nsp_setstr(N, tobj, var->aliasname, NULL, 0);
		//nsp_setstr(N, tobj, var->aliasname, NULL, 0);
		//printf("[%s]\r\n", "null");
		cursor[0]='\0';
		return 0;
	}
	buf[0]='\0';
	switch (dtype) {
	case SQL_TEXT:
		//nsp_setstr(N, tobj, var->aliasname, var->sqldata, var->sqllen);
		//printf("[%s]\r\n", var->sqldata);

		//printf("a[%s][%*.*s]\r\n", var->aliasname, var->sqllen, var->sqllen, var->sqldata);
		snprintf(cursor, max, "%*.*s", var->sqllen, var->sqllen, var->sqldata);
		return var->sqllen;
	case SQL_VARYING: {
		VARY *vary=(VARY *)var->sqldata;

		//nsp_setstr(N, tobj, var->aliasname, vary->vary_string, vary->vary_length);
		//printf("[%s]\r\n", vary->vary_string);
		//printf("b[%s][%*.*s]\r\n", var->aliasname, vary->vary_length, vary->vary_length, vary->vary_string);
		snprintf(cursor, max, "%*.*s", vary->vary_length, vary->vary_length, vary->vary_string);
		return vary->vary_length;
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
		//nsp_setstr(N, tobj, var->aliasname, buf, -1);
		//printf("c[%s][%*.*s]\r\n", var->aliasname, strlen(buf), strlen(buf), buf);
		snprintf(cursor, max, "%*.*s", (int)strlen(buf), (int)strlen(buf), buf);
		return strlen(buf);
	}
	case SQL_FLOAT:
		sprintf(buf, "%15g", *(float ISC_FAR *) (var->sqldata));
		//nsp_setstr(N, tobj, var->aliasname, buf, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(buf), (int)strlen(buf), buf);
		return strlen(buf);
	case SQL_DOUBLE:
		sprintf(buf, "%24f", *(double ISC_FAR *) (var->sqldata));
		//nsp_setstr(N, tobj, var->aliasname, buf, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(buf), (int)strlen(buf), buf);
		return strlen(buf);
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
		//nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(date_s), (int)strlen(date_s), date_s);
		return strlen(date_s);
	case SQL_TYPE_DATE:
		libfbsql.isc_decode_sql_date((ISC_DATE ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%04d-%02d-%02d",
			times.tm_year + 1900,
			times.tm_mon+1,
			times.tm_mday);
		//nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(date_s), (int)strlen(date_s), date_s);
		return strlen(date_s);
	case SQL_TYPE_TIME:
		libfbsql.isc_decode_sql_time((ISC_TIME ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%02d:%02d:%02d",
			times.tm_hour,
			times.tm_min,
			times.tm_sec
		);
		//nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(date_s), (int)strlen(date_s), date_s);
		return strlen(date_s);
	case SQL_BLOB: {
		int bloblen=0;
		ISC_QUAD        blob_id=*(ISC_QUAD ISC_FAR *)var->sqldata;
		isc_blob_handle blob_handle = NULL;
		short           blob_seg_len;
		char            blob_segment[256];
		long            blob_stat;
//		obj_t *cobj;

//		cobj=nsp_setstr(N, tobj, var->aliasname, "NULL", 0);
		/* Open the blob with the fetched blob_id. */
		if (libfbsql.isc_open_blob(isc_status, &isc_db, &trans, &blob_handle, &blob_id)) {
			print_warning(isc_status, __FUNCTION__, "isc_open_blob");
		}
		/* Get blob segments and their lengths and print each segment. */
		blob_stat = libfbsql.isc_get_segment(isc_status, &blob_handle, (unsigned short *)&blob_seg_len, sizeof(blob_segment), blob_segment);
		while (blob_stat == 0 || isc_status[1] == isc_segment) {
//			nsp_strcat(N, cobj, blob_segment, blob_seg_len);
//			printf("j[%s][%*.*s]\r\n", var->aliasname, blob_seg_len, blob_seg_len, blob_segment);
			snprintf(cursor, max, "%*.*s", blob_seg_len, blob_seg_len, blob_segment);
			cursor+=blob_seg_len;
			max-=blob_seg_len;
			bloblen+=blob_seg_len;
			blob_stat = libfbsql.isc_get_segment(isc_status, &blob_handle, (unsigned short *)&blob_seg_len, sizeof(blob_segment), blob_segment);
		}
		/* Close the blob.  Should be blob_stat to check */
		if (isc_status[1] == isc_segstr_eof) {
			if (libfbsql.isc_close_blob(isc_status, &blob_handle)) {
				print_warning(isc_status, __FUNCTION__, "isc_close_blob");
			}
		}
		return bloblen;
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
	//nsp_setstr(N, tobj, var->aliasname, buf, -1);
	printf("k[%s][%*.*s]\r\n", var->aliasname, (int)strlen(buf), (int)strlen(buf), buf);
	return 0;
}


















static int fbsqlUpdate(int verbose, char *sqlquery)
{
	fbsql_startquery(sqlquery);
	fbsql_endquery();
	return 0;
}

static int fbsqlQuery(SQLRES *sqr, char *sqlquery)
{
	ISC_STATUS isc_status[20];

	char *column;
	unsigned int field;
	unsigned int tuple;

	unsigned int rowsalloc;

	fbsql_startquery(sqlquery);

	sqr->NumFields=numfields;
	sqr->NumTuples=numtuples;

	/* build our cursor and track the number of tuples */
	rowsalloc=50;
	sqr->cursor=(char **)calloc(rowsalloc, sizeof(char *));
	/* now to populate the cursor */
	for (tuple=0;;tuple++) {
		int max=MAX_TUPLE_SIZE;
		if ((fetch_stat=libfbsql.isc_dsql_fetch(isc_status, &stmt, SQL_DIALECT_V6, sqlda))!=0) {
			break;
		}
		sqr->cursor[tuple]=calloc(MAX_TUPLE_SIZE, sizeof(char));
		if (sqr->cursor[tuple]==NULL) {
			printf("malloc() error creating SQL cursor tuple.\r\n");
			exit(-1);
		}
		if (tuple==0) {
			column=sqr->fields;
			for (field=0;field<sqr->NumFields;field++) {
				snprintf(column, MAX_FIELD_SIZE, "%s", sqlda->sqlvar[field].aliasname);
				column+=strlen(column)+1;
			}
		}
		column=sqr->cursor[tuple];
		for (field=0;field<sqr->NumFields;field++) {
			//int len=fbsql_store_field((XSQLVAR ISC_FAR *)&sqlda->sqlvar[field], column, sizeof(sqr->fields));
			int len=fbsql_store_field((XSQLVAR ISC_FAR *)&sqlda->sqlvar[field], column, max);
			if (len<0) {
				printf("fbsql_store_field len<0\r\n");
				exit(-1);
			}
			max-=strlen(column)-1;
			column+=strlen(column)+1;
		}
		numtuples++;
		if (tuple+2>rowsalloc) {
			rowsalloc+=50;
			sqr->cursor=(char **)realloc(sqr->cursor, rowsalloc*sizeof(char *));
		}
	}
	sqr->NumTuples=tuple;
	//printf("done query\r\n");
	fbsql_endquery();
	return 0;
}
#endif /* HAVE_FBSQL */





























#ifdef HAVE_MYSQL
static int mysqlDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s.12", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libmysqlclient.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
#ifdef WIN32
	snprintf(libname, sizeof(libname)-1, "libmysql.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
#endif
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
	if ((libmysql.store_result       = lib_sym(hinstLib, "mysql_store_result"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
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
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "libpq.%s", LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	goto fail;
found:
	if ((libpgsql.clear=(LIBPGSQL_CLEAR)lib_sym(hinstLib, "PQclear"))==NULL) goto fail;
	if ((libpgsql.errormessage=(LIBPGSQL_ERRORMESSAGE)lib_sym(hinstLib, "PQerrorMessage"))==NULL) goto fail;
	if ((libpgsql.exec=(LIBPGSQL_EXEC)lib_sym(hinstLib, "PQexec"))==NULL) goto fail;
	if ((libpgsql.finish=(LIBPGSQL_FINISH)lib_sym(hinstLib, "PQfinish"))==NULL) goto fail;
	if ((libpgsql.fname=(LIBPGSQL_FNAME)lib_sym(hinstLib, "PQfname"))==NULL) goto fail;
	if ((libpgsql.getvalue=(LIBPGSQL_GETVALUE)lib_sym(hinstLib, "PQgetvalue"))==NULL) goto fail;
	if ((libpgsql.nfields=(LIBPGSQL_NFIELDS)lib_sym(hinstLib, "PQnfields"))==NULL) goto fail;
	if ((libpgsql.ntuples=(LIBPGSQL_NTUPLES)lib_sym(hinstLib, "PQntuples"))==NULL) goto fail;
	if ((libpgsql.resultstatus=(LIBPGSQL_RESULTSTATUS)lib_sym(hinstLib, "PQresultStatus"))==NULL) goto fail;
	if ((libpgsql.setdblogin=(LIBPGSQL_SETDBLOGIN)lib_sym(hinstLib, "PQsetdbLogin"))==NULL) goto fail;
	if ((libpgsql.status=(LIBPGSQL_STATUS)lib_sym(hinstLib, "PQstatus"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
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
		if (verbose) printf("PGSQL Update - %s\r\n", libpgsql.errormessage(pgconn));
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
		printf("PGSQL Query - %s\r\n", libpgsql.errormessage(pgconn));
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
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite2.%s", config.dir_lib, LIBEXT);
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
	if ((libsqlite2.open=(SQLITE2_OPEN)lib_sym(hinstLib, "sqlite_open"))==NULL) goto fail;
	if ((libsqlite2.exec=(SQLITE2_EXEC)lib_sym(hinstLib, "sqlite_exec"))==NULL) goto fail;
	if ((libsqlite2.close=(SQLITE2_CLOSE)lib_sym(hinstLib, "sqlite_close"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
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
/*	libsqlite2.exec(db2, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg); */
/*	libsqlite2.exec(db2, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg); */
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
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite3.%s", config.dir_lib, LIBEXT);
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
	if ((libsqlite3.open=(SQLITE3_OPEN)lib_sym(hinstLib, "sqlite3_open"))==NULL) goto fail;
	if ((libsqlite3.exec=(SQLITE3_EXEC)lib_sym(hinstLib, "sqlite3_exec"))==NULL) goto fail;
	if ((libsqlite3.close=(SQLITE3_CLOSE)lib_sym(hinstLib, "sqlite3_close"))==NULL) goto fail;
	return 0;
fail:
	printf("ERROR: Failed to load %s\r\n", libname);
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
/*	libsqlite3.exec(db3, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg); */
/*	libsqlite3.exec(db3, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg); */
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
#ifdef HAVE_FBSQL
	memset((char *)&libfbsql, 0, sizeof(libfbsql));
#endif
	memset((char *)&libmysql, 0, sizeof(libmysql));
	memset((char *)&libpgsql, 0, sizeof(libpgsql));
	memset((char *)&libsqlite2, 0, sizeof(libsqlite2));
	memset((char *)&libsqlite3, 0, sizeof(libsqlite3));
	if (hinstLib!=NULL) lib_close(hinstLib);
	hinstLib=NULL;
	return 0;
}

void sql_disconnect()
{
/*	pthread_mutex_lock(&Lock.SQL); */
	if (strcmp(config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		fbsqlDisconnect();
#endif
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
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
/*	pthread_mutex_unlock(&Lock.SQL); */
	return;
}
/*
void sql_unsafedisconnect()
{
	printf("SQL Disconnection\r\n");
	if (strcmp(config.sql_type, "FBSQL")==0) {
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
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

/*	pthread_mutex_lock(&Lock.SQL); */
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
/*	printf("SQL query [%d] freed\r\n", sqr); */
/*	pthread_mutex_unlock(&Lock.SQL); */
	return;
}

int sql_update(int verbose, char *sqlquery)
{
	int rc=-1;

/*	pthread_mutex_lock(&Lock.SQL); */
	if (strcmp(config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		if (fbsqlConnect()<0) {
			return -1;
		}
		rc=fbsqlUpdate(verbose, sqlquery);
#endif
	}
	else if (strcmp(config.sql_type, "MYSQL") == 0) {
		if (mysqlConnect()<0) {
			return -1;
		}
		rc=mysqlUpdate(verbose, sqlquery);
	} else if (strcmp(config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		rc=odbcUpdate(verbose, sqlquery);
#endif
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		rc=pgsqlUpdate(verbose, sqlquery);
	} else if (strcmp(config.sql_type, "SQLITE2")==0) {
		if (sqlite2Connect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		rc=sqlite2Update(verbose, sqlquery);
	} else if ((strcmp(config.sql_type, "SQLITE3")==0)||(strcmp(config.sql_type, "SQLITE")==0)) {
		if (sqlite3Connect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		rc=sqlite3Update(verbose, sqlquery);
	}
/*	pthread_mutex_unlock(&Lock.SQL); */
	return rc;
}

int sql_updatef(int verbose, char *format, ...)
{
	char sqlquery[16384];
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

/*	pthread_mutex_lock(&Lock.SQL); */
	memset((char *)sqr, 0, sizeof(SQLRES));
	if (strcmp(config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		if (fbsqlConnect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		if ((rc=fbsqlQuery(sqr, query))<0) {
			fbsqlDisconnect();
			fbsqlConnect();
		}
#endif
	} else if (strcmp(config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		if ((rc=mysqlQuery(sqr, query))<0) {
			mysqlDisconnect();
			mysqlConnect();
		}
	} else if (strcmp(config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		if ((rc=odbcQuery(sqr, query))<0) {
			odbcDisconnect();
			odbcConnect();
		}
#endif
	} else if (strcmp(config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		if ((rc=pgsqlQuery(sqr, query))<0) {
			pgsqlDisconnect();
			pgsqlConnect();
		}
	} else if (strcmp(config.sql_type, "SQLITE2")==0) {
		if (sqlite2Connect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		if ((rc=sqlite2Query(sqr, query))<0) {
			sqlite2Disconnect();
			sqlite2Connect();
		}
	} else if ((strcmp(config.sql_type, "SQLITE3")==0)||(strcmp(config.sql_type, "SQLITE")==0)) {
		if (sqlite3Connect()<0) {
/*			pthread_mutex_unlock(&Lock.SQL); */
			return -1;
		}
		if ((rc=sqlite3Query(sqr, query))<0) {
			sqlite3Disconnect();
			sqlite3Connect();
		}
	}
/*	pthread_mutex_unlock(&Lock.SQL); */
	return rc;
}

int sql_queryf(SQLRES *sqr, char *format, ...)
{
	char sqlquery[16384];
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

int sqlfprintf(FILE *fp, const char *format, ...)
{
	char buffer[16384];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	while (buffer[offset]) {
		if (buffer[offset]==13) {
/*			fprintf(fp, "\\r"); */
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
