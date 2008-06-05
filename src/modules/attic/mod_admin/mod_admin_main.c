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
#include "mod_admin.h"

void admin_stats(CONN *sid)
{
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Server Statistics</TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>Version         </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%s</TD></TR>\r\n", PACKAGE_VERSION);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>Start Time      </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%s</TD></TR>\r\n", time_unix2text(sid, proc->stats.starttime+time_tzoffset(sid, time(NULL))));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>HTTP Pages      </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%d</TD></TR>\r\n", proc->stats.http_pages);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>HTTP Connections</TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%d</TD></TR>\r\n", proc->stats.http_conns);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>SQL Queries     </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%d</TD></TR>\r\n", proc->stats.sql_queries);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>SQL Updates     </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%d</TD></TR>\r\n", proc->stats.sql_updates);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>SQL Handles     </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%d</TD></TR>\r\n", proc->stats.sql_handlecount);
	prints(sid, "</TABLE>\r\n");
}

void admin_status(CONN *sid)
{
	obj_t *modobj=nes_getobj(proc->N, &proc->N->g, "GWMODULES");
	obj_t *tobj, *cobj;
//	int i;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	admin_stats(sid);
	prints(sid, "<BR />\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=7 STYLE='border-style:solid'>Loaded Modules</TH></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MODULE</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MENU NAME</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MENU URI</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MENU PERM</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>FN NAME</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>FN URI</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>FN PTR</TD></TR>\r\n");
	for (tobj=modobj->val->d.table;tobj;tobj=tobj->next) {
		if (tobj->val->type!=NT_TABLE) break;
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='border-style:solid'>%s&nbsp;</TD>", tobj->name);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", nes_getstr(proc->N, tobj, "name"));
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", nes_getstr(proc->N, tobj, "uri"));
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", nes_getstr(proc->N, tobj, "perm"));
		prints(sid, "<TD STYLE='border-style:solid'>%s()&nbsp;</TD>", nes_getstr(proc->N, tobj, "fn_name"));
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", nes_getstr(proc->N, tobj, "fn_uri"));
		cobj=nes_getobj(proc->N, tobj, "mod_main");
		if (cobj->val->type==NT_CFUNC) {
			prints(sid, "<TD STYLE='border-style:solid'>0x%08X&nbsp;</TD></TR>\r\n", (int)cobj->val->d.cfunc);
		}
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<BR />\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=3 STYLE='border-style:solid'>Exported Module Functions</TH></TR>\r\n");
//	for (i=0;;i++) {
//		if ((proc->mod_functions[i].fn_name==NULL)||(proc->mod_functions[i].fn_ptr==NULL)) break;
//		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>%s&nbsp;</TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%s()&nbsp;</TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>0x%08X&nbsp;</TD></TR>\r\n", proc->mod_functions[i].mod_name, proc->mod_functions[i].fn_name, proc->mod_functions[i].fn_ptr);
//	}
	prints(sid, "</TABLE>\r\n");
	return;
}

void admin_su(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	int userid;
	int domainid;
	char *ptemp;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (!(auth_priv(sid, "domainadmin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	userid=sid->dat->uid;
	domainid=sid->dat->did;
	if ((ptemp=getgetenv(sid, "UID"))!=NULL) userid=atoi(ptemp);
	if ((ptemp=getgetenv(sid, "DID"))!=NULL) domainid=atoi(ptemp);
	sql_updatef(proc->N, "UPDATE gw_users_sessions SET userid = %d, domainid = %d WHERE token = '%s'", userid, domainid, sid->dat->token);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=\"text/javascript\">\r\n<!--\r\nif (self!=parent) open('%s/','_top');\r\n// -->\r\n</SCRIPT>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
	db_log_activity(sid, "users", sid->dat->uid, "su", "%s - %s became user %d:%d", sid->socket.RemoteAddr, sid->dat->username, userid, domainid);
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");

	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, "admin");
	if ((strncmp(RequestURI, "/admin/activity", 15)!=0)&&(strncmp(RequestURI, "/admin/access", 13)!=0)&&(strncmp(RequestURI, "/admin/error", 12)!=0)) {
		prints(sid, "<BR />\r\n");
	}
	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return 0;
	}
	if (strncmp(RequestURI, "/admin/su", 9)==0)
		admin_su(sid);
	else if (strncmp(RequestURI, "/admin/access", 13)==0)
		adminaccess(sid);
	else if (strncmp(RequestURI, "/admin/activitylist", 19)==0)
		adminactivitylist(sid);
	else if (strncmp(RequestURI, "/admin/activityview", 19)==0)
		adminactivityview(sid);
	else if (strncmp(RequestURI, "/admin/error", 12)==0)
		adminerror(sid);
/*
	else if (strncmp(RequestURI, "/admin/configedit", 17)==0)
		adminconfigedit(sid);
	else if (strncmp(RequestURI, "/admin/configsave", 17)==0)
		adminconfigsave(sid);
*/
	else if (strncmp(RequestURI, "/admin/domainedit", 17)==0)
		admindomainedit(sid);
	else if (strncmp(RequestURI, "/admin/domainlist", 17)==0)
		admindomainlist(sid);
	else if (strncmp(RequestURI, "/admin/domainsave", 17)==0)
		admindomainsave(sid);
	else if (strncmp(RequestURI, "/admin/groupedit", 16)==0)
		admingroupedit(sid);
	else if (strncmp(RequestURI, "/admin/grouplist", 16)==0)
		admingrouplist(sid);
	else if (strncmp(RequestURI, "/admin/groupsave", 16)==0)
		admingroupsave(sid);
	else if (strncmp(RequestURI, "/admin/groupmemberedit", 22)==0)
		admingroupmemberedit(sid);
	else if (strncmp(RequestURI, "/admin/groupmembersave", 22)==0)
		admingroupmembersave(sid);
	else if (strncmp(RequestURI, "/admin/grouptimeedit", 20)==0)
		admingrouptimeedit(sid);
	else if (strncmp(RequestURI, "/admin/grouptimesave", 20)==0)
		admingrouptimesave(sid);
	else if (strncmp(RequestURI, "/admin/status", 13)==0)
		admin_status(sid);
	else if (strncmp(RequestURI, "/admin/syscheck", 15)==0)
		admin_syscheck(sid);
	else if (strncmp(RequestURI, "/admin/useredit", 15)==0)
		adminuseredit(sid, NULL);
	else if (strncmp(RequestURI, "/admin/userlist", 15)==0)
		adminuserlist(sid);
	else if (strncmp(RequestURI, "/admin/usersave", 15)==0)
		adminusersave(sid);
	else if (strncmp(RequestURI, "/admin/usertimeedit", 19)==0)
		adminusertimeedit(sid);
	else if (strncmp(RequestURI, "/admin/usertimesave", 19)==0)
		adminusertimesave(sid);
	else if (strncmp(RequestURI, "/admin/zoneedit", 15)==0)
		adminzoneedit(sid);
	else if (strncmp(RequestURI, "/admin/zonelist", 15)==0)
		adminzonelist(sid);
	else if (strncmp(RequestURI, "/admin/zonesave", 15)==0)
		adminzonesave(sid);
	else {
		prints(sid, "<CENTER>\r\n");
		admin_stats(sid);
		prints(sid, "<A HREF=%s/admin/status>Details</A>", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
		prints(sid, "</CENTER>\r\n");
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *modobj, *tobj, *cobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	modobj=nes_settable(proc->N, &proc->N->g, "GWMODULES");
	tobj=nes_settable(proc->N, modobj, "admin");
	cobj=nes_setnum(proc->N, tobj, "submenu", 3);
	cobj=nes_setstr(proc->N, tobj, "name",    "ADMINISTRATION",  strlen("ADMINISTRATION"));
	cobj=nes_setstr(proc->N, tobj, "pic",     "admin.png",       strlen("admin.png"));
	cobj=nes_setstr(proc->N, tobj, "uri",     "/admin/",         strlen("/admin/"));
	cobj=nes_setstr(proc->N, tobj, "perm",    "admin",           strlen("admin"));
	cobj=nes_setstr(proc->N, tobj, "fn_name", "mod_main",        strlen("mod_main"));
	cobj=nes_setstr(proc->N, tobj, "fn_uri",  "/admin/",         strlen("/admin/"));
	cobj=nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	cobj=nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	cobj=nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
	return 0;
}
