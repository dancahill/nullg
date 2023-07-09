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

typedef struct TXT_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's object-specific */
	int fd;
	unsigned long offset;
} TXT_CONN;

static void txt_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":txt_murder()"
	//TXT_CONN *dbconn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "txt-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a txt conn");
	//dbconn=(TXT_CONN *)cobj->val->d.str;
	n_free(N, (void *)&cobj->val->d.str, sizeof(TXT_CONN) + 1);
	return;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_txt_open)
{
#define __FN__ __FILE__ ":libnsp_data_txt_open()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj = NULL;
	TXT_CONN *dbconn;
	char *dbfile = NULL;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	dbconn = n_alloc(N, sizeof(TXT_CONN) + 1, 1);
	if (dbconn == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(TXT_CONN) + 1);
		return -1;
	}
	nc_strncpy(dbconn->obj_type, "txt-conn", sizeof(dbconn->obj_type) - 1);
	dbconn->obj_term = (NSP_CFREE)txt_murder;
	if (nsp_isstr((cobj = nsp_getobj(N, &N->context->l, "1")))) {
		dbfile = cobj->val->d.str;
	} else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "dbfile")))) {
		dbfile = cobj->val->d.str;
	} else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for dbfile");
	}
	if ((dbconn->fd = open(dbfile, O_RDONLY | O_BINARY)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		n_free(N, (void *)&dbconn, sizeof(TXT_CONN) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "db", NULL, 0);
	cobj->val->d.str = (void *)dbconn;
	cobj->val->size = sizeof(TXT_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_txt_close)
{
#define __FN__ __FILE__ ":libnsp_data_txt_close()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TXT_CONN *dbconn;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "txt-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a txt-conn");
	dbconn = (TXT_CONN *)cobj->val->d.str;
	close(dbconn->fd);
	n_free(N, (void *)&cobj->val->d.str, sizeof(TXT_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "db", 0);
	return 0;
#undef __FN__
}

#define MAXBUF 512

NSP_CLASSMETHOD(libnsp_data_txt_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_txt_getnext()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	TXT_CONN *dbconn;
	char buf[MAXBUF + 1];
	char *ss, *se;
	int r;
	//unsigned long oldoffset;

	settrace();
	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "txt-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a txt-conn");
	dbconn = (TXT_CONN *)cobj->val->d.str;

	lseek(dbconn->fd, dbconn->offset, SEEK_SET);
	if ((r = read(dbconn->fd, buf, MAXBUF)) < 0) n_error(N, NE_SYNTAX, __FN__, "error reading txt");
	if (r == 0) return 0;
	buf[r] = '\0';
	se = ss = buf;
	//oldoffset=dbconn->offset;
	for (; *se;) {
		if (se[0] == '\r' && se[1] == '\n') {
			dbconn->offset += 2;
			break;
		} else if (se[0] == '\n') {
			dbconn->offset++;
			break;
		}
		dbconn->offset++;
		se++;
	}
	nsp_setstr(N, &N->r, "", ss, se - ss);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_txt_query)
{
#define __FN__ __FILE__ ":libnsp_data_txt_query()"
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_txt_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_txt_endquery()"
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_data_txt_reader)
{
#define __FN__ __FILE__ ":libnsp_data_txt_reader()"
	nsp_setcfunc(N, &N->context->l, "open", (NSP_CFUNC)libnsp_data_txt_open);
	nsp_setcfunc(N, &N->context->l, "close", (NSP_CFUNC)libnsp_data_txt_close);
	nsp_setcfunc(N, &N->context->l, "query", (NSP_CFUNC)libnsp_data_txt_query);
	nsp_setcfunc(N, &N->context->l, "getnext", (NSP_CFUNC)libnsp_data_txt_getnext);
	nsp_setcfunc(N, &N->context->l, "endquery", (NSP_CFUNC)libnsp_data_txt_endquery);
	nsp_setstr(N, &N->context->l, "dbfile", "", 0);
	nsp_setbool(N, &N->context->l, "db", 0);
	return 0;
#undef __FN__
}
