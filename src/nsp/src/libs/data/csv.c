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
#include "data.h"

typedef struct CSV_CONN {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's object-specific */
	int fd;
	unsigned long offset;
} CSV_CONN;

static void csv_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":csv_murder()"
	//CSV_CONN *dbconn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "csv-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a csv conn");
	//dbconn=(CSV_CONN *)cobj->val->d.str;
	n_free(N, (void *)&cobj->val->d.str, sizeof(CSV_CONN) + 1);
	return;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_csv_open)
{
#define __FN__ __FILE__ ":libnsp_data_csv_open()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj = NULL;
	CSV_CONN *dbconn;
	char *dbfile = NULL;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	dbconn = n_alloc(N, sizeof(CSV_CONN) + 1, 1);
	if (dbconn == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(CSV_CONN) + 1);
		return -1;
	}
	nc_strncpy(dbconn->obj_type, "csv-conn", sizeof(dbconn->obj_type) - 1);
	dbconn->obj_term = (NSP_CFREE)csv_murder;
	if (nsp_isstr((cobj = nsp_getobj(N, &N->l, "1")))) {
		dbfile = cobj->val->d.str;
	}
	else if (nsp_isstr((cobj = nsp_getobj(N, thisobj, "dbfile")))) {
		dbfile = cobj->val->d.str;
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for dbfile");
	}
	if ((dbconn->fd = open(dbfile, O_RDONLY | O_BINARY)) == -1) {
		nsp_setnum(N, &N->r, "", -1);
		n_free(N, (void *)&dbconn, sizeof(CSV_CONN) + 1);
		return -1;
	}
	cobj = nsp_setcdata(N, thisobj, "db", NULL, 0);
	cobj->val->d.str = (void *)dbconn;
	cobj->val->size = sizeof(CSV_CONN) + 1;
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_csv_close)
{
#define __FN__ __FILE__ ":libnsp_data_csv_close()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	CSV_CONN *dbconn;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "csv-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a csv-conn");
	dbconn = (CSV_CONN *)cobj->val->d.str;
	close(dbconn->fd);
	n_free(N, (void *)&cobj->val->d.str, sizeof(CSV_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "db", 0);
	return 0;
#undef __FN__
}

#define MAXBUF 512

NSP_CLASSMETHOD(libnsp_data_csv_getnext)
{
#define __FN__ __FILE__ ":libnsp_data_csv_getnext()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	CSV_CONN *dbconn;
	char buf[MAXBUF + 1];
	char *separator = ",";
	obj_t tobj;
	char *ss, *se;
	int i = 0;
	char *s2;
	int l2;
	char namebuf[MAX_OBJNAMELEN + 1];
	int r;

	settrace();
	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	cobj = nsp_getobj(N, thisobj, "db");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (nc_strcmp(cobj->val->d.str, "csv-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a csv-conn");
	dbconn = (CSV_CONN *)cobj->val->d.str;

	lseek(dbconn->fd, dbconn->offset, SEEK_SET);
	if ((r = read(dbconn->fd, buf, MAXBUF)) < 0) n_error(N, NE_SYNTAX, __FN__, "error reading csv");;
	if (r == 0) return 0;
	buf[r] = '\0';
	tobj.val = n_newval(N, NT_TABLE);
	tobj.val->attr |= NST_AUTOSORT;
	se = ss = buf;
	s2 = separator;
	l2 = nc_strlen(separator);
	for (;*se;) {
		if (*se == '"') {
			dbconn->offset++;
			ss = ++se;
			for (;*se;se++, dbconn->offset++) {
				if (*se != '"') continue;
				nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
				ss = ++se; dbconn->offset++;
				break;
			}
			if (nc_strncmp(se, s2, l2) == 0) {
				ss = se += l2;
				dbconn->offset += l2;
			}
			continue;
		}
		if (nc_strncmp(se, s2, l2) != 0 && *se != '\r' && *se != '\n') {
			dbconn->offset++;
			se++;
			continue;
		}
		if (se[0] == '\r' && se[1] == '\n') {
			dbconn->offset += 2;
			break;
		}
		if (*se == '\n') {
			dbconn->offset++;
			break;
		}
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
		if (*se != '\r' && *se != '\n') { ss = se += l2; dbconn->offset += l2; }
		if (*se != '\0' && *se != '\r' && *se != '\n') continue;
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
		if (*se == '\r' || *se == '\n') dbconn->offset++;
		if (*se == '\r' || *se == '\n') dbconn->offset++;
		break;
	}
	if (se > ss) {
		nsp_setstr(N, &tobj, n_ntoa(N, namebuf, i++, 10, 0), ss, se - ss);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_csv_query)
{
#define __FN__ __FILE__ ":libnsp_data_csv_query()"
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_data_csv_endquery)
{
#define __FN__ __FILE__ ":libnsp_data_csv_endquery()"
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_data_csv_reader)
{
#define __FN__ __FILE__ ":libnsp_data_csv_reader()"
	nsp_setcfunc(N, &N->l, "open", (NSP_CFUNC)libnsp_data_csv_open);
	nsp_setcfunc(N, &N->l, "close", (NSP_CFUNC)libnsp_data_csv_close);
	nsp_setcfunc(N, &N->l, "query", (NSP_CFUNC)libnsp_data_csv_query);
	nsp_setcfunc(N, &N->l, "getnext", (NSP_CFUNC)libnsp_data_csv_getnext);
	nsp_setcfunc(N, &N->l, "endquery", (NSP_CFUNC)libnsp_data_csv_endquery);
	nsp_setstr(N, &N->l, "dbfile", "", 0);
	nsp_setbool(N, &N->l, "db", 0);
	return 0;
#undef __FN__
}
