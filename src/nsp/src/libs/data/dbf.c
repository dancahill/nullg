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
#include "data.h"

#define DBF_NAMELEN 11
#define DBF_RECBUFROWS 256

typedef struct {
	uchar dbh_dbt;        /* indentification field */
	uchar dbh_year;       /* last modification-date */
	uchar dbh_month;
	uchar dbh_day;
	uchar dbh_records[4]; /* number of records */
	uchar dbh_hlen[2];    /* length of this header */
	uchar dbh_rlen[2];    /* length of a record */
	uchar dbh_stub[20];   /* misc stuff we don't need */
} dbf_header;

typedef struct {
	uchar dbf_name[DBF_NAMELEN]; /* field-name terminated with \0 */
	uchar dbf_type;              /* field-type */
	uchar dbf_reserved[4];       /* some reserved stuff */
	uchar dbf_flen;              /* field-length */
	uchar dbf_dec;               /* number of decimal positions if type is * 'N' */
	uchar dbf_stub[14];          /* stuff we don't need */
} dbf_field;

typedef struct DBF_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's object-specific */
	dbf_header header;
	dbf_field  field[64];
	int fd;
	unsigned long offset;
	int rowsize;
	int nfields;
	int nrows;
	int nrows_expected;

	char *recbuf;
	unsigned long recbuflen;
	unsigned long recbufoffset;
} DBF_CONN;

static short get_short(uchar *cp)
{
	short ret;

	ret = *cp++;
	ret += ((*cp++) << 8);
	return ret;
}

static long get_long(uchar *cp)
{
	long ret;

	ret = *cp++;
	ret += ((*cp++) << 8);
	ret += ((*cp++) << 16);
	ret += ((*cp++) << 24);
	return ret;
}
/*
static void put_short(uchar *cp, short sval)
{
	cp[0] = sval & 0xff;
	cp[1] = (sval >> 8) & 0xff;
}

static void put_long(uchar *cp, long lval)
{
	cp[0] = lval & 0xff;
	cp[1] = (lval >> 8) & 0xff;
	cp[2] = (lval >> 16) & 0xff;
	cp[3] = (lval >> 24) & 0xff;
}
*/
static void dbf_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":dbf_murder()"
	DBF_CONN *dbconn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "dbf-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a dbf conn");
	dbconn = (DBF_CONN *)cobj->val->d.str;
	if (dbconn->recbuf != NULL) n_free(N, (void *)&dbconn->recbuf, dbconn->rowsize * DBF_RECBUFROWS);
	n_free(N, (void *)&cobj->val->d.str, sizeof(DBF_CONN) + 1);
	return;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_dbf_open)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_open()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj = NULL;
	obj_t tobj;
	DBF_CONN *dbconn;
	char *dbfile = NULL;
	char fbuf;
	int r;
	unsigned long hlen;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	dbconn = n_alloc(N, sizeof(DBF_CONN) + 1, 1);
	if (dbconn == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(DBF_CONN) + 1);
		return -1;
	}
	nc_strncpy(dbconn->obj_type, "dbf-conn", sizeof(dbconn->obj_type) - 1);
	dbconn->obj_term = (NSP_CFREE)dbf_murder;
	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "1")))) {
		dbfile = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "dbfile")))) {
		dbfile = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for dbfile");
	}
	if ((dbconn->fd = open(dbfile, O_RDONLY | O_BINARY)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		n_free(N, (void *)&dbconn, sizeof(DBF_CONN) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "db", NULL, 0);
	cobj->val->d.str = (void *)dbconn;
	cobj->val->size = sizeof(DBF_CONN) + 1;
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	/* read the dbf header */
	r = read(dbconn->fd, &dbconn->header, sizeof(dbf_header));
	if (r != sizeof(dbf_header)) goto err;
	dbconn->offset += r;
	hlen = get_short((uchar *)&dbconn->header.dbh_hlen);
	dbconn->nrows_expected = get_long((uchar *)&dbconn->header.dbh_records);
	do {
		if (dbconn->offset + sizeof(dbf_field) > hlen) {
			r = read(dbconn->fd, &fbuf, 1);
			if (r > 0) dbconn->offset += r;
			if (fbuf == 13) {
				nsp_unlinkval(N, &N->r);
				nsp_unlinkval(N, &tobj);
				return 0;
			} else {
				n_warn(N, __FN__, "end of fields...ERROR");
				goto err;
			}
			break;
		}
		/* read the dbf fields one at a time */
		r = read(dbconn->fd, &dbconn->field[dbconn->nfields], sizeof(dbf_field));
		if (r != 32) goto err;
		if (r > 0) dbconn->offset += r;
		dbconn->rowsize += dbconn->field[dbconn->nfields].dbf_flen;
		dbconn->nfields++;
	} while (1);
	goto done;
err:
	n_warn(N, __FN__, "ERROR?");
	close(dbconn->fd);
done:
	tobj.val->d.table.l = cobj;
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);

	dbconn->recbuf = NULL;
	dbconn->recbuflen = 0;
	dbconn->recbufoffset = 0;

	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_dbf_close)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_close()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	DBF_CONN *dbconn;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "dbf-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a dbf-conn");
	dbconn = (DBF_CONN *)cobj->val->d.str;
	close(dbconn->fd);
	if (dbconn->recbuf != NULL) n_free(N, (void *)&dbconn->recbuf, dbconn->rowsize * DBF_RECBUFROWS);
	n_free(N, (void *)&cobj->val->d.str, sizeof(DBF_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "db", 0);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_dbf_query)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_query()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	//DBF_CONN *dbconn;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "dbf-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a dbf-conn");
	//dbconn=(DBF_CONN *)cobj->val->d.str;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_dbf_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_getnext()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	obj_t tobj;
	DBF_CONN *dbconn;
	char *p1;
	int i, r;
	unsigned short plen;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "dbf-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a dbf-conn");
	dbconn = (DBF_CONN *)cobj->val->d.str;
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr &= ~NST_AUTOSORT;
	if (dbconn->recbuf == NULL) {
		//		n_warn(N, __FN__, "allocing buffer size of %d", dbconn->rowsize*DBF_RECBUFROWS+DBF_RECBUFROWS);
		dbconn->recbuf = n_alloc(N, dbconn->rowsize * DBF_RECBUFROWS + DBF_RECBUFROWS, 0);
		dbconn->recbuflen = 0;
		dbconn->recbufoffset = 0;
	}
	if (dbconn->recbuflen - dbconn->recbufoffset == 0) {
		dbconn->recbuflen = 0;
		dbconn->recbufoffset = 0;
		dbconn->recbuflen = read(dbconn->fd, dbconn->recbuf, dbconn->rowsize * DBF_RECBUFROWS + DBF_RECBUFROWS);
		if (dbconn->recbuflen < 0) dbconn->recbuflen = 0;
	}
	r = dbconn->recbuflen - dbconn->recbufoffset;
	r = r < dbconn->rowsize ? r : dbconn->rowsize;
	if (r == 1 && dbconn->recbuf[dbconn->recbufoffset] == 26) {
		nsp_unlinkval(N, &N->r);
		nsp_unlinkval(N, &tobj);
		return 0;
	}
	if (r == 0) {
		n_warn(N, __FN__, "zero bytes read at row %d of %d", dbconn->nrows, dbconn->nrows_expected);
		nsp_unlinkval(N, &N->r);
		nsp_unlinkval(N, &tobj);
		return 0;
	}
	if (r != dbconn->rowsize) {
		n_error(N, NE_SYNTAX, __FN__, "row size mismatch at row %d, s=%d r=%d", dbconn->nrows, dbconn->rowsize + 1, r);
	}
	p1 = dbconn->recbuf + dbconn->recbufoffset + 1;
	dbconn->recbufoffset += dbconn->rowsize + 1;
	for (i = 0; i < dbconn->nfields; i++) {
		plen = dbconn->field[i].dbf_flen;
		while (plen > 1 && p1[plen - 1] == ' ') plen--;
		nsp_setstr(N, &tobj, (char *)dbconn->field[i].dbf_name, p1, plen);
		p1 += dbconn->field[i].dbf_flen;
	}
	dbconn->nrows++;
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_dbf_freerow)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_freerow()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "dbf-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a dbf-conn");
	if (nsp_istable((cobj = nsp_getobj(N, &N->context->l, "1")))) {
		nsp_freetable(N, cobj);
	}
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_dbf_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_endquery()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	//DBF_CONN *dbconn;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "dbf-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a dbf-conn");
	//dbconn=(DBF_CONN *)cobj->val->d.str;
	return 0;
#undef __FN__
}

NSP_CLASS(libnsp_data_dbf_reader)
{
#define __FN__ __FILE__ ":libnsp_data_dbf_reader()"
	nsp_setcfunc(N, &N->context->l, "open", (NSP_CFUNC)libnsp_data_dbf_open);
	nsp_setcfunc(N, &N->context->l, "close", (NSP_CFUNC)libnsp_data_dbf_close);
	nsp_setcfunc(N, &N->context->l, "query", (NSP_CFUNC)libnsp_data_dbf_query);
	nsp_setcfunc(N, &N->context->l, "getnext", (NSP_CFUNC)libnsp_data_dbf_getnext);
	nsp_setcfunc(N, &N->context->l, "freerow", (NSP_CFUNC)libnsp_data_dbf_freerow);
	nsp_setcfunc(N, &N->context->l, "endquery", (NSP_CFUNC)libnsp_data_dbf_endquery);
	nsp_setstr(N, &N->context->l, "dbfile", "", 0);
	nsp_setbool(N, &N->context->l, "db", 0);
	return 0;
#undef __FN__
}
