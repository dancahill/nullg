/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "http_mod.h"
#include "mod_xmlrpc.h"
#include <stdarg.h>

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

DllExport int mod_main(CONN *sid)
{
	char methodname[40];
	char *ptemp;
	int index;

	DEBUG_IN(sid, "xmlrpc_main()");
	if (auth_setcookie(sid)!=0) {
		send_header(sid, 0, 200, "1", "text/xml", -1, -1);
		xmlrpc_fault(sid, -1, "Authentication failure");
		return 0;
	}
	if ((ptemp=getxmlenv(sid, "methodName"))==NULL) {
		if (strncmp(sid->dat->in_RequestURI, "/xml-rpc/contacts/read", 22)==0) {
			if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) {
				xmlrpc_fault(sid, -1, "Missing contactid");
			}
			index=atoi(ptemp);
			xmlrpc_contactread(sid, index);
		} else if (strncmp(sid->dat->in_RequestURI, "/xml-rpc/contacts/list", 22)==0) {
			xmlrpc_contactlist(sid);
		} else if (strncmp(sid->dat->in_RequestURI, "/xml-rpc/contacts/write", 23)==0) {
			xmlrpc_contactwrite(sid);
		} else {
			send_header(sid, 0, 200, "1", "text/html", -1, -1);
			htpage_topmenu(sid, MENU_XMLRPC);
			prints(sid, "<BR>\r\n");
			prints(sid, "</BODY>\r\n</HTML>\r\n");
		}
		DEBUG_OUT(sid, "xmlrpc_main()");
		return 0;
	} else {
		memset(methodname, 0, sizeof(methodname));
		snprintf(methodname, sizeof(methodname)-1, "%s", ptemp);
		if (strncmp(methodname, "contacts.read", 13)==0) {
			if ((ptemp=getxmlparam(sid, 3, "int"))==NULL) {
				send_header(sid, 0, 200, "1", "text/html", -1, -1);
				xmlrpc_fault(sid, -1, "Missing contactid");
				DEBUG_OUT(sid, "xmlrpc_main()");
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
	DEBUG_OUT(sid, "xmlrpc_main()");
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_xmlrpc",		/* mod_name	*/
		0,			/* mod_submenu	*/
		"",			/* mod_menuname	*/
		"",			/* mod_menuuri	*/
		"",			/* mod_menuperm	*/
		"mod_main",		/* fn_name	*/
		"/xml-rpc/",		/* fn_uri	*/
		mod_init,		/* fn_init	*/
		mod_main,		/* fn_main	*/
		mod_exit		/* fn_exit	*/
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	lang_read();
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
