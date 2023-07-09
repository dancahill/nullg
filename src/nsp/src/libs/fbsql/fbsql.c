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
#include "fbsql.h"

#ifdef HAVE_FBSQL

#include "fbsql.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <firebird/ibase.h>

#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#define    MAXLEN    8192

typedef struct FBSQL_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's fbsql-specific */
	isc_db_handle db;
	ISC_STATUS status[20];

	isc_tr_handle trans;
	isc_stmt_handle stmt;

	XSQLDA ISC_FAR *sqlda;
	long fetch_stat;
	short trans_block;

	int numfields;
	int numtuples;

	long buffer[MAXLEN];

} FBSQL_CONN;

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

#if (defined(_MSC_VER) && defined(_WIN32)) || (defined(__BORLANDC__) && defined(__WIN32__))
#define FB_ALIGN(n,b)          ((n + b - 1) & ~(b - 1))
#endif

//#ifndef ALIGN
#ifndef FB_ALIGN
#define FB_ALIGN(n,b)          ((n+1) & ~1)
#endif

#endif /* FB_ALIGN */

typedef struct vary {
	short          vary_length;
	char           vary_string[1];
} VARY;

/* Define a format string for printf.  Printing of 64-bit integers
   is not standard between platforms */
#ifndef ISC_INT64_FORMAT
#if (defined(_MSC_VER) && defined(_WIN32)) || (defined(__BORLANDC__) && defined(__WIN32__))
#define	ISC_INT64_FORMAT	"I64"
#else
#define	ISC_INT64_FORMAT	"ll"
#endif
#endif

static void print_warning(nsp_state *N, const ISC_STATUS *pvector, const char *__FN__, const char *errstr)
{
	char errbuf[1024];
	long int rc;
	obj_t *cobj;

	cobj = nsp_setstr(N, nsp_settable(N, nsp_settable(N, nsp_getobj(N, &N->g, "lib"), "data"), "firebird"), "last_error", (char *)errstr, -1);
	while ((rc = fb_interpret(errbuf, sizeof(errbuf), &pvector)) > 0) {
		nsp_strcat(N, cobj, "\r\n\t", 3);
		nsp_strcat(N, cobj, errbuf, rc);
	}
	if (cobj->val->size > 0) {
		n_warn(N, __FN__, "%s", cobj->val->d.str);
		N->warnings--;
	}
	return;
}

static int rows_affected(nsp_state *N, FBSQL_CONN *conn)
{

	//if (!isc_dsql_sql_info(conn->status, &conn->stmt, sizeof(type_item), type_item, sizeof(res_buffer), res_buffer)) {
	//	length = (short)isc_vax_integer((char ISC_FAR *) res_buffer + 1, 2);
	//	statement_type = isc_vax_integer((char ISC_FAR *) res_buffer + 3, length);
	//	n_warn(N, __FN__, "isc_dsql_sql_info ... %d %d", length, statement_type);
	//	length = isc_portable_integer(res_buffer[1], 2);
	//	statement_type = isc_portable_integer(res_buffer[3], length);
	//	n_warn(N, __FN__, "isc_dsql_sql_info ... %d %d", length, statement_type);

	//	n_warn(N, __FN__, "'%s'", res_buffer);
	//	//				if (res_buffer[0] == isc_info_sql_records) {
	//	unsigned i = 3, result_size = isc_vax_integer(&res_buffer[1], 2);

	//	while (res_buffer[i] != isc_info_end && i < result_size) {
	//		short len = (short)isc_vax_integer(&res_buffer[i + 1], 2);
	//		if (res_buffer[i] == isc_info_sql_records) {//isc_info_sql_records
	//			n_warn(N, __FN__, "reported row count : %d", isc_vax_integer(&res_buffer[i + 3], len));
	//		}
	//		i += len + 3;
	//	}
	//}


	//http://www.ibase.ru/mail/RowsAffected.txt
	/*

	Result buffer for DELETE statement contains:
	23, 29,0, (isc_info_sql_records, length=29)
	15, 4,0, 0,0,0,0, (isc_info_req_update_count, 0 rows updated)
	16, 4,0, 4,0,0,0, (isc_info_req_delete_count, 4 rows deleted)
	13, 4,0, 4,0,0,0, (isc_info_req_select_count, 4 rows selected)
	14, 4,0, 0,0,0,0, (isc_info_req_insert_count)
	1, (isc_info_end)

	Result buffer for UPDATE statement contains:
	23, 29,0,
	15, 4,0, 4,0,0,0,
	16, 4,0, 0,0,0,0,
	13, 4,0, 4,0,0,0,
	14, 4,0, 0,0,0,0,
	1,

	Result buffer for INSERT statement contains:
	23, 29,0,
	15, 4,0, 0,0,0,0,
	16, 4,0, 0,0,0,0,
	13, 4,0, 0,0,0,0,
	14, 4,0, 1,0,0,0,
	1,
	*/


	//char string[1024];
	char	cnt_info[2], string[1024], count_type, *ptr;
	short	l;
	long	count;
	//char type_item[] = { isc_info_sql_records };
	//char res_buffer[8];
	////short           l;
	////long            statement_type = 0;
	//int statement_type;
	//int length;
	int updated = 0;


	cnt_info[0] = isc_info_sql_records;
	cnt_info[1] = isc_info_end;
	/* added per suggestion by Deej */
	(void)isc_dsql_sql_info(conn->status, &conn->stmt, sizeof(cnt_info), cnt_info, sizeof(string), string);
	int x = 0;
	for (ptr = string + 3; *ptr != isc_info_end; ) {
		x++;
		count_type = *ptr++;
		l = (short)isc_vax_integer(ptr, 2);
		ptr += 2;
		count = isc_vax_integer(ptr, l);
		ptr += l;
		//	n_warn(N, __FN__, "count_type=%d, l=%d, count=%d", count_type, l, count);
		if (count_type != isc_info_req_select_count && count_type != isc_info_req_insert_count && count_type != isc_info_req_update_count && count_type != isc_info_req_delete_count) break;
		//	if (count_type == 0) break;
		switch (count_type) {

		case isc_info_req_update_count:
			//n_warn(N, __FN__, "Records updated : %d", count);
			updated += count;
			break;
		case isc_info_req_delete_count:
			//n_warn(N, __FN__, "Records deleted : %d", count);
			updated += count;
			break;
		case isc_info_req_select_count:
			//n_warn(N, __FN__, "Records retrieved : %d", count);
			break;
		case isc_info_req_insert_count:
			//n_warn(N, __FN__, "Records inserted : %d", count);
			updated += count;
			break;
		}
	}
	return updated;
}

/*
 *    Print column's data.
 */
static void store_field(nsp_state *N, FBSQL_CONN *conn, XSQLVAR ISC_FAR *var, obj_t *tobj)
{
#define __FN__ __FILE__ ":fbsql store_field()"
	char  buf[1024];
	char  blob_s[20];
	char  date_s[25];
	short dtype;
	struct tm times;

	dtype = var->sqltype & ~1;
	/* Null handling.  If the column is nullable and null */
	if ((var->sqltype & 1) && (*var->sqlind < 0)) {
		nsp_setstr(N, tobj, var->aliasname, NULL, 0);
		return;
	}
	buf[0] = '\0';
	switch (dtype) {
	case SQL_TEXT:
		nsp_setstr(N, tobj, var->aliasname, var->sqldata, var->sqllen);
		return;
	case SQL_VARYING: {
		VARY *vary = (VARY *)var->sqldata;

		nsp_setstr(N, tobj, var->aliasname, vary->vary_string, vary->vary_length);
		return;
	}
	case SQL_SHORT:
	case SQL_LONG:
	case SQL_INT64: {
		ISC_INT64 value = 0;
		short     field_width = 0;
		short     dscale;

		switch (dtype) {
		case SQL_SHORT:
			value = (ISC_INT64) * (short ISC_FAR *) var->sqldata;
			field_width = 6;
			break;
		case SQL_LONG:
			// breaks on Linux x86_64 due to wrong assumption about size of long
			//value = (ISC_INT64) *(long ISC_FAR *) var->sqldata;
			value = (ISC_INT64) * (int ISC_FAR *) var->sqldata;
			field_width = 11;
			break;
		case SQL_INT64:
			value = (ISC_INT64) * (ISC_INT64 ISC_FAR *) var->sqldata;
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
				sprintf(buf, "%*" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d", field_width - 1 + dscale, (ISC_INT64)value / tens, -dscale, (ISC_INT64)value % tens);
			else if ((value / tens) != 0)
				sprintf(buf, "%*" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d", field_width - 1 + dscale, (ISC_INT64)(value / tens), -dscale, (ISC_INT64)-(value % tens));
			else
				sprintf(buf, "%*s.%0*" ISC_INT64_FORMAT "d", field_width - 1 + dscale, "-0", -dscale, (ISC_INT64)-(value % tens));
		} else if (dscale) {
			sprintf(buf, "%*" ISC_INT64_FORMAT "d%0*d", field_width, (ISC_INT64)value, dscale, 0);
		} else {
			//sprintf(buf, "%" ISC_INT64_FORMAT "d", (ISC_INT64)value);
			nsp_setnum(N, tobj, var->aliasname, (double)value);
			return;
		}
		nsp_setstr(N, tobj, var->aliasname, buf, -1);
		return;
	}
	case SQL_FLOAT:
		sprintf(buf, "%15g", *(float ISC_FAR *) (var->sqldata));
		nsp_setstr(N, tobj, var->aliasname, buf, -1);
		return;
	case SQL_DOUBLE:
		sprintf(buf, "%24f", *(double ISC_FAR *) (var->sqldata));
		nsp_setstr(N, tobj, var->aliasname, buf, -1);
		return;
	case SQL_TIMESTAMP:
		isc_decode_timestamp((ISC_TIMESTAMP ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%04d-%02d-%02d %02d:%02d:%02d",
			times.tm_year + 1900,
			times.tm_mon + 1,
			times.tm_mday,
			times.tm_hour,
			times.tm_min,
			times.tm_sec
		);
		nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		return;
	case SQL_TYPE_DATE:
		isc_decode_sql_date((ISC_DATE ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%04d-%02d-%02d",
			times.tm_year + 1900,
			times.tm_mon + 1,
			times.tm_mday);
		nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		return;
	case SQL_TYPE_TIME:
		isc_decode_sql_time((ISC_TIME ISC_FAR *)var->sqldata, &times);
		sprintf(date_s, "%02d:%02d:%02d",
			times.tm_hour,
			times.tm_min,
			times.tm_sec
		);
		nsp_setstr(N, tobj, var->aliasname, date_s, -1);
		return;
	case SQL_BLOB: {
		ISC_QUAD        blob_id = *(ISC_QUAD ISC_FAR *)var->sqldata;
		isc_blob_handle blob_handle = 0;
		short           blob_seg_len;
		char            blob_segment[256];
		long            blob_stat;
		obj_t *cobj;

		cobj = nsp_setstr(N, tobj, var->aliasname, "NULL", 0);
		/* Open the blob with the fetched blob_id. */
		if (isc_open_blob(conn->status, &conn->db, &conn->trans, &blob_handle, &blob_id)) {
			print_warning(N, conn->status, __FN__, "isc_open_blob");
		}
		/* Get blob segments and their lengths and print each segment. */
		blob_stat = isc_get_segment(conn->status, &blob_handle, (unsigned short *)&blob_seg_len, sizeof(blob_segment), blob_segment);
		while (blob_stat == 0 || conn->status[1] == isc_segment) {
			nsp_strcat(N, cobj, blob_segment, blob_seg_len);
			//printf("[%*.*s][%d]\r\n", blob_seg_len, blob_seg_len, blob_segment, blob_seg_len);
			blob_stat = isc_get_segment(conn->status, &blob_handle, (unsigned short *)&blob_seg_len, sizeof(blob_segment), blob_segment);
		}
		/* Close the blob.  Should be blob_stat to check */
		if (conn->status[1] == isc_segstr_eof) {
			if (isc_close_blob(conn->status, &blob_handle)) {
				print_warning(N, conn->status, __FN__, "isc_close_blob");
			}
		}
		return;
	}
	case SQL_ARRAY: {
		/* Print the blob id on blobs or arrays */
		ISC_QUAD  bid = *(ISC_QUAD ISC_FAR *)var->sqldata;

		sprintf(blob_s, "%08x:%08x", (unsigned int)bid.isc_quad_high, (unsigned int)bid.isc_quad_low);
		sprintf(buf, "%17s", blob_s);
		break;
	}
	default:
		n_warn(N, __FN__, "Unhandled type!");
		break;
	}
	nsp_setstr(N, tobj, var->aliasname, buf, -1);
#undef __FN__
}

static void fbsqlDisconnect(nsp_state *N, FBSQL_CONN *conn)
{
#define __FN__ __FILE__ ":fbsqlDisconnect()"
	if (isc_detach_database(conn->status, &conn->db)) {
		print_warning(N, conn->status, __FN__, "isc_detach_database");
	}
	if (conn->db != 0) {
		conn->db = 0;
	}
	return;
#undef __FN__
}

static int fbsqlConnect(nsp_state *N, FBSQL_CONN *conn, char *host, char *user, char *pass, char *db, int port)
{
#define __FN__ __FILE__ ":fbsqlConnect()"
	char buf[1024];
	char buf2[1024];
	short len = 0;
	long rc;

	if (user && pass) {
		len = snprintf(buf, sizeof(buf), "%c%c%c%s%c%c%s", isc_dpb_version1, isc_dpb_user_name, (int)strlen(user), user, isc_dpb_password, (int)strlen(pass), pass);
		snprintf(buf2, sizeof(buf2), "%s:%s", host, db);
		rc = isc_attach_database(conn->status, 0, buf2, &conn->db, len, buf);
	} else {
		snprintf(buf2, sizeof(buf2), "%s:%s", host, db);
		rc = isc_attach_database(conn->status, 0, buf2, &conn->db, 0, NULL);
	}
	if (rc) {
		n_warn(N, __FN__, "connection error %s - %s", db, conn->status);
		return -1;
	}
	return 0;
#undef __FN__
}

void fbsql_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":fbsql_murder()"
	FBSQL_CONN *conn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	fbsqlDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(FBSQL_CONN) + 1);
	cobj->val->size = 0;
	return;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_open)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_open()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	FBSQL_CONN *conn;
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
	conn = n_alloc(N, sizeof(FBSQL_CONN) + 1, 1);
	strcpy(conn->obj_type, "fbsql-conn");
	conn->obj_term = (NSP_CFREE)fbsql_murder;
	rc = fbsqlConnect(N, conn, host, user, pass, db, port);
	if (rc < 0) {
		n_free(N, (void *)&conn, sizeof(FBSQL_CONN) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
	cobj->val->d.str = (void *)conn;
	cobj->val->size = sizeof(FBSQL_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_close)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_close()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	FBSQL_CONN *conn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	fbsqlDisconnect(N, conn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(FBSQL_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "connection", 0);
	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_query)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_query()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t *cobj;
	FBSQL_CONN *conn;
	char *sqlquery;
	short expect_results = 1;

	XSQLVAR ISC_FAR *var;
	unsigned short  i;
	short           length, alignment, type, offset;
	char            stmt_info[] = { isc_info_sql_stmt_type };
	char            info_buffer[20];
	short           l;
	long            statement_type = 0;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	sqlquery = cobj1->val->d.str;

	if (nsp_isbool((cobj = nsp_getobj(N, &N->context->l, "2")))) {
		expect_results = nsp_tobool(N, cobj);
	}

	/* Allocate a global statement */
	if (isc_dsql_allocate_statement(conn->status, &conn->db, &conn->stmt)) {
		print_warning(N, conn->status, __FN__, "isc_dsql_allocate_statement");
		n_error(N, NE_SYNTAX, __FN__, "isc_dsql_allocate_statement");
		return 1;
	}

	/*
	 *    Allocate enough space for 20 fields.
	 *    If more fields get selected, re-allocate SQLDA later.
	 */
	conn->sqlda = (XSQLDA ISC_FAR *) malloc(XSQLDA_LENGTH(20));
	conn->sqlda->sqln = 20;
	conn->sqlda->version = 1;
	if (!conn->trans) {
		if (isc_start_transaction(conn->status, &conn->trans, 1, &conn->db, 0, NULL)) {
			print_warning(N, conn->status, __FN__, "isc_start_transaction");
			return 1;
		}
		conn->trans_block = 0;
	}
	if (isc_dsql_prepare(conn->status, &conn->trans, &conn->stmt, 0, sqlquery, SQL_DIALECT_V6, conn->sqlda)) {
		print_warning(N, conn->status, __FN__, "isc_dsql_prepare");
		return 2;
	}
	/* What is the statement type of this statement?
	**
	** stmt_info is a 1 byte info request.  info_buffer is a buffer
	** large enough to hold the returned info packet
	** The info_buffer returned contains a isc_info_sql_stmt_type in the first byte,
	** two bytes of length, and a statement_type token.
	*/
	if (!isc_dsql_sql_info(conn->status, &conn->stmt, sizeof(stmt_info), stmt_info, sizeof(info_buffer), info_buffer)) {
		l = (short)isc_vax_integer((char ISC_FAR *) info_buffer + 1, 2);
		statement_type = isc_vax_integer((char ISC_FAR *) info_buffer + 3, l);
	}
	/*
	 *    Execute a non-select statement.
	 */
	 //	if (!expect_results || !conn->sqlda->sqld) {
	if (!conn->sqlda->sqld) {
		if (isc_dsql_execute(conn->status, &conn->trans, &conn->stmt, SQL_DIALECT_V6, NULL)) {
			print_warning(N, conn->status, __FN__, "isc_dsql_execute");
			return 2;
		}
		/* Commit DDL statements if that is what sql_info says */
		if (conn->trans && conn->trans_block == 0 && (statement_type == isc_info_sql_stmt_ddl)) {
			//printf ("\tCommitting...\n");
			if (isc_commit_transaction(conn->status, &conn->trans)) {
				print_warning(N, conn->status, __FN__, "isc_commit_transaction");
				return 2;
			}
		}
		if (!expect_results) {
			nsp_setnum(N, thisobj, "changes", rows_affected(N, conn));
		}
		conn->fetch_stat = 100L;



		if (!expect_results) {
			// this is more than a little broken, but I can't look at this code any more today
			if (conn->stmt) {
				if (isc_dsql_free_statement(conn->status, &conn->stmt, DSQL_drop)) {
					print_warning(N, conn->status, __FN__, "isc_dsql_free_statement");
				}
			}
			if (conn->fetch_stat != 100L) {
				/* print_warning(N, conn->status, __FN__, "conn->fetch_stat != 100L (are there unfetched rows?)"); */
			}
			conn->fetch_stat = 0;
			if (conn->sqlda) {
				free(conn->sqlda);
				conn->sqlda = NULL;
			}
			if (conn->trans && conn->trans_block == 0) {
				if (isc_commit_transaction(conn->status, &conn->trans)) {
					print_warning(N, conn->status, __FN__, "isc_commit_transaction");
					return 0;
				}
				conn->trans = 0;
			}
			conn->stmt = 0;
		}


		return 0;
	}

	/*
	 *    Process select statements.
	 */
	conn->numfields = conn->sqlda->sqld;
	/* Need more room. */
	if (conn->sqlda->sqln < conn->numfields) {
		conn->sqlda = (XSQLDA ISC_FAR *) realloc(conn->sqlda, XSQLDA_LENGTH(conn->numfields));
		conn->sqlda->sqln = conn->numfields;
		conn->sqlda->version = 1;
		if (isc_dsql_describe(conn->status, &conn->stmt, SQL_DIALECT_V6, conn->sqlda)) {
			print_warning(N, conn->status, __FN__, "isc_dsql_describe");
			return 2;
		}
		conn->numfields = conn->sqlda->sqld;
	}
	conn->numtuples = 0;
//	cobj = nsp_settable(N, thisobj, "last_query");
//	nsp_setstr(N, cobj, "_query", sqlquery, -1);
//	nsp_setnum(N, cobj, "_fields", conn->numfields);
//	nsp_setstr(N, cobj, "_tuples", "firebird does not support this", -1);
	nsp_setstr(N, thisobj, "last_query", sqlquery, -1);
	if (expect_results) {
		nsp_setnum(N, thisobj, "changes", -1);
	}
	/*
	 *     Set up SQLDA.
	 */
	for (var = conn->sqlda->sqlvar, offset = 0, i = 0; i < conn->numfields; var++, i++) {
		length = alignment = var->sqllen;
		type = var->sqltype & ~1;
		if (type == SQL_TEXT) {
			alignment = 1;
		} else if (type == SQL_VARYING) {
			length += sizeof(short) + 1;
			alignment = sizeof(short);
		}
		/*  RISC machines are finicky about word alignment
		**  So the output buffer values must be placed on
		**  word boundaries where appropriate
		*/
		offset = FB_ALIGN(offset, alignment);
		var->sqldata = (char ISC_FAR *) conn->buffer + offset;
		offset += length;
		offset = FB_ALIGN(offset, sizeof(short));
		var->sqlind = (short*)((char ISC_FAR *) conn->buffer + offset);
		offset += sizeof(short);
	}
	if (isc_dsql_execute(conn->status, &conn->trans, &conn->stmt, SQL_DIALECT_V6, NULL)) {
		print_warning(N, conn->status, __FN__, "isc_dsql_execute");
		return 2;
	}











	nsp_setnum(N, &N->r, "", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_getnext()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	obj_t tobj;
	FBSQL_CONN *conn;
	int field;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	if ((conn->fetch_stat = isc_dsql_fetch(conn->status, &conn->stmt, SQL_DIALECT_V6, conn->sqlda)) != 0) {
		return 0;
	}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	for (field = 0; field < conn->numfields; field++) {
		store_field(N, conn, (XSQLVAR ISC_FAR *) & conn->sqlda->sqlvar[field], &tobj);
	}
	conn->numtuples++;
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_endquery()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj = nsp_getobj(N, nsp_getobj(N, &N->context->l, "this"), "connection");
	FBSQL_CONN *conn;

	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	nsp_setnum(N, thisobj, "changes", rows_affected(N, conn));
	if (conn->stmt) {
		if (isc_dsql_free_statement(conn->status, &conn->stmt, DSQL_drop)) {
			print_warning(N, conn->status, __FN__, "isc_dsql_free_statement");
		}
	}
	if (conn->fetch_stat != 100L) {
		/* print_warning(N, conn->status, __FN__, "conn->fetch_stat != 100L (are there unfetched rows?)"); */
	}
	conn->fetch_stat = 0;
	if (conn->sqlda) {
		free(conn->sqlda);
		conn->sqlda = NULL;
	}
	if (conn->trans && conn->trans_block == 0) {
		if (isc_commit_transaction(conn->status, &conn->trans)) {
			print_warning(N, conn->status, __FN__, "isc_commit_transaction");
			return 0;
		}
		conn->trans = 0;
	}
	conn->stmt = 0;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_begin)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_begin()"
	obj_t *cobj = nsp_getobj(N, nsp_getobj(N, &N->context->l, "this"), "connection");
	FBSQL_CONN *conn;

	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	if (!conn->trans) {
		if (isc_start_transaction(conn->status, &conn->trans, 1, &conn->db, 0, NULL)) {
			print_warning(N, conn->status, __FN__, "isc_start_transaction");
			return 0;
		}
		conn->trans_block = 1;
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_commit)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_commit()"
	obj_t *cobj = nsp_getobj(N, nsp_getobj(N, &N->context->l, "this"), "connection");
	FBSQL_CONN *conn;

	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	if (conn->trans && conn->trans_block) {
		if (isc_commit_transaction(conn->status, &conn->trans)) {
			print_warning(N, conn->status, __FN__, "isc_commit_transaction");
			return 0;
		}
		conn->trans_block = 0;
		conn->trans = 0;
	} else {
		n_warn(N, __FN__, "no transaction in progress");
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_sql_fbsql_rollback)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_rollback()"
	obj_t *cobj = nsp_getobj(N, nsp_getobj(N, &N->context->l, "this"), "connection");
	FBSQL_CONN *conn;

	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "fbsql-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a fbsql-conn");
	conn = (FBSQL_CONN *)cobj->val->d.str;
	if (conn->trans && conn->trans_block) {
		if (isc_rollback_transaction(conn->status, &conn->trans)) {
			print_warning(N, conn->status, __FN__, "isc_rollback_transaction");
			return 0;
		}
		conn->trans_block = 0;
		conn->trans = 0;
	} else {
		n_warn(N, __FN__, "no transaction in progress");
	}
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_data_sql_fbsql_client)
{
#define __FN__ __FILE__ ":libnsp_data_sql_fbsql_client()"
	obj_t *tobj, *cobj;

	nsp_setstr(N, &N->context->l, "host", "localhost", 9);
	nsp_setnum(N, &N->context->l, "port", 3050);
	nsp_setstr(N, &N->context->l, "username", "SYSDBA", 6);
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

	cobj = nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "fbsql");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->context->l, cobj);
	else n_warn(N, __FN__, "data.fbsql not found");
	cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "data"), "sql"), "common");
	if (nsp_istable(cobj)) nsp_zlink(N, &N->context->l, cobj);
	else n_warn(N, __FN__, "data.sql.common not found");
	return 0;
#undef __FN__
}

int nspfbsql_register_all(nsp_state *N)
{
	obj_t *tobj;

	tobj = nsp_settable(N, nsp_settable(N, &N->g, "lib"), "data");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "fbsql");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "client", (NSP_CFUNC)libnsp_data_sql_fbsql_client);
	nsp_setcfunc(N, tobj, "open", (NSP_CFUNC)libnsp_data_sql_fbsql_open);
	nsp_setcfunc(N, tobj, "close", (NSP_CFUNC)libnsp_data_sql_fbsql_close);
	nsp_setcfunc(N, tobj, "query", (NSP_CFUNC)libnsp_data_sql_fbsql_query);
	nsp_setcfunc(N, tobj, "getnext", (NSP_CFUNC)libnsp_data_sql_fbsql_getnext);
	nsp_setcfunc(N, tobj, "endquery", (NSP_CFUNC)libnsp_data_sql_fbsql_endquery);
	nsp_setcfunc(N, tobj, "begin", (NSP_CFUNC)libnsp_data_sql_fbsql_begin);
	nsp_setcfunc(N, tobj, "commit", (NSP_CFUNC)libnsp_data_sql_fbsql_commit);
	nsp_setcfunc(N, tobj, "rollback", (NSP_CFUNC)libnsp_data_sql_fbsql_rollback);

	// nsp_setcfunc(N, tobj, "escape", (NSP_CFUNC)libnsp_data_sql_fbsql_escape);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspfbsql_register_all(N);
	return 0;
}
#endif

#endif /* HAVE_FBSQL */
