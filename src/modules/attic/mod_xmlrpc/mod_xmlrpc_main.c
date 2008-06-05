/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#define SRVMOD_MAIN 1
#include "mod_xmlrpc.h"

void xmlrpc_addmember(CONN *sid, char *name, char *type, const char *format, ...)
{
	char value[1024];
	char timebuf[20];
	va_list ap;
	time_t t;

	memset(value, 0, sizeof(value));
	va_start(ap, format);
	vsnprintf(value, sizeof(value)-1, format, ap);
	va_end(ap);
	if (strncasecmp(type, "dateTime", 8)==0) type="dateTime.iso8601";
	prints(sid, "<member><name>%s</name><value><%s>", name, type);
	if (strcasecmp("dateTime.iso8601", type)==0) {
		t=atoi(value);
		strftime(timebuf, sizeof(timebuf)-1, "%Y%m%dT%H:%M:%S", gmtime(&t));
		prints(sid, "%s", timebuf);
	} else {
		prints(sid, "%s", str2html(sid, value));
	}
	prints(sid, "</%s></value></member>\r\n", type);
	return;
}

void xmlrpc_fault(CONN *sid, int faultid, char *fault)
{
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n<fault>\r\n");
	prints(sid, "<value>\r\n<struct>\r\n");
	xmlrpc_addmember(sid, "faultCode",   "int",    "%d", faultid);
	xmlrpc_addmember(sid, "faultString", "string", "%s", fault);
	prints(sid, "</struct>\r\n</value>\r\n");
	prints(sid, "</fault>\r\n</methodResponse>\r\n");
	return;
}

void xmlrpc_domaindump_addstruct(CONN *sid, obj_t **qobj1, int pid)
{
	obj_t *qobj2=NULL;
	unsigned int i;
	char *ptemp;

	if (pid>1000) return;
	if (sql_queryf(proc->N, &qobj2, "SELECT * FROM nullgs_attributes WHERE pid = %d ORDER BY pid, id", pid)<0) return;
	prints(sid, "<value>\r\n<struct>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj2);i++) {
		ptemp="string";
		if (strcasecmp(sql_getvalue(proc->N, &qobj2, i, 3), "createTimestamp")==0) ptemp="dateTime";
		if (strcasecmp(sql_getvalue(proc->N, &qobj2, i, 3), "modifyTimestamp")==0) ptemp="dateTime";
		xmlrpc_addmember(sid, sql_getvalue(proc->N, &qobj2, i, 3), ptemp, "%s", sql_getvalue(proc->N, &qobj2, i, 4));
	}
/*
	xmlrpc_addmember(sid, "contactid",      "int",      "%d", atoi(sql_getvalue(proc->N, &qobj2, tuple, 0)));
	xmlrpc_addmember(sid, "obj_ctime",      "dateTime", "%d", time_sql2unix(sql_getvalue(proc->N, &qobj2, tuple, 1)));
	xmlrpc_addmember(sid, "username",       "string",   "%s", sql_getvalue(proc->N, &qobj2, tuple, 9));
*/
	prints(sid, "</struct>\r\n</value>\r\n");
	sql_freeresult(proc->N, &qobj2);
	return;
}

void xmlrpc_domaindump(CONN *sid)
{
	obj_t *qobj=NULL;
	int i;

	send_header(sid, 0, 200, "1", "text/xml", -1, -1);
/*
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		xmlrpc_fault(sid, -1, lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		rc=sql_queryf(&sqr, "SELECT * from gw_contacts ORDER BY contactid ASC");
	} else {
		rc=sql_queryf(&sqr, "SELECT * from gw_contacts WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY contactid ASC", sid->dat->uid, sid->dat->gid);
	}
*/
	if (sql_queryf(proc->N, &qobj, "SELECT * FROM nullgs_entries WHERE did = 1 ORDER BY pid, id")<0) {
		xmlrpc_fault(sid, -1, lang_gets(sid, "common", "err_noaccess"));
		return;
	}
/*
	if (rc<0) {
		xmlrpc_fault(sid, -1, lang_gets(sid, "common", "err_noaccess"));
		return;
	}
*/
	prints(sid, "<?xml version=\"1.0\"?>\r\n");
	prints(sid, "<methodResponse>\r\n<params>\r\n<param>\r\n");
	prints(sid, "<value>\r\n<array>\r\n<data>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj);i++) {
		xmlrpc_domaindump_addstruct(sid, &qobj, atoi(sql_getvalue(proc->N, &qobj, i, 0)));
	}
	prints(sid, "</data>\r\n</array>\r\n</value>\r\n");
	prints(sid, "</param>\r\n</params>\r\n</methodResponse>\r\n");
	sql_freeresult(proc->N, &qobj);
	return;
}

void dsml_domaindump(CONN *sid)
{
	obj_t *qobj1=NULL;
//	obj_t *qobj2=NULL;
	unsigned int i;
//	unsigned int j;
	int pid;

	send_header(sid, 0, 200, "1", "text/xml", -1, -1);
/*
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		xmlrpc_fault(sid, -1, lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		rc=sql_queryf(&sqr, "SELECT * from gw_contacts ORDER BY contactid ASC");
	} else {
		rc=sql_queryf(&sqr, "SELECT * from gw_contacts WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY contactid ASC", sid->dat->uid, sid->dat->gid);
	}
*/
	if (sql_queryf(proc->N, &qobj1, "SELECT * FROM nullgs_entries WHERE did = 1 ORDER BY pid, id")<0) {
		xmlrpc_fault(sid, -1, lang_gets(sid, "common", "err_noaccess"));
		return;
	}
/*
	if (rc<0) {
		xmlrpc_fault(sid, -1, lang_gets(sid, "common", "err_noaccess"));
		return;
	}
*/
	prints(sid, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\r\n");
	prints(sid, "<dsml:dsml xmlns:dsml=\"http://www.dsml.org/DSML\">\r\n");
	prints(sid, "	<dsml:directory-entries>\r\n");
	for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
		pid=atoi(sql_getvalue(proc->N, &qobj1, i, 0));
//		if (sql_queryf(proc->N, &qobj2, "SELECT * FROM nullgs_attributes WHERE pid = %d ORDER BY pid, id", pid)<0) return;
		prints(sid, "		<dsml:entry dn=\"");
		printhex(sid, "%s", sql_getvaluebyname(proc->N, &qobj1, i, "name"));
		prints(sid, "\">\r\n");
//		prints(sid, "			<dsml:objectclass>\r\n");
//		prints(sid, "				<dsml:oc-value>%s</dsml:oc-value>\r\n", "top");
//		prints(sid, "				<dsml:oc-value>%s</dsml:oc-value>\r\n", "inetorgperson");
//		prints(sid, "			</dsml:objectclass>\r\n");
//		for (j=0;j<sql_numtuples(proc->N, &qobj2);j++) {
//			prints(sid, "			<dsml:attr name=\"%s\">\r\n", sql_getvalue(proc->N, &qobj2, j, 3));
//			prints(sid, "				<dsml:value>");
//			printhex(sid, "%s", sql_getvalue(proc->N, &qobj2, j, 4));
//			prints(sid, "</dsml:value>\r\n");
//			prints(sid, "			</dsml:attr>\r\n", sql_getvalue(proc->N, &qobj2, j, 3));
//		}
//		sql_freeresult(proc->N, &qobj2);
		prints(sid, "		</dsml:entry>\r\n");
		if (pid>100) break;
	}
	prints(sid, "	</dsml:directory-entries>\r\n");
	prints(sid, "</dsml:dsml>\r\n");
	sql_freeresult(proc->N, &qobj1);
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *cobj=nes_getobj(sid->N, htobj, "REQUEST_URI");
	char *RequestURI=cobj->val->d.str;
	char methodname[40];
	char *ptemp;
	int index;

/*
	if (auth_setcookie(sid)!=0) {
		send_header(sid, 0, 200, "1", "text/xml", -1, -1);
		xmlrpc_fault(sid, -1, "Authentication failure");
		return 0;
	}
*/
	if ((ptemp=getxmlenv(sid, "methodName"))==NULL) {
		if (strncmp(RequestURI, "/xml-rpc/contacts/read", 22)==0) {
			if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) {
				xmlrpc_fault(sid, -1, "Missing contactid");
			}
			index=atoi(ptemp);
			xmlrpc_contactread(sid, index);
		} else if (strncmp(RequestURI, "/xml-rpc/contacts/list", 22)==0) {
			xmlrpc_contactlist(sid);
		} else if (strncmp(RequestURI, "/xml-rpc/contacts/write", 23)==0) {
			xmlrpc_contactwrite(sid);
		} else if (strncmp(RequestURI, "/xml-rpc/testdump", 17)==0) {
			xmlrpc_domaindump(sid);
		} else if (strncmp(RequestURI, "/xml-rpc/dsmldump", 17)==0) {
			dsml_domaindump(sid);
		} else {
			send_header(sid, 0, 200, "1", "text/html", -1, -1);
			htpage_topmenu(sid, MENU_XMLRPC);
			prints(sid, "<BR />\r\n");
			prints(sid, "</BODY>\r\n</HTML>\r\n");
		}
		return 0;
	} else {
		memset(methodname, 0, sizeof(methodname));
		snprintf(methodname, sizeof(methodname)-1, "%s", ptemp);
		if (strncmp(methodname, "contacts.read", 13)==0) {
			if ((ptemp=getxmlparam(sid, 3, "int"))==NULL) {
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Missing contactid");
				return 0;
			}
			index=atoi(ptemp);
			xmlrpc_contactread(sid, index);
		} else if (strncmp(methodname, "contacts.login", 14)==0) {
			if ((index=xmlrpc_auth_login(sid))>0) {
				xmlrpc_contactread(sid, index);
			}
		} else if (strncmp(methodname, "contacts.list", 13)==0) {
			xmlrpc_contactlist(sid);
		} else if (strncmp(methodname, "contacts.write", 14)==0) {
			xmlrpc_contactwrite(sid);
		} else {
			send_header(sid, 0, 200, "1", "text/html", -1, -1);
			xmlrpc_fault(sid, -1, "Invalid methodName");
		}
	}
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *tobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	tobj=nes_settable(proc->N, nes_settable(proc->N, &proc->N->g, "GWMODULES"), "xmlrpc");
	nes_exec(proc->N, "GWMODULES.xmlrpc={fn_name='mod_main',fn_uri='/xml-rpc/'};");
	nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
