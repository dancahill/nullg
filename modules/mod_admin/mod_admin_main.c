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

void admin_status(CONN *sid)
{
	int i;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", proc->config.colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Server Statistics</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Start Time </FONT></TD><TD BGCOLOR=%s>%s</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, time_unix2text(sid, proc->stats.starttime+time_tzoffset(sid, time(NULL))));
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Pages      </FONT></TD><TD BGCOLOR=%s>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.http_pages);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>Connections</FONT></TD><TD BGCOLOR=%s>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.http_conns);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>SQL Queries</FONT></TD><TD BGCOLOR=%s>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.sql_queries);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>SQL Updates</FONT></TD><TD BGCOLOR=%s>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.sql_updates);
	prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>SQL Handles</FONT></TD><TD BGCOLOR=%s>%d</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, config->colour_fieldval, proc->stats.sql_handlecount);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<BR>\r\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", proc->config.colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=6><FONT COLOR=%s>Loaded Modules</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	for (i=0;;i++) {
		if ((proc->mod_menuitems[i].fn_name==NULL)||(proc->mod_menuitems[i].fn_ptr==NULL)) break;
		prints(sid, "<TR><TD BGCOLOR=%s><FONT COLOR=%s>%s&nbsp;</FONT></TD>\r\n", config->colour_fieldname, config->colour_fieldvaltext, proc->mod_menuitems[i].mod_name);
		prints(sid, "<TD BGCOLOR=%s>%s&nbsp;</TD>\r\n", config->colour_fieldval, proc->mod_menuitems[i].mod_menuname);
		prints(sid, "<TD BGCOLOR=%s>%s&nbsp;</TD>\r\n", config->colour_fieldval, proc->mod_menuitems[i].mod_menuuri);
		prints(sid, "<TD BGCOLOR=%s>%s()&nbsp;</TD>\r\n", config->colour_fieldval, proc->mod_menuitems[i].fn_name);
		prints(sid, "<TD BGCOLOR=%s>%s&nbsp;</TD>\r\n", config->colour_fieldval, proc->mod_menuitems[i].fn_uri);
		prints(sid, "<TD BGCOLOR=%s>0x%08X&nbsp;</TD></TR>\r\n", config->colour_fieldval, proc->mod_menuitems[i].fn_ptr);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<BR>\r\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", proc->config.colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3><FONT COLOR=%s>Exported Module Functions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	for (i=0;;i++) {
		if ((proc->mod_functions[i].fn_name==NULL)||(proc->mod_functions[i].fn_ptr==NULL)) break;
		prints(sid, "<TR BGCOLOR=%s><TD><FONT COLOR=%s>%s&nbsp;</FONT></TD><TD BGCOLOR=%s>%s()&nbsp;</TD><TD BGCOLOR=%s>0x%08X&nbsp;</TD></TR>\r\n", config->colour_fieldname, config->colour_fieldvaltext, proc->mod_functions[i].mod_name, config->colour_fieldval, proc->mod_functions[i].fn_name, config->colour_fieldval, proc->mod_functions[i].fn_ptr);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</BODY>\r\n</HTML>\r\n");
	return;
}

void adminzoneedit(CONN *sid)
{
	REC_ZONE zone;
	int zoneid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/zoneeditnew")==0) {
		zoneid=0;
		dbread_zone(sid, 2, 0, &zone);
	} else {
		if (getgetenv(sid, "ZONEID")==NULL) return;
		zoneid=atoi(getgetenv(sid, "ZONEID"));
		if (dbread_zone(sid, 2, zoneid, &zone)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", zoneid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/zonesave NAME=zoneedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=zoneid VALUE='%d'>\n", zone.zoneid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (zoneid!=0) {
		prints(sid, "zone %d</FONT></TH></TR>\n", zoneid);
	} else {
		prints(sid, "New Zone</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Zone Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=zonename   value=\"%s\" SIZE=25></TD></TR>\n", config->colour_editform, str2html(sid, zone.zonename));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(zoneid>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", config->colour_editform);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.zoneedit.zonename.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void adminzonelist(CONN *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_query(sid, "SELECT zoneid, zonename FROM gw_zones ORDER BY zonename ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", proc->config.colour_tabletrim);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP WIDTH=150><FONT COLOR=%s>&nbsp;Zone Name&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/zoneedit?zoneid=%d'\">", config->colour_fieldval, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/zoneedit?zoneid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no zones<BR>\n");
	}
	sql_freeresult(sqr);
	prints(sid, "<A HREF=%s/admin/zoneeditnew>New Zone</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void adminzonesave(CONN *sid)
{
	REC_ZONE zone;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int zoneid;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ZONEID"))==NULL) return;
	zoneid=atoi(ptemp);
	if (dbread_zone(sid, 2, zoneid, &zone)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "ZONENAME"))!=NULL) snprintf(zone.zonename, sizeof(zone.zonename)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_zones WHERE zoneid = %d", zone.zoneid)<0) return;
		prints(sid, "<CENTER>Zone %d deleted successfully</CENTER><BR>\n", zone.zoneid);
		db_log_activity(sid, 1, "zones", zone.zoneid, "delete", "%s - %s deleted zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	} else if (zone.zoneid==0) {
		if (strlen(zone.zonename)<1) {
			prints(sid, "<CENTER>Zone name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sql_query(sid, "SELECT max(zoneid) FROM gw_zones"))<0) return;
		zone.zoneid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (zone.zoneid<1) zone.zoneid=1;
		strcpy(query, "INSERT INTO gw_zones (zoneid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, zonename) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", zone.zoneid, curdate, curdate, zone.obj_uid, zone.obj_gid, zone.obj_gperm, zone.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, zone.zonename));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Zone %d added successfully</CENTER><BR>\n", zone.zoneid);
		db_log_activity(sid, 1, "zones", zone.zoneid, "insert", "%s - %s added zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_zones SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, zone.obj_uid, zone.obj_gid, zone.obj_gperm, zone.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "zonename = '%s'", str2sql(sid, zone.zonename));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE zoneid = %d", zone.zoneid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Zone %d modified successfully</CENTER><BR>\n", zone.zoneid);
		db_log_activity(sid, 1, "zones", zone.zoneid, "modify", "%s - %s modified zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/zonelist\">\n", sid->dat->in_ScriptName);
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
	if (strcmp(sid->dat->in_RequestURI, "/admin/")==0)
		admin_status(sid);
	else if (strncmp(sid->dat->in_RequestURI, "/admin/access", 13)==0)
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
	htpage_footer(sid);
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_admin", "ADMINISTRATION", "/admin/", "mod_main", "/admin/", mod_main)!=0) return -1;
	return 0;
}
