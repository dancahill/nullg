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
#include "smtpd_main.h"

static NSP_FUNCTION(htnsp_sqlquery)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *qobj = NULL;
	int rc;

	if (cobj1->val->type != NT_STRING) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	rc = sql_query(proc->N, &qobj, cobj1->val->d.str);
	nsp_linkval(N, &N->r, qobj);
	nsp_unlinkval(N, qobj);
	free(qobj);
	return rc;
}

static NSP_FUNCTION(htnsp_sqlupdate)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	int rc;

	if (cobj1->val->type != NT_STRING) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		return 0;
	}
	rc = sql_update(proc->N, cobj1->val->d.str);
	return rc;
}

static NSP_FUNCTION(htnsp_sqlgetsequence)
{
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	int rc = -1;

	if (cobj1->val->type != NT_STRING) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s() expected a string for arg1\r\n", nsp_getstr(N, &N->l, "0"));
		nsp_setnum(N, &N->r, "", rc);
		return 0;
	}
	rc = sql_getsequence(proc->N, cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	return rc;
}

static NSP_FUNCTION(smtpnsp_logaccess)
{
	log_access(proc->N, MODSHORTNAME, "%s", nsp_getstr(N, &N->l, "1"));
	return 0;
}






static int smtp_nsp_receive(CONN *conn)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	char filename[512];
	struct stat sb;

	snprintf(filename, sizeof(filename) - 1, "%s/share/scripts/smtp/events.ns", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"));
	fixslashes(filename);
	if (stat(filename, &sb) != 0) {
		snprintf(filename, sizeof(filename) - 1, "%s/scripts/smtp/events.ns", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "lib"));
		fixslashes(filename);
		if (stat(filename, &sb) != 0) return 0;
	}
	nsp_execfile(conn->N, filename);
	if (conn->N->err) {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "smtp_nsp_receive exception: [%s]", conn->N->errbuf);
	}
	else {
		nsp_exec(conn->N, "if (typeof(nsd.smtp.events)=='table') global _smtp_events=new nsd.smtp.events();");
	}
	return conn->N->err;
}

int smtp_nsp_init(CONN *conn)
{
	obj_t *confobj = nsp_getobj(proc->N, &proc->N->g, "CONFIG");
	obj_t *newconfobj;
	obj_t *reqobj;
	//obj_t *recobj;
	obj_t *hsobj, *cobj;
	char varname[64];

	conn->N = nsp_newstate();
	reqobj = nsp_settable(conn->N, &conn->N->g, "_REQUEST");
	//recobj=
	nsp_settable(conn->N, reqobj, "RECIPIENTS");
	/* conn->N->warnformat='h'; */
	hsobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	//hsobj->val->attr |= NST_AUTOSORT;
	nsp_setstr(conn->N, hsobj, "REMOTE_ADDR", conn->socket.RemoteAddr, -1);
	nsp_setnum(conn->N, hsobj, "REMOTE_PORT", conn->socket.RemotePort);
	nsp_setstr(conn->N, hsobj, "SERVER_ADDR", conn->socket.ServerAddr, -1);
	nsp_setnum(conn->N, hsobj, "SERVER_PORT", conn->socket.ServerPort);
	snprintf(varname, sizeof(varname) - 1, "%s %s", SERVER_NAME, PACKAGE_VERSION);
	nsp_setstr(conn->N, hsobj, "SERVER_SOFTWARE", varname, -1);
	nsp_setstr(conn->N, hsobj, "NSP_VERSION", NSP_VERSION, -1);
	newconfobj = nsp_settable(conn->N, &conn->N->g, "_CONFIG");
	newconfobj->val->attr |= NST_AUTOSORT;
	cobj = nsp_getobj(proc->N, nsp_settable(proc->N, confobj, "paths"), "var");
	if (nsp_isstr(cobj)) nsp_setstr(conn->N, nsp_settable(proc->N, newconfobj, "paths"), "var", cobj->val->d.str, cobj->val->size);
	cobj = nsp_getobj(proc->N, confobj, "host_name");
	if (nsp_isstr(cobj)) nsp_setstr(conn->N, newconfobj, "host_name", cobj->val->d.str, cobj->val->size);
	/* cheat and peek at the master process's global var table */
	nsp_linkval(conn->N, nsp_settable(conn->N, &conn->N->g, "MASTERGLOBAL"), &proc->N->g);

	nsp_setcfunc(conn->N, &conn->N->g, "logaccess", (NSP_CFUNC)smtpnsp_logaccess);
	nsp_setcfunc(conn->N, &conn->N->g, "sqlquery", (NSP_CFUNC)htnsp_sqlquery);
	nsp_setcfunc(conn->N, &conn->N->g, "sqlupdate", (NSP_CFUNC)htnsp_sqlupdate);
	nsp_setcfunc(conn->N, &conn->N->g, "sqlgetsequence", (NSP_CFUNC)htnsp_sqlgetsequence);

	smtp_nsp_receive(conn);
	return 0;
}

int smtp_nsp_prefilter(CONN *conn)
{
	obj_t *tobj = nsp_getobj(conn->N, nsp_settable(conn->N, &conn->N->g, "_smtp_events"), "prefilter");
	obj_t *robj;

	if (nsp_isnull(tobj)) return 0;
	robj = nsp_eval(conn->N, "_smtp_events.prefilter();");
	if (nsp_isnum(robj)) return (int)nsp_tonum(conn->N, robj);
	return 0;
}

int smtp_nsp_postfilter(CONN *conn)
{
	obj_t *tobj = nsp_getobj(conn->N, nsp_settable(conn->N, &conn->N->g, "_smtp_events"), "postfilter");
	obj_t *robj;

	if (nsp_isnull(tobj)) return 0;
	robj = nsp_eval(conn->N, "_smtp_events.postfilter();");
	if (nsp_isnum(robj)) return (int)nsp_tonum(conn->N, robj);
	return 0;
}
