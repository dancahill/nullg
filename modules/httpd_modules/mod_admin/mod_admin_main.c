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
#include "mod_admin.h"

void admin_stats(CONN *sid)
{
	char *ptemp;

	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Server Statistics</TH></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>Version         </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%s</TD></TR>\r\n", PACKAGE_VERSION);
	if (http_proc->RunAsCGI) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>Host            </TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>");
		if ((ptemp=getenv("SERVER_SOFTWARE"))!=NULL) {
			prints(sid, "%s</TD></TR>\r\n", ptemp);
		} else {
			prints(sid, "unknown</TD></TR>\r\n");
		}
	}
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
	int i;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	admin_stats(sid);
	prints(sid, "<BR>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=7 STYLE='border-style:solid'>Loaded Modules</TH></TR>\n");
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MODULE</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MENU NAME</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MENU URI</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>MENU PERM</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>FN NAME</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>FN URI</TD>");
	prints(sid, "<TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>FN PTR</TD></TR>\r\n");
	for (i=0;;i++) {
		if ((http_proc->mod_menuitems[i].fn_name==NULL)||(http_proc->mod_menuitems[i].fn_main==NULL)) break;
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD STYLE='border-style:solid'>%s&nbsp;</TD>", http_proc->mod_menuitems[i].mod_name);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", http_proc->mod_menuitems[i].mod_menuname);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", http_proc->mod_menuitems[i].mod_menuuri);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", http_proc->mod_menuitems[i].mod_menuperm);
		prints(sid, "<TD STYLE='border-style:solid'>%s()&nbsp;</TD>", http_proc->mod_menuitems[i].fn_name);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", http_proc->mod_menuitems[i].fn_uri);
		prints(sid, "<TD STYLE='border-style:solid'>0x%08X&nbsp;</TD></TR>\r\n", http_proc->mod_menuitems[i].fn_main);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<BR>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=3 STYLE='border-style:solid'>Exported Module Functions</TH></TR>\n");
	for (i=0;;i++) {
		if ((http_proc->mod_functions[i].fn_name==NULL)||(http_proc->mod_functions[i].fn_ptr==NULL)) break;
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" STYLE='border-style:solid'>%s&nbsp;</TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>%s()&nbsp;</TD><TD CLASS=\"FIELDVAL\" STYLE='border-style:solid'>0x%08X&nbsp;</TD></TR>\r\n", http_proc->mod_functions[i].mod_name, http_proc->mod_functions[i].fn_name, http_proc->mod_functions[i].fn_ptr);
	}
	prints(sid, "</TABLE>\r\n");
	return;
}

DllExport int mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ADMIN);
	if ((strncmp(sid->dat->in_RequestURI, "/admin/activity", 15)!=0)&&(strncmp(sid->dat->in_RequestURI, "/admin/access", 13)!=0)&&(strncmp(sid->dat->in_RequestURI, "/admin/error", 12)!=0)) {
		prints(sid, "<BR>\r\n");
	}
	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return 0;
	}
	if (strncmp(sid->dat->in_RequestURI, "/admin/access", 13)==0)
		adminaccess(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/activitylist", 19)==0)
		adminactivitylist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/activityview", 19)==0)
		adminactivityview(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/error", 12)==0)
		adminerror(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/configedit", 17)==0)
		adminconfigedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/configsave", 17)==0)
		adminconfigsave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/domainedit", 17)==0)
		admindomainedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/domainlist", 17)==0)
		admindomainlist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/domainsave", 17)==0)
		admindomainsave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupedit", 16)==0)
		admingroupedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouplist", 16)==0)
		admingrouplist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupsave", 16)==0)
		admingroupsave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupmemberedit", 22)==0)
		admingroupmemberedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupmembersave", 22)==0)
		admingroupmembersave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouptimeedit", 20)==0)
		admingrouptimeedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouptimesave", 20)==0)
		admingrouptimesave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/status", 13)==0)
		admin_status(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/syscheck", 15)==0)
		admin_syscheck(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/useredit", 15)==0)
		adminuseredit(sid, NULL);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/userlist", 15)==0)
		adminuserlist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/usersave", 15)==0)
		adminusersave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/usertimeedit", 19)==0)
		adminusertimeedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/usertimesave", 19)==0)
		adminusertimesave(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/zoneedit", 15)==0)
		adminzoneedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/zonelist", 15)==0)
		adminzonelist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/zonesave", 15)==0)
		adminzonesave(sid);
	else {
		prints(sid, "<CENTER>\n");
		admin_stats(sid);
		prints(sid, "<A HREF=%s/admin/status>Details</A>", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\n");
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_admin",		// mod_name
		3,			// mod_submenu
		"ADMINISTRATION",	// mod_menuname
		"/admin/",		// mod_menuuri
		"admin",		// mod_menuperm
		"mod_main",		// fn_name
		"/admin/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
