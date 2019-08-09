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
#define LIBEXT "dll"
#else
#define LIBEXT "so"
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

static int fbsqlDLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libfbclient.%s", proc.config.dir_lib, LIBEXT);
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
		log_error("sql", __FILE__, __LINE__, 1, "%s: %s\r\n", line++==0?"Firebird exception":"                  ", errbuf);
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
	snprintf(port, sizeof(port)-1, "%d", proc.config.sql_port);

	if (strlen(proc.config.sql_username)&&strlen(proc.config.sql_password)) {
		len=snprintf(buf, sizeof(buf), "%c%c%c%s%c%c%s", isc_dpb_version1, isc_dpb_user_name, (int)strlen(proc.config.sql_username), proc.config.sql_username, isc_dpb_password, (int)strlen(proc.config.sql_password), proc.config.sql_password);
		snprintf(buf2, sizeof(buf2), "%s:%s", proc.config.sql_hostname, proc.config.sql_dbname);
		rc=libfbsql.isc_attach_database(isc_status, 0, buf2, &isc_db, len, buf);
	} else {
		snprintf(buf2, sizeof(buf2), "%s:%s", proc.config.sql_hostname, proc.config.sql_dbname);
		rc=libfbsql.isc_attach_database(isc_status, 0, buf2, &isc_db, 0, NULL);
	}
	if (rc) {
		log_error("sql", __FILE__, __LINE__, 1, "connection error %s - %s\r\n", proc.config.sql_dbname, (char *)isc_status);
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

//		log_error("sql", __FILE__, __LINE__, 6, "a[%s][%*.*s]\r\n", var->aliasname, var->sqllen, var->sqllen, var->sqldata);
		snprintf(cursor, max, "%*.*s", var->sqllen, var->sqllen, var->sqldata);
		return var->sqllen;
	case SQL_VARYING: {
		VARY *vary=(VARY *)var->sqldata;

		//nsp_setstr(N, tobj, var->aliasname, vary->vary_string, vary->vary_length);
		//printf("[%s]\r\n", vary->vary_string);
//		log_error("sql", __FILE__, __LINE__, 6, "b[%s][%*.*s]\r\n", var->aliasname, vary->vary_length, vary->vary_length, vary->vary_string);
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
//		log_error("sql", __FILE__, __LINE__, 6, "[%s][%*.*s]\r\n", var->aliasname, strlen(buf), strlen(buf), buf);
		snprintf(cursor, max, "%*.*s", (int)strlen(buf), (int)strlen(buf), buf);
		return strlen(buf);
	}
	case SQL_FLOAT:
		sprintf(buf, "%15g", *(float ISC_FAR *) (var->sqldata));
		//nsp_setstr(N, tobj, var->aliasname, buf, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(buf), (int)strlen(buf), buf);
//		log_error("sql", __FILE__, __LINE__, 6, "[%s][%*.*s]\r\n", var->aliasname, (int)strlen(buf), (int)strlen(buf), buf);
		return strlen(buf);
	case SQL_DOUBLE:
		sprintf(buf, "%24f", *(double ISC_FAR *) (var->sqldata));
		//nsp_setstr(N, tobj, var->aliasname, buf, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(buf), (int)strlen(buf), buf);
//		log_error("sql", __FILE__, __LINE__, 6, "[%s][%*.*s]\r\n", var->aliasname, (int)strlen(buf), (int)strlen(buf), buf);
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
//		log_error("sql", __FILE__, __LINE__, 6, "[%s][%*.*s]\r\n", var->aliasname, (int)strlen(date_s), (int)strlen(date_s), date_s);
		return strlen(date_s);
	case SQL_TYPE_DATE:
		libfbsql.isc_decode_sql_date((ISC_DATE ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%04d-%02d-%02d",
			times.tm_year + 1900,
			times.tm_mon+1,
			times.tm_mday);
		//nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		snprintf(cursor, max, "%*.*s", (int)strlen(date_s), (int)strlen(date_s), date_s);
//		log_error("sql", __FILE__, __LINE__, 6, "[%s][%*.*s]\r\n", var->aliasname, (int)strlen(date_s), (int)strlen(date_s), date_s);
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
//		log_error("sql", __FILE__, __LINE__, 6, "[%s][%*.*s]\r\n", var->aliasname, (int)strlen(date_s), (int)strlen(date_s), date_s);
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
		log_error("sql", __FILE__, __LINE__, 6, "blob");
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
//	log_error("sql", __FILE__, __LINE__, 6, "k[%s][%*.*s]\r\n", var->aliasname, (int)strlen(buf), (int)strlen(buf), buf);
	return 0;
}


















static int fbsqlUpdate(char *sqlquery)
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
	//unsigned int tuple;

	unsigned int rowsalloc;
	unsigned int slen;

	fbsql_startquery(sqlquery);

	sqr->NumFields=numfields;

	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		char *ptemp=column;
		snprintf(column, MAX_FIELD_SIZE, "%s", sqlda->sqlvar[field].aliasname);
		//while (*ptemp) *ptemp++=tolower(*ptemp);
		column+=strlen(column)+1;
	}
	/* build our cursor and track the number of tuples */
	rowsalloc=50;
	if ((sqr->row=calloc(rowsalloc, sizeof(SQLROW)))==NULL) {
		log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL result.");
		exit(-1);
	}
	/* now to populate the cursor */
	for (sqr->NumTuples=0;;sqr->NumTuples++) {
		if ((fetch_stat=libfbsql.isc_dsql_fetch(isc_status, &stmt, SQL_DIALECT_V6, sqlda))!=0) break;
		for (field=0;field<sqr->NumFields;field++) {
			char  buf[8192];

			//memset(buf, 0, sizeof(buf));
			fbsql_store_field((XSQLVAR ISC_FAR *)&sqlda->sqlvar[field], buf, sizeof(buf));
			slen=strlen(buf);
			//log_error("sql", __FILE__, __LINE__, 6, "[%s][%s]\r\n", sqlda->sqlvar[field].aliasname, buf);
			if ((sqr->row[sqr->NumTuples].field[field]=malloc(slen+1))==NULL) exit(-1);
			memcpy(sqr->row[sqr->NumTuples].field[field], buf, slen);
			sqr->row[sqr->NumTuples].field[field][slen]='\0';
		}
		if (sqr->NumTuples+2>rowsalloc) {
			rowsalloc+=50;
			if ((sqr->row=realloc(sqr->row, rowsalloc*sizeof(SQLROW)))==NULL) {
				log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				exit(-1);
			}
		}
	}
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
	snprintf(libname, sizeof(libname)-1, "%s/libmysql.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/libmysqlclient.%s", proc.config.dir_lib, LIBEXT);
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
	if ((libmysql.use_result         = lib_sym(hinstLib, "mysql_use_result"))==NULL) goto fail;
	return 0;
fail:
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
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
	if (!(mysock=libmysql.real_connect(&mysql, proc.config.sql_hostname, proc.config.sql_username, proc.config.sql_password, proc.config.sql_dbname, proc.config.sql_port, NULL, 0))) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int mysqlUpdate(char *sqlquery)
{
	if (libmysql.query(mysock, sqlquery)) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		log_error("sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
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
//	unsigned int tuple;
	unsigned int rowsalloc;
	unsigned int slen;

	if (libmysql.ping(mysock)!=0) {
		if (mysqlConnect()<0) return -1;
	}
	if (libmysql.query(mysock, sqlquery)) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
		log_error("sql", __FILE__, __LINE__, 2, "MYSQL: [%s]", sqlquery);
		return -1;
	}
	if (!(myres=libmysql.use_result(mysock))) {
		log_error("sql", __FILE__, __LINE__, 1, "MYSQL error: %s", libmysql.error(&mysql));
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
	sqr->NumTuples=0;
	rowsalloc=50;
	if ((sqr->row=calloc(rowsalloc, sizeof(SQLROW)))==NULL) {
		log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL result.");
		exit(-1);
	}
	/* now to populate the cursor */
	for (sqr->NumTuples=0;;sqr->NumTuples++) {
		if ((MYrow=libmysql.fetch_row(myres))==NULL) break;
		for (field=0;field<sqr->NumFields;field++) {
			column=MYrow[field];
			column=column?column:"NULL";
			slen=strlen(column);
			if ((sqr->row[sqr->NumTuples].field[field]=malloc(slen+1))==NULL) exit(-1);
			memcpy(sqr->row[sqr->NumTuples].field[field], column, slen);
			sqr->row[sqr->NumTuples].field[field][slen]='\0';
		}
		if (sqr->NumTuples+2>rowsalloc) {
			rowsalloc+=50;
			if ((sqr->row=realloc(sqr->row, rowsalloc*sizeof(SQLROW)))==NULL) {
				log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				exit(-1);
			}
		}
	}
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
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - Unable to allocate an environment handle.");
		return -1;
	}
	rc=SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLSetEnvAttr %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDBC);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLAllocHandle %s", buf);
		odbcDisconnect();
		return -1;
	}
	rc=SQLDriverConnect(hDBC, NULL, proc.config.sql_odbc_dsn, (short int)strlen(proc.config.sql_odbc_dsn), szConnStr, sizeof(szConnStr), &cbConnStr, SQL_DRIVER_NOPROMPT);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Connect - SQLDriverConnect %s", buf);
		odbcDisconnect();
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int odbcUpdate(char *sqlquery)
{
	char sqlstate[15];
	char buf[250];
	RETCODE rc;

	rc=SQLAllocHandle(SQL_HANDLE_STMT, hDBC, &hStmt);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Update - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Update - SQLExecDirect %s", buf);
		log_error("sql", __FILE__, __LINE__, 2, "ODBC UPDATE: [%s]", sqlquery);
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
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLAllocHandle %s", buf);
		return -1;
	}
	rc=SQLExecDirect(hStmt, sqlquery, SQL_NTS); 
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLExecDirect %s", buf);
		log_error("sql", __FILE__, __LINE__, 2, "ODBC SELECT: [%s]", sqlquery);
		return -1;
	}
	rc=SQLNumResultCols(hStmt, &pccol);
	if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
		SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
		log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLNumResultCols %s", buf);
		return -1;
	}
	sqr->NumFields=pccol;
	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		rc=SQLDescribeCol(hStmt, (SQLSMALLINT)(field+1), column, MAX_FIELD_SIZE, NULL, NULL, NULL, NULL, NULL);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) {
			SQLError(hEnv, hDBC, hStmt, sqlstate, NULL, buf, sizeof(buf), NULL);
			log_error("sql", __FILE__, __LINE__, 1, "ODBC Query - SQLDescribeCol %s", buf);
			return -1;
		}
		column+=strlen(column)+1;
	}
	/* build our cursor and track the number of tuples */
	rowsalloc=50;
	if ((sqr->row=calloc(rowsalloc, sizeof(SQLROW)))==NULL) {
		log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL result.");
		exit(-1);
	}
	/* now to populate the cursor */
	for (sqr->NumTuples=0;;sqr->NumTuples++) {
		rc=SQLFetch(hStmt);
		if ((rc!=SQL_SUCCESS)&&(rc!=SQL_SUCCESS_WITH_INFO)) break;
		/* odbc is a fucking piece of shit */
		for (field=0;field<sqr->NumFields;field++) {
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, 0, &slen);
			if ((sqr->row[sqr->NumTuples].field[field]=malloc(slen+1))==NULL) exit(-1);
			rc=SQLGetData(hStmt, (SQLUSMALLINT)(field+1), SQL_C_CHAR, column, slen, &slen);
			sqr->row[sqr->NumTuples].field[field][slen]='\0';
		}
		if (sqr->NumTuples+2>rowsalloc) {
			rowsalloc+=50;
			if ((sqr->row=realloc(sqr->row, rowsalloc*sizeof(SQLROW)))==NULL) {
				log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				exit(-1);
			}
		}
	}
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
	snprintf(libname, sizeof(libname)-1, "%s/libpq.%s", proc.config.dir_lib, LIBEXT);
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
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
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
	snprintf(port, sizeof(port)-1, "%d", proc.config.sql_port);
	pgconn=libpgsql.setdblogin(proc.config.sql_hostname, port, NULL, NULL, proc.config.sql_dbname, proc.config.sql_username, proc.config.sql_password);
	if (libpgsql.status(pgconn)==CONNECTION_BAD) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	sql_is_connected=1;
	return 0;
}

static int pgsqlUpdate(char *sqlquery)
{
	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	pgres=libpgsql.exec(pgconn, sqlquery);
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL error: %s", libpgsql.errormessage(pgconn));
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
	unsigned int slen;

	pgres=libpgsql.exec(pgconn, "BEGIN");
	if (libpgsql.resultstatus(pgres)!=PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Connect - %s", libpgsql.errormessage(pgconn));
		return -1;
	}
	libpgsql.clear(pgres);
	memset(query, 0, sizeof(query));
	snprintf(query, sizeof(query)-1, "DECLARE myportal CURSOR FOR ");
	strncat(query, sqlquery, sizeof(query));
	pgres = libpgsql.exec(pgconn, query);
	if (libpgsql.resultstatus(pgres) != PGRES_COMMAND_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	libpgsql.clear(pgres);
	pgres = libpgsql.exec(pgconn, "FETCH ALL IN myportal");
	if (libpgsql.resultstatus(pgres) != PGRES_TUPLES_OK) {
		log_error("sql", __FILE__, __LINE__, 1, "PGSQL Query - %s", libpgsql.errormessage(pgconn));
		pgsqlDisconnect();
		return -1;
	}
	sqr->NumFields=libpgsql.nfields(pgres);
	sqr->NumTuples=libpgsql.ntuples(pgres);
	/* retreive the field names */
	column=sqr->fields;
	for (field=0;field<sqr->NumFields;field++) {
		snprintf(column, MAX_FIELD_SIZE, "%s", libpgsql.fname(pgres, field));
		column+=strlen(column)+1;
	}
	/* k..  now we know how many tuples and fields, we can build our cursor */
	if ((sqr->row=calloc(sqr->NumTuples, sizeof(SQLROW)))==NULL) {
		log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL result.");
		exit(-1);
	}
	/* now to populate the cursor */
	for (tuple=0;tuple<sqr->NumTuples;tuple++) {
		for (field=0;field<sqr->NumFields;field++) {
			column=libpgsql.getvalue(pgres, tuple, field);
			column=column?column:"NULL";
			slen=strlen(column);
			if ((sqr->row[sqr->NumTuples].field[field]=malloc(slen+1))==NULL) exit(-1);
			memcpy(sqr->row[sqr->NumTuples].field[field], column, slen);
			sqr->row[sqr->NumTuples].field[field][slen]='\0';
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
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite2.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite2.%s", proc.config.dir_lib, LIBEXT);
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
	log_error("sql", __FILE__, __LINE__, 0, "ERROR: Failed to load %s", libname);
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
	snprintf(dbname, sizeof(dbname)-1, "%s/%s.db", proc.config.dir_var_db, SERVER_BASENAME);
	fixslashes(dbname);
	db2=libsqlite2.open(dbname, 0, &zErrMsg);
	if (db2==0) {
		log_error("sql", __FILE__, __LINE__, 1, "SQLite Connect - %s", zErrMsg);
		return -1;
	}
	sql_is_connected=1;
	libsqlite2.exec(db2, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
	libsqlite2.exec(db2, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
	return 0;
}

static int sqlite2Update(char *sqlquery)
{
	char *zErrMsg=0;
	int rc;
	short int retries=0;

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
		log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %s", zErrMsg);
		log_error("sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite2Callback(void *vpsqr, int argc, char **argv, char **azColName)
{
	char *column;
	static unsigned int rowsalloc;
	unsigned int field;
	unsigned int slen;
	SQLRES *sqr=vpsqr;

	if (sqr->row==NULL) {
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
		if ((sqr->row=calloc(rowsalloc, sizeof(SQLROW)))==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL result.");
			exit(-1);
		}
	}
	/* now to populate the cursor */
	if (argv!=NULL) {
		for (field=0;field<sqr->NumFields;field++) {
			column=argv[field]?argv[field]:"NULL";
			slen=strlen(column);
			if ((sqr->row[sqr->NumTuples].field[field]=malloc(slen+1))==NULL) exit(-1);
			memcpy(sqr->row[sqr->NumTuples].field[field], column, slen);
			sqr->row[sqr->NumTuples].field[field][slen]='\0';
		}
		if (sqr->NumTuples+2>rowsalloc) {
			rowsalloc+=50;
			if ((sqr->row=realloc(sqr->row, rowsalloc*sizeof(SQLROW)))==NULL) {
				log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				exit(-1);
			}
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
	log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
	log_error("sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE2 */

#ifdef HAVE_SQLITE3
static int SQLite3DLLInit()
{
	char libname[255];

	if (hinstLib!=NULL) return 0;
	memset(libname, 0, sizeof(libname));
	snprintf(libname, sizeof(libname)-1, "%s/libsqlite3.%s", proc.config.dir_lib, LIBEXT);
	fixslashes(libname);
	if ((hinstLib=lib_open(libname))!=NULL) goto found;
	snprintf(libname, sizeof(libname)-1, "%s/sqlite3.%s", proc.config.dir_lib, LIBEXT);
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

static int sqlite3Connect()
{
	char dbname[255];
	char *zErrMsg=0;

	if (SQLite3DLLInit()!=0) return -1;
	if (sql_is_connected) return 0;
	snprintf(dbname, sizeof(dbname)-1, "%s/%s.db", proc.config.dir_var_db, SERVER_BASENAME);
	fixslashes(dbname);
	if (libsqlite3.open(dbname, &db3)!=SQLITE_OK) {
		printf("\nSQLite Connect error");
		return -1;
	}
	sql_is_connected=1;
	libsqlite3.exec(db3, "PRAGMA default_synchronous = TRUE;", NULL, 0, &zErrMsg);
	libsqlite3.exec(db3, "PRAGMA empty_result_callbacks = ON;", NULL, 0, &zErrMsg);
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
		return 0;
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
		log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %s", zErrMsg);
		log_error("sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
		return -1;
	}
	return 0;
}

static int sqlite3Callback(void *vpsqr, int argc, char **argv, char **azColName)
{
	char *column;
	static unsigned int rowsalloc;
	unsigned int field;
	unsigned int slen;
	SQLRES *sqr=vpsqr;

	if (sqr->row==NULL) {
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
		if ((sqr->row=calloc(rowsalloc, sizeof(SQLROW)))==NULL) {
			log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL result.");
			exit(-1);
		}
	}
	/* now to populate the cursor */
	if (argv!=NULL) {
		for (field=0;field<sqr->NumFields;field++) {
			column=argv[field]?argv[field]:"NULL";
			slen=strlen(column);
			if ((sqr->row[sqr->NumTuples].field[field]=malloc(slen+1))==NULL) exit(-1);
			memcpy(sqr->row[sqr->NumTuples].field[field], column, slen);
			sqr->row[sqr->NumTuples].field[field][slen]='\0';
		}
		if (sqr->NumTuples+2>rowsalloc) {
			rowsalloc+=50;
			if ((sqr->row=realloc(sqr->row, rowsalloc*sizeof(SQLROW)))==NULL) {
				log_error("sql", __FILE__, __LINE__, 1, "malloc() error while creating SQL cursor.");
				exit(-1);
			}
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
	log_error("sql", __FILE__, __LINE__, 1, "SQLite error: %d %s", rc, zErrMsg);
	log_error("sql", __FILE__, __LINE__, 2, "SQLite: [%s]", sqlquery);
	return -1;
}
#endif /* HAVE_SQLITE3 */

int sql_dll_unload()
{
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
	pthread_mutex_lock(&Lock.SQL);
	log_error("sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if (strcmp(proc.config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		fbsqlDisconnect();
#endif
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "SQLITE2")==0) {
		sqlite2Disconnect();
	} else if ((strcmp(proc.config.sql_type, "SQLITE3")==0)||(strcmp(proc.config.sql_type, "SQLITE")==0)) {
		sqlite3Disconnect();
	}
	sql_dll_unload();
	pthread_mutex_unlock(&Lock.SQL);
	return;
}

void sql_unsafedisconnect()
{
	log_error("sql", __FILE__, __LINE__, 4, "SQL Disconnection");
	if (strcmp(proc.config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		fbsqlDisconnect();
#endif
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		mysqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		odbcDisconnect();
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		pgsqlDisconnect();
	} else if (strcmp(proc.config.sql_type, "SQLITE2")==0) {
		sqlite2Disconnect();
	} else if ((strcmp(proc.config.sql_type, "SQLITE3")==0)||(strcmp(proc.config.sql_type, "SQLITE")==0)) {
		sqlite3Disconnect();
	}
	sql_dll_unload();
	return;
}

void sql_freeresult(SQLRES *sqr)
{
	unsigned int tuple;
	unsigned int field;

	memset(sqr->fields, 0, sizeof(sqr->fields));
	if (sqr->row!=NULL) {
		for (tuple=0;tuple<sqr->NumTuples;tuple++) {
			for (field=0;field<sqr->NumFields;field++) {
				if (sqr->row[tuple].field[field]!=NULL) {
					free(sqr->row[tuple].field[field]);
					sqr->row[tuple].field[field]=NULL;
				}
			}
		}
		free(sqr->row);
		sqr->row=NULL;
		proc.stats.sql_handlecount--;
	} else {
		log_error("sql", __FILE__, __LINE__, 1, "freeing an unused sql result?");
	}
	sqr->NumFields=0;
	sqr->NumTuples=0;
	log_error("sql", __FILE__, __LINE__, 4, "SQL query [0x%08X] freed", sqr);
	return;
}

int sql_update(char *sqlquery)
{
	int rc=-1;

	log_error("sql", __FILE__, __LINE__, 2, "SQL update: %s", sqlquery);
	pthread_mutex_lock(&Lock.SQL);
	proc.stats.sql_updates++;
	if (strcmp(proc.config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		if (fbsqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=fbsqlUpdate(sqlquery);
#endif
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=mysqlUpdate(sqlquery);
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=odbcUpdate(sqlquery);
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=pgsqlUpdate(sqlquery);
	} else if (strcmp(proc.config.sql_type, "SQLITE2")==0) {
		if (sqlite2Connect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=sqlite2Update(sqlquery);
	} else if ((strcmp(proc.config.sql_type, "SQLITE3")==0)||(strcmp(proc.config.sql_type, "SQLITE")==0)) {
		if (sqlite3Connect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		rc=sqlite3Update(sqlquery);
	}
	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_updatef(char *format, ...)
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
	rc=sql_update(sqlquery);
	free(sqlquery);
	return rc;
}

int sql_query(SQLRES *sqr, char *query)
{
	int rc=-1;

	memset((char *)sqr, 0, sizeof(SQLRES));
	log_error("sql", __FILE__, __LINE__, 3, "SQL query: [0x%08X] %s", sqr, query);
	pthread_mutex_lock(&Lock.SQL);
	proc.stats.sql_queries++;
	if (strcmp(proc.config.sql_type, "FBSQL")==0) {
#ifdef HAVE_FBSQL
		if (fbsqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=fbsqlQuery(sqr, query))<0) {
			fbsqlDisconnect();
			fbsqlConnect();
		}
#endif
	} else if (strcmp(proc.config.sql_type, "MYSQL")==0) {
		if (mysqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=mysqlQuery(sqr, query))<0) {
			mysqlDisconnect();
			mysqlConnect();
		}
	} else if (strcmp(proc.config.sql_type, "ODBC")==0) {
#ifdef HAVE_ODBC
		if (odbcConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=odbcQuery(sqr, query))<0) {
			odbcDisconnect();
			odbcConnect();
		}
#endif
	} else if (strcmp(proc.config.sql_type, "PGSQL")==0) {
		if (pgsqlConnect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=pgsqlQuery(sqr, query))<0) {
			pgsqlDisconnect();
			pgsqlConnect();
		}
	} else if (strcmp(proc.config.sql_type, "SQLITE2")==0) {
		if (sqlite2Connect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=sqlite2Query(sqr, query))<0) {
			sqlite2Disconnect();
			sqlite2Connect();
		}
	} else if ((strcmp(proc.config.sql_type, "SQLITE3")==0)||(strcmp(proc.config.sql_type, "SQLITE")==0)) {
		if (sqlite3Connect()<0) {
			pthread_mutex_unlock(&Lock.SQL);
			return -1;
		}
		if ((rc=sqlite3Query(sqr, query))<0) {
			sqlite3Disconnect();
			sqlite3Connect();
		}
	}
	if (rc>-1) proc.stats.sql_handlecount++;
	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_queryf(SQLRES *sqr, char *format, ...)
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
	rc=sql_query(sqr, sqlquery);
	free(sqlquery);
	return rc;
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
	if ((tuple<0)||(tuple+1>(int)sqr->NumTuples)) return NULL;
	if ((field<0)||(field+1>(int)sqr->NumFields)) return NULL;
	//log_error("sql", __FILE__, __LINE__, 1, "sql_getvalue[%d][%s].", field, sqr->row[tuple].field[field]);
	return sqr->row[tuple].field[field];
}

char *sql_getvaluebyname(SQLRES *sqr, int tuple, char *fieldname)
{
	unsigned int field;

	if ((tuple<0)||(tuple+1>(int)sqr->NumTuples)) return NULL;
	for (field=0;field<sqr->NumFields;field++) {
		if (strcasecmp(fieldname, sql_getname(sqr, field))==0) {
			//log_error("sql", __FILE__, __LINE__, 1, "sql_getvaluebyname[%s][%s].", fieldname, sqr->row[tuple].field[field]);
			return sqr->row[tuple].field[field];
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
