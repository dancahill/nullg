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
#include "http_mod.h"
#include "mod_admin.h"

void admingroupmemberedit(CONN *sid)
{
	REC_GROUPMEMBER groupmember;
	char *ptemp;
	int groupmemberid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/admin/groupmembereditnew", 25)==0) {
		groupmemberid=0;
		if (dbread_groupmember(sid, 2, 0, &groupmember)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((ptemp=getgetenv(sid, "GROUPID"))==NULL) return;
		groupmember.groupid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "GROUPMEMBERID"))==NULL) return;
		groupmemberid=atoi(ptemp);
		if (dbread_groupmember(sid, 2, groupmemberid, &groupmember)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", groupmemberid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/admin/groupmembersave NAME=memberedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupmemberid VALUE='%d'>\n", groupmember.groupmemberid);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", groupmember.groupid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints(sid, "<TR><TH COLSPAN=2><A HREF=%s/admin/groupedit?groupid=%d>Group %d</A> - ", sid->dat->in_ScriptName, groupmember.groupid, groupmember.groupid);
	if (groupmemberid>0) {
		prints(sid, " Group Member %d</TH></TR>\n", groupmemberid);
	} else {
		prints(sid, " New Group Member</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group&nbsp; </B></TD><TD ALIGN=RIGHT><SELECT NAME=groupid style='width:217px' DISABLED>");
	htselect_group(sid, A_ADMIN, groupmember.groupid, groupmember.obj_did);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;User&nbsp;  </B></TD><TD ALIGN=RIGHT><SELECT NAME=userid style='width:217px'>");
	htselect_user(sid, groupmember.userid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if (groupmember.groupmemberid>0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.memberedit.groupid.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void admingroupmemberlist(CONN *sid, int domainid, int groupid)
{
	int i;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (!auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (domainid!=sid->dat->user_did) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\r\n");
	if ((sqr=sql_queryf("SELECT userid, username FROM gw_users WHERE groupid = %d AND domainid = %d ORDER BY username ASC", groupid, domainid))<0) return;
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<TR><TD>&nbsp;</TD>");
		prints(sid, "<TD COLSPAN=2 WIDTH=100%% NOWRAP style='cursor:hand' onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	sql_freeresult(sqr);
	if ((sqr=sql_queryf("SELECT gw_groupmembers.groupmemberid, gw_users.userid, gw_users.username FROM gw_users, gw_groupmembers WHERE gw_users.userid = gw_groupmembers.userid AND gw_groupmembers.groupid = %d AND gw_users.domainid = %d ORDER BY username ASC", groupid, domainid))<0) return;
	for (i=0;i<sql_numtuples(sqr);i++) {
		prints(sid, "<TR><TD><A HREF=%s/admin/groupmemberedit?groupmemberid=%d>edit</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
		prints(sid, "<TD WIDTH=100%% NOWRAP style='cursor:hand' onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 1)));
		prints(sid, "+<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 1)));
		prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(sqr, i, 2)));
	}
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\r\n");
	return;
}

void admingroupmembersave(CONN *sid)
{
	REC_GROUPMEMBER groupmember;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int groupmemberid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPMEMBERID"))==NULL) return;
	groupmemberid=atoi(ptemp);
	if (dbread_groupmember(sid, 2, groupmemberid, &groupmember)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getpostenv(sid, "GROUPID"))!=NULL) groupmember.groupid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "USERID"))!=NULL) groupmember.userid=atoi(ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_groupmembers WHERE groupmemberid = %d", groupmember.groupmemberid)<0) return;
		prints(sid, "<CENTER>Group Member %d deleted successfully</CENTER><BR>\n", groupmember.groupmemberid);
		db_log_activity(sid, 1, "groupmembers", groupmember.groupmemberid, "delete", "%s - %s deleted groupmember %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupmember.groupmemberid);
	} else if (groupmember.groupmemberid==0) {
		snprintf(query, sizeof(query)-1, "INSERT INTO gw_groupmembers (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupid, userid) values ("
			"'%s', '%s', '0', '0', '%d', '0', '0', '%d', '%d')", curdate, curdate, groupmember.obj_did, groupmember.groupid, groupmember.userid);
		if (sql_update(query)<0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Group Member %d added successfully</CENTER><BR>\n", groupmember.groupmemberid);
		db_log_activity(sid, 1, "groupmembers", groupmember.groupmemberid, "insert", "%s - %s added groupmember %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupmember.groupmemberid);
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_groupmembers SET obj_mtime = '%s', groupid = '%d', userid = '%d' WHERE groupmemberid = %d",
			curdate, groupmember.groupid, groupmember.userid, groupmember.groupmemberid);
		if (sql_update(query)<0) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Group Member %d modified successfully</CENTER><BR>\n", groupmember.groupmemberid);
		db_log_activity(sid, 1, "groupmembers", groupmember.groupmemberid, "modify", "%s - %s modified groupmember %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupmember.groupmemberid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/groupedit?groupid=%d\");\n", sid->dat->in_ScriptName, groupmember.groupid);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/groupedit?groupid=%d\">\n", sid->dat->in_ScriptName, groupmember.groupid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}

void admingroupedit(CONN *sid)
{
	REC_GROUP group;
	char *ptemp;
	int groupid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/groupeditnew")==0) {
		groupid=0;
		dbread_group(sid, 2, 0, &group);
	} else {
		if ((ptemp=getgetenv(sid, "GROUPID"))==NULL) return;
		groupid=atoi(ptemp);
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
	if (groupid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Group %d</TH></TR>\n", group.groupid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Group</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Group Name</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=groupname   value=\"%s\" SIZE=50></TD></TR>\n", str2html(sid, group.groupname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>Message of the Day</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=motd ROWS=6 COLS=60>%s</TEXTAREA></TD></TR>\n", str2html(sid, group.motd));
	if (groupid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Group Members [<A HREF=%s/admin/groupmembereditnew?groupid=%d>new</A>]</TH></TR>\r\n", sid->dat->in_ScriptName, group.groupid);
		prints(sid, "<TR><TD COLSPAN=2 CLASS=\"FIELDVAL\" WIDTH=100%%>\n");
		admingroupmemberlist(sid, group.obj_did, group.groupid);
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(groupid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
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
	int i, j;
	int lastdomain;
	int thisdomain;
	int sqr1;
	int sqr2;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if ((sqr1=sql_queryf("SELECT groupid, obj_did, groupname FROM gw_groups ORDER BY obj_did, groupname ASC"))<0) return;
	} else {
		if ((sqr1=sql_queryf("SELECT groupid, obj_did, groupname FROM gw_groups WHERE obj_did = %d ORDER BY obj_did, groupname ASC", sid->dat->user_did))<0) return;
	}
	if ((sqr2=sql_query("SELECT domainid, domainname FROM gw_domains ORDER BY domainid ASC"))<0) {
		sql_freeresult(sqr1);
		return;
	}
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr1)<1) {
		prints(sid, "There are no groups<BR>\n");
		prints(sid, "<A HREF=%s/admin/groupeditnew>New Group</A>\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\n");
		sql_freeresult(sqr2);
		sql_freeresult(sqr1);
		return;
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=200 STYLE='border-style:solid'>&nbsp;Group Name&nbsp;</TH></TR>\n");
	lastdomain=-1;
	if (sql_numtuples(sqr2)<2) {
		lastdomain=atoi(sql_getvalue(sqr1, 0, 1));
	}
	for (i=0;i<sql_numtuples(sqr1);i++) {
		thisdomain=atoi(sql_getvalue(sqr1, i, 1));
		if (lastdomain!=thisdomain) {
			lastdomain=thisdomain;
			prints(sid, "<TR CLASS=\"FIELDNAME\"><TD COLSPAN=4 NOWRAP style='border-style:solid'>");
			for (j=0;j<sql_numtuples(sqr2);j++) {
				if (atoi(sql_getvalue(sqr2, j, 0))==atoi(sql_getvalue(sqr1, i, 1))) {
					prints(sid, "<B>%s</B></TD>", str2html(sid, sql_getvalue(sqr2, j, 1)));
					break;
				}
			}
			if (j==sql_numtuples(sqr2)) {
				prints(sid, "&nbsp;</TD>");
			}
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/groupedit?groupid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/admin/groupedit?groupid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD></TR>\n", str2html(sid, sql_getvalue(sqr1, i, 2)));
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<A HREF=%s/admin/groupeditnew>New Group</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr2);
	sql_freeresult(sqr1);
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
		if (sql_updatef("DELETE FROM gw_groups WHERE groupid = %d", group.groupid)<0) return;
		prints(sid, "<CENTER>Group %d deleted successfully</CENTER><BR>\n", group.groupid);
		db_log_activity(sid, 1, "groups", group.groupid, "delete", "%s - %s deleted group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	} else if (group.groupid==0) {
		if ((sqr=sql_queryf("SELECT groupname FROM gw_groups where groupname = '%s' AND obj_did = %d", group.groupname, group.obj_did))<0) return;
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
		if ((sqr=sql_query("SELECT max(groupid) FROM gw_groups"))<0) return;
		group.groupid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (group.groupid<1) group.groupid=1;
		strcpy(query, "INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd, members) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", group.groupid, curdate, curdate, group.obj_uid, group.obj_gid, group.obj_did, group.obj_gperm, group.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.groupname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.availability));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.motd));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.members));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Group %d added successfully</CENTER><BR>\n", group.groupid);
		db_log_activity(sid, 1, "groups", group.groupid, "insert", "%s - %s added group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_queryf("SELECT groupname FROM gw_groups where groupname = '%s' AND groupid <> %d AND obj_did = %d", group.groupname, group.groupid, group.obj_did))<0) return;
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
		snprintf(query, sizeof(query)-1, "UPDATE gw_groups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, group.obj_uid, group.obj_gid, group.obj_gperm, group.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "groupname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.groupname));
		strncatf(query, sizeof(query)-strlen(query)-1, "motd = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.motd));
		strncatf(query, sizeof(query)-strlen(query)-1, "members = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.members));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE groupid = %d AND obj_did = %d", group.groupid, group.obj_did);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Group %d modified successfully</CENTER><BR>\n", group.groupid);
		db_log_activity(sid, 1, "groups", group.groupid, "modify", "%s - %s modified group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, group.groupid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/grouplist\">\n", sid->dat->in_ScriptName);
	return;
}

void admingrouptimeedit(CONN *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char *ptemp;
	char availability[170];
	REC_GROUP group;
	int groupid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
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
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/grouptimesave NAME=availability>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", groupid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Group availability for <A HREF=%s/admin/groupedit?groupid=%d>%s</A></TH></TR>\n", sid->dat->in_ScriptName, groupid, group.groupname);
	prints(sid, "<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2 STYLE='border-style:solid'>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR CLASS=\"FIELDNAME\">\n");
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER STYLE='border-style:solid'><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP CLASS=\"FIELDNAME\" STYLE='border-style:solid'><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", i, dow[i]);
		for (j=0;j<24;j++) {
			prints(sid, "<TD STYLE='border-style:solid'><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, availability[i*24+j]=='1'?"checked":"");
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
	if (sql_updatef("UPDATE gw_groups SET obj_mtime = '%s', availability = '%s' WHERE groupid = %d", curdate, availability, groupid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	db_log_activity(sid, 1, "groups", groupid, "modify", "%s - %s modified availability for group %d", sid->dat->in_RemoteAddr, sid->dat->user_username, groupid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/groupedit?groupid=%d\");\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/groupedit?groupid=%d\">\n", sid->dat->in_ScriptName, groupid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}
