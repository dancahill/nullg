/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#include "mod_stub.h"
#include "mod_admin.h"

void admin_stats(CONN *sid)
{
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2 STYLE='border-style:solid'><FONT COLOR=%s>Server Statistics</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>Version         </FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%s</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, SERVER_VERSION);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>Start Time      </FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%s</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, time_unix2text(sid, proc->stats.starttime+time_tzoffset(sid, time(NULL))));
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>HTTP Pages      </FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.http_pages);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>HTTP Connections</FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.http_conns);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>SQL Queries     </FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.sql_queries);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>SQL Updates     </FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.sql_updates);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>SQL Handles     </FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.sql_handlecount);
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
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=7 STYLE='border-style:solid'><FONT COLOR=%s>Loaded Modules</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>MODULE</FONT></TD>", config->colour_fieldname, config->colour_fieldvaltext);
	prints(sid, "<TD STYLE='border-style:solid'><FONT COLOR=%s>MENU NAME</TD>", config->colour_fieldvaltext);
	prints(sid, "<TD STYLE='border-style:solid'><FONT COLOR=%s>MENU URI</TD>", config->colour_fieldvaltext);
	prints(sid, "<TD STYLE='border-style:solid'><FONT COLOR=%s>MENU PERM</TD>", config->colour_fieldvaltext);
	prints(sid, "<TD STYLE='border-style:solid'><FONT COLOR=%s>FN NAME</TD>", config->colour_fieldvaltext);
	prints(sid, "<TD STYLE='border-style:solid'><FONT COLOR=%s>FN URI</TD>", config->colour_fieldvaltext);
	prints(sid, "<TD STYLE='border-style:solid'><FONT COLOR=%s>FN PTR</TD></TR>\r\n", config->colour_fieldvaltext);
	for (i=0;;i++) {
		if ((proc->mod_menuitems[i].fn_name==NULL)||(proc->mod_menuitems[i].fn_ptr==NULL)) break;
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'>%s&nbsp;</TD>", config->colour_fieldval, proc->mod_menuitems[i].mod_name);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", proc->mod_menuitems[i].mod_menuname);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", proc->mod_menuitems[i].mod_menuuri);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", proc->mod_menuitems[i].mod_menuperm);
		prints(sid, "<TD STYLE='border-style:solid'>%s()&nbsp;</TD>", proc->mod_menuitems[i].fn_name);
		prints(sid, "<TD STYLE='border-style:solid'>%s&nbsp;</TD>", proc->mod_menuitems[i].fn_uri);
		prints(sid, "<TD STYLE='border-style:solid'>0x%08X&nbsp;</TD></TR>\r\n", proc->mod_menuitems[i].fn_ptr);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<BR>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3 STYLE='border-style:solid'><FONT COLOR=%s>Exported Module Functions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	for (i=0;;i++) {
		if ((proc->mod_functions[i].fn_name==NULL)||(proc->mod_functions[i].fn_ptr==NULL)) break;
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='border-style:solid'><FONT COLOR=%s>%s&nbsp;</FONT></TD><TD BGCOLOR=%s STYLE='border-style:solid'>%s()&nbsp;</TD><TD BGCOLOR=%s STYLE='border-style:solid'>0x%08X&nbsp;</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, proc->mod_functions[i].mod_name, config->colour_fieldval, proc->mod_functions[i].fn_name, config->colour_fieldval, proc->mod_functions[i].fn_ptr);
	}
	prints(sid, "</TABLE>\r\n");
	return;
}

void mod_main(CONN *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_ADMIN);
	if ((strncmp(sid->dat->in_RequestURI, "/admin/activity", 15)!=0)&&(strncmp(sid->dat->in_RequestURI, "/admin/access", 13)!=0)&&(strncmp(sid->dat->in_RequestURI, "/admin/error", 12)!=0)) {
		prints(sid, "<BR>\r\n");
	}
	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
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
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupedit", 16)==0)
		admingroupedit(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/grouplist", 16)==0)
		admingrouplist(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/groupsave", 16)==0)
		admingroupsave(sid);
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
		prints(sid, "</CENTER>\n");
	}
	htpage_footer(sid);
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_admin",		// mod_name
		3,			// mod_submenu
		"ADMINISTRATION",	// mod_menuname
		"/admin/",		// mod_menuuri
		"admin",		// mod_menuperm
		"mod_main",		// fn_name
		"/admin/",		// fn_uri
		mod_main		// fn_ptr
	};

	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
