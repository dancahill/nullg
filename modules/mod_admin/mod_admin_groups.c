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
#include "mod_substub.h"
#include "mod_admin.h"

void admingroupedit(CONN *sid)
{
	REC_GROUP group;
	int groupid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/groupeditnew")==0) {
		groupid=0;
		dbread_group(sid, 2, 0, &group);
	} else {
		if (getgetenv(sid, "GROUPID")==NULL) return;
		groupid=atoi(getgetenv(sid, "GROUPID"));
		if (dbread_group(sid, 2, groupid, &group)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", groupid);
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
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/groupsave NAME=groupedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", group.groupid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	if (groupid!=0) {
		prints(sid, "Group %d</FONT></TH></TR>\n", groupid);
	} else {
		prints(sid, "New Group</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>Group Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=groupname   value=\"%s\" SIZE=50></TD></TR>\n", config->colour_editform, str2html(sid, group.groupname));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2><B>Message of the Day</B></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=motd ROWS=6 COLS=60>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, group.motd));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(groupid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", config->colour_editform);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if (groupid!=0) {
		prints(sid, "[<A HREF=%s/admin/grouptimeedit?groupid=%d>Edit Availability</A>]\n", sid->dat->in_ScriptName, groupid);
	}
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.groupedit.groupname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void admingrouplist(CONN *sid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr=sql_query(sid, "SELECT groupid, groupname FROM gw_groups ORDER BY groupid ASC"))<0) return;
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1>\r\n", config->colour_tabletrim);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP WIDTH=150><FONT COLOR=%s>&nbsp;Group Name&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/groupedit?groupid=%d'\">", config->colour_fieldval, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/groupedit?groupid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
		prints(sid, "</TABLE>\n");
	} else {
		prints(sid, "There are no groups<BR>\n");
	}
	sql_freeresult(sqr);
	prints(sid, "<A HREF=%s/admin/groupeditnew>New Group</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void admingroupsave(CONN *sid)
{
	REC_GROUP group;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int groupid;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
	if (dbread_group(sid, 2, groupid, &group)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "GROUPNAME"))!=NULL) snprintf(group.groupname, sizeof(group.groupname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MOTD"))!=NULL) snprintf(group.motd, sizeof(group.motd)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MEMBERS"))!=NULL) snprintf(group.members, sizeof(group.members)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_groups WHERE groupid = %d", group.groupid)<0) return;
		prints(sid, "<CENTER>Group %d deleted successfully</CENTER><BR>\n", group.groupid);
		db_log_activity(sid, 1, "groups", group.groupid, "delete", "%s - %s deleted group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	} else if (group.groupid==0) {
		if ((sqr=sql_queryf(sid, "SELECT groupname FROM gw_groups where groupname = '%s'", group.groupname))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER>Group %s already exists</CENTER><BR>\n", group.groupname);
			sql_freeresult(sqr);
			return;
		}
		sql_freeresult(sqr);
		if (strlen(group.groupname)<1) {
			prints(sid, "<CENTER>Group name is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sql_query(sid, "SELECT max(groupid) FROM gw_groups"))<0) return;
		group.groupid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (group.groupid<1) group.groupid=1;
		strcpy(query, "INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, groupname, availability, motd, members) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", group.groupid, curdate, curdate, group.obj_uid, group.obj_gid, group.obj_gperm, group.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, group.groupname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, group.availability));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, group.motd));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, group.members));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Group %d added successfully</CENTER><BR>\n", group.groupid);
		db_log_activity(sid, 1, "groups", group.groupid, "insert", "%s - %s added group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_groups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, group.obj_uid, group.obj_gid, group.obj_gperm, group.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "groupname = '%s', ", str2sql(sid, group.groupname));
		strncatf(query, sizeof(query)-strlen(query)-1, "motd = '%s', ", str2sql(sid, group.motd));
		strncatf(query, sizeof(query)-strlen(query)-1, "members = '%s'", str2sql(sid, group.members));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE groupid = %d", group.groupid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>Group %d modified successfully</CENTER><BR>\n", group.groupid);
		db_log_activity(sid, 1, "groups", group.groupid, "modify", "%s - %s modified group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/grouplist\">\n", sid->dat->in_ScriptName);
	return;
}

void admingrouptimeedit(CONN *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char availability[170];
	REC_GROUP group;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "GROUPID")==NULL) return;
	groupid=atoi(getgetenv(sid, "GROUPID"));
	if (dbread_group(sid, 2, groupid, &group)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", groupid);
		return;
	}
	memset(availability, 0, sizeof(availability));
	strncpy(availability, group.availability, sizeof(availability)-1);
	if (strlen(availability)!=168) {
		for (i=0;i<168;i++) {
			availability[i]='0';
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function toggle(b,n)\n");
	prints(sid, "{\n");
	prints(sid, "	if (b == \"t\" ) {\n");
	prints(sid, "		if (document.availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (document.availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				document.availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (document.availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (document.availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				document.availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=0 CELLSPACING=1>\r\n", config->colour_tabletrim);
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/grouptimesave NAME=availability>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", groupid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=25><FONT COLOR=%s>Group availability for <A HREF=%s/admin/groupedit?groupid=%d STYLE='color: %s'>%s</A></FONT></TH></TR>\n", config->colour_th, config->colour_thtext, sid->dat->in_ScriptName, groupid, config->colour_thlink, group.groupname);
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldname);
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldname);
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldval);
		prints(sid, "<TD ALIGN=LEFT NOWRAP BGCOLOR=%s><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", config->colour_fieldname, i, dow[i]);
		for (j=0;j<24;j++) {
			prints(sid, "<TD><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, availability[i*24+j]=='1'?"checked":"");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void admingrouptimesave(CONN *sid)
{
	char availability[170];
	char curdate[40];
	char qvar[10];
	time_t t;
	char *ptemp;
	char *pdest;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
	memset(availability, 0, sizeof(availability));
	pdest=availability;
	for (i=0;i<7;i++) {
		for (j=0;j<24;j++) {
			memset(qvar, 0, sizeof(qvar));
			snprintf(qvar, sizeof(qvar)-1, "D%dT%d", i, j);
			if ((ptemp=getpostenv(sid, qvar))==NULL) {
				*pdest++='0';
				continue;
			}
			if (strcasecmp(ptemp, "false")==0) {
				*pdest++='1';
			} else {
				*pdest++='1';
			}
		}
	}
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (sql_updatef(sid, "UPDATE gw_groups SET obj_mtime = '%s', availability = '%s' WHERE groupid = %d", curdate, availability, groupid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	db_log_activity(sid, 1, "groups", groupid, "modify", "%s - %s modified availability for group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/groupedit?groupid=%d\");\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/groupedit?groupid=%d\">\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}
