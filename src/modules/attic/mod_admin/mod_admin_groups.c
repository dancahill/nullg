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
#include "mod_admin.h"

static int dirmake(const char *format, ...)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	struct stat sb;
	char dirname[512];
	va_list ap;

	memset(dirname, 0, sizeof(dirname));
	va_start(ap, format);
	vsnprintf(dirname, sizeof(dirname)-1, format, ap);
	va_end(ap);
	fixslashes(dirname);
	if (stat(dirname, &sb)==0) return 0;
#ifdef WIN32
	if (mkdir(dirname)!=0) {
#else
	if (mkdir(dirname, ~(int)nes_getnum(proc->N, confobj, "umask")&0777)!=0) {
#endif
		return -1;
	}
	return 0;
}

void admingroupmemberedit(CONN *sid)
{
/*
	REC_GROUPMEMBER groupmember;
	char *ptemp;
	int groupmemberid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strncmp(sid->dat->in_RequestURI, "/admin/groupmembereditnew", 25)==0) {
		groupmemberid=0;
		if (dbread_groupmember(sid, 2, 0, &groupmember)!=0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if ((ptemp=getgetenv(sid, "GROUPID"))!=NULL) groupmember.groupid=atoi(ptemp);
		if ((ptemp=getgetenv(sid, "USERID"))!=NULL) groupmember.userid=atoi(ptemp);
	} else {
		if ((ptemp=getgetenv(sid, "GROUPMEMBERID"))==NULL) return;
		groupmemberid=atoi(ptemp);
		if (dbread_groupmember(sid, 2, groupmemberid, &groupmember)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", groupmemberid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n<FORM METHOD=POST ACTION=%s/admin/groupmembersave NAME=memberedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupmemberid VALUE='%d'>\r\n", groupmember.groupmemberid);
	if (groupmember.groupid) {
		prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\r\n", groupmember.groupid);
	} else if (groupmember.userid) {
		prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\r\n", groupmember.userid);
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n<TR><TH COLSPAN=2>");
	if (groupmember.groupid) {
		prints(sid, "<A HREF=%s/admin/groupedit?groupid=%d>Group %d</A> - ", sid->dat->in_ScriptName, groupmember.groupid, groupmember.groupid);
	} else if (groupmember.userid) {
		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>User %d</A> - ", sid->dat->in_ScriptName, groupmember.userid, groupmember.userid);
	}
	if (groupmemberid>0) {
		prints(sid, " Group Member %d</TH></TR>\r\n", groupmemberid);
	} else {
		prints(sid, " New Group Member</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group&nbsp; </B></TD><TD ALIGN=RIGHT><SELECT NAME=groupid style='width:217px'%s>", groupmember.groupid?" DISABLED":"");
	htselect_group(sid, A_ADMIN, groupmember.groupid, groupmember.obj_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;User&nbsp;  </B></TD><TD ALIGN=RIGHT><SELECT NAME=userid style='width:217px'%s>", groupmember.userid?" DISABLED":"");
	htselect_user(sid, groupmember.userid, groupmember.obj_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\r\n");
	if (groupmember.groupmemberid>0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "</FORM>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.memberedit.groupid.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void admingroupmemberlist(CONN *sid, int domainid, int groupid)
{
/*
	int i;
	SQLRES sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (!auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (domainid!=sid->dat->did) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\r\n");
//	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE groupid = %d AND domainid = %d ORDER BY username ASC", groupid, domainid)<0) return;
//	for (i=0;i<sql_numtuples(&sqr);i++) {
//		prints(sid, "<TR><TD>&nbsp;</TD>");
//		prints(sid, "<TD COLSPAN=2 WIDTH=100%% NOWRAP style='cursor:hand' onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
//		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
//		prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr, i, 1)));
//	}
//	sql_freeresult(&sqr);
	if (sql_queryf(&sqr, "SELECT gw_groups_members.groupmemberid, gw_users.userid, gw_users.username FROM gw_users, gw_groups_members WHERE gw_users.userid = gw_groups_members.userid AND gw_groups_members.groupid = %d AND gw_users.domainid = %d ORDER BY username ASC", groupid, domainid)<0) return;
	for (i=0;i<sql_numtuples(&sqr);i++) {
		prints(sid, "<TR><TD><A HREF=%s/admin/groupmemberedit?groupmemberid=%d>edit</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
		prints(sid, "<TD WIDTH=100%% NOWRAP style='cursor:hand' onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 1)));
		prints(sid, "+<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 1)));
		prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr, i, 2)));
	}
	sql_freeresult(&sqr);
	prints(sid, "</TABLE>\r\n");
*/
	return;
}

void admingroupmembersave(CONN *sid)
{
/*
	REC_GROUPMEMBER groupmember;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int groupmemberid;
	SQLRES sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPMEMBERID"))==NULL) return;
	groupmemberid=atoi(ptemp);
	if (dbread_groupmember(sid, 2, groupmemberid, &groupmember)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getpostenv(sid, "GROUPID"))!=NULL) groupmember.groupid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "USERID"))!=NULL) groupmember.userid=atoi(ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (sql_updatef("DELETE FROM gw_groups_members WHERE groupmemberid = %d", groupmember.groupmemberid)<0) return;
		prints(sid, "<CENTER>Group Member %d deleted successfully</CENTER><BR />\r\n", groupmember.groupmemberid);
		db_log_activity(sid, "groupmembers", groupmember.groupmemberid, "delete", "%s - %s deleted groupmember %d", sid->dat->in_RemoteAddr, sid->dat->username, groupmember.groupmemberid);
	} else if (groupmember.groupmemberid==0) {
		if (sql_queryf(&sqr, "SELECT groupmemberid FROM gw_groups_members where obj_did = %d AND userid = %d AND groupid = %d", groupmember.obj_did, groupmember.userid, groupmember.groupid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>User is already in group</CENTER><BR />\r\n");
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		snprintf(query, sizeof(query)-1, "INSERT INTO gw_groups_members (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupid, userid) values ("
			"'%s', '%s', '0', '0', '%d', '0', '0', '%d', '%d')", curdate, curdate, groupmember.obj_did, groupmember.groupid, groupmember.userid);
		if (sql_update(query)<0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		prints(sid, "<CENTER>Group Member %d added successfully</CENTER><BR />\r\n", groupmember.groupmemberid);
		db_log_activity(sid, "groupmembers", groupmember.groupmemberid, "insert", "%s - %s added groupmember %d", sid->dat->in_RemoteAddr, sid->dat->username, groupmember.groupmemberid);
	} else {
		snprintf(query, sizeof(query)-1, "UPDATE gw_groups_members SET obj_mtime = '%s', groupid = '%d', userid = '%d' WHERE groupmemberid = %d",
			curdate, groupmember.groupid, groupmember.userid, groupmember.groupmemberid);
		if (sql_update(query)<0) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		prints(sid, "<CENTER>Group Member %d modified successfully</CENTER><BR />\r\n", groupmember.groupmemberid);
		db_log_activity(sid, "groupmembers", groupmember.groupmemberid, "modify", "%s - %s modified groupmember %d", sid->dat->in_RemoteAddr, sid->dat->username, groupmember.groupmemberid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/admin/groupedit?groupid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName, groupmember.groupid);
	prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/groupedit?groupid=%d\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName, groupmember.groupid);
*/
	return;
}

void admingroupedit(CONN *sid)
{
/*
	REC_GROUP group;
	char *ptemp;
	int groupid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/groupeditnew")==0) {
		groupid=0;
		dbread_group(sid, 2, 0, &group);
	} else {
		if ((ptemp=getgetenv(sid, "GROUPID"))==NULL) return;
		groupid=atoi(ptemp);
		if (dbread_group(sid, 2, groupid, &group)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", groupid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/groupsave NAME=groupedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\r\n", group.groupid);
	if (groupid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Group %d</TH></TR>\r\n", group.groupid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Group</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Domain</B></TD><TD ALIGN=RIGHT><SELECT NAME=domainid style='width:255px'%s>\r\n", !(auth_priv(sid, "domainadmin")&A_ADMIN)||(group.groupid>0)?" DISABLED":"");
	htselect_domain(sid, group.obj_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Group Name</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=groupname value=\"%s\" SIZE=25 style='width:255px'></TD></TR>\r\n", str2html(sid, group.groupname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>Message of the Day</B></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=motd ROWS=6 COLS=60>%s</TEXTAREA></TD></TR>\r\n", str2html(sid, group.motd));
	if (groupid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Group Members [<A HREF=%s/admin/groupmembereditnew?groupid=%d>new</A>]</TH></TR>\r\n", sid->dat->in_ScriptName, group.groupid);
		prints(sid, "<TR><TD COLSPAN=2 CLASS=\"FIELDVAL\" WIDTH=100%%>\r\n");
		admingroupmemberlist(sid, group.obj_did, group.groupid);
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(groupid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	if (groupid!=0) {
		prints(sid, "[<A HREF=%s/admin/grouptimeedit?groupid=%d>Edit Availability</A>]\r\n", sid->dat->in_ScriptName, groupid);
	}
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.groupedit.groupname.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void admingrouplist(CONN *sid)
{
/*
	int i, j;
	int lastdomain;
	int thisdomain;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (sql_queryf(&sqr1, "SELECT groupid, obj_did, groupname FROM gw_groups ORDER BY obj_did, groupname ASC")<0) return;
	} else {
		if (sql_queryf(&sqr1, "SELECT groupid, obj_did, groupname FROM gw_groups WHERE obj_did = %d ORDER BY obj_did, groupname ASC", sid->dat->did)<0) return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (sql_queryf(&sqr2, "SELECT domainid, domainname FROM gw_domains ORDER BY domainname ASC")<0) {
			sql_freeresult(&sqr1);
			return;
		}
	} else {
		if (sql_queryf(&sqr2, "SELECT domainid, domainname FROM gw_domains WHERE domainid = %d", sid->dat->did)<0) {
			sql_freeresult(&sqr1);
			return;
		}
	}
	prints(sid, "<CENTER>\r\n");
	if (sql_numtuples(&sqr1)<1) {
		prints(sid, "There are no groups<BR />\r\n");
		prints(sid, "<A HREF=%s/admin/groupeditnew>New Group</A>\r\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\r\n");
		sql_freeresult(&sqr2);
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=200 STYLE='border-style:solid'>&nbsp;Group Name&nbsp;</TH></TR>\r\n");
	lastdomain=-1;
	if (sql_numtuples(&sqr2)<2) {
		lastdomain=atoi(sql_getvalue(&sqr1, 0, 1));
	}
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		thisdomain=atoi(sql_getvalue(&sqr1, i, 1));
		if (lastdomain!=thisdomain) {
			lastdomain=thisdomain;
			prints(sid, "<TR CLASS=\"FIELDNAME\"><TD COLSPAN=4 NOWRAP style='border-style:solid'>");
			for (j=0;j<sql_numtuples(&sqr2);j++) {
				if (atoi(sql_getvalue(&sqr2, j, 0))==atoi(sql_getvalue(&sqr1, i, 1))) {
					prints(sid, "<B>%s</B></TD>", str2html(sid, sql_getvalue(&sqr2, j, 1)));
					break;
				}
			}
			if (j==sql_numtuples(&sqr2)) {
				prints(sid, "&nbsp;</TD>");
			}
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/groupedit?groupid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "<A HREF=%s/admin/groupedit?groupid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr1, i, 2)));
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<A HREF=%s/admin/groupeditnew>New Group</A>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr2);
	sql_freeresult(&sqr1);
*/
	return;
}

void admingroupsave(CONN *sid)
{
/*
	REC_GROUP group;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int groupid;
	SQLRES sqr;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "GROUPID"))==NULL) return;
	groupid=atoi(ptemp);
	if (dbread_group(sid, 2, groupid, &group)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "DOMAINID"))!=NULL) group.obj_did=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "GROUPNAME"))!=NULL) snprintf(group.groupname, sizeof(group.groupname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MOTD"))!=NULL) snprintf(group.motd, sizeof(group.motd)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_groups WHERE groupid = %d", group.groupid)<0) return;
		prints(sid, "<CENTER>Group %d deleted successfully</CENTER><BR />\r\n", group.groupid);
		db_log_activity(sid, "groups", group.groupid, "delete", "%s - %s deleted group %d", sid->dat->in_RemoteAddr, sid->dat->username, group.groupid);
	} else if (group.groupid==0) {
		if (sql_queryf(&sqr, "SELECT groupname FROM gw_groups where groupname = '%s' AND obj_did = %d", group.groupname, group.obj_did)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>Group %s already exists</CENTER><BR />\r\n", group.groupname);
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(group.groupname)<1) {
			prints(sid, "<CENTER>Group name is too short</CENTER><BR />\r\n");
			return;
		}
		if (sql_query(&sqr, "SELECT max(groupid) FROM gw_groups")<0) return;
		group.groupid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (group.groupid<1) group.groupid=1;
		strcpy(query, "INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", group.groupid, curdate, curdate, group.obj_uid, group.obj_gid, group.obj_did, group.obj_gperm, group.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.groupname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.availability));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.motd));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Group %d added successfully</CENTER><BR />\r\n", group.groupid);
		if (dirmake("%s/%04d/files/groups/%s", config->dir_var_domains, group.obj_did, group.groupname)!=0) return;
		if (sql_updatef("INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('%s', '%s', '0', '%d', '%d', '2', '1', '/files/groups/', '%s', 'dir', '%s', '%s', '0', '')", curdate, curdate, group.groupid, group.obj_did, group.groupname, curdate, curdate)<0) return;
		if (sql_queryf(&sqr, "SELECT folderid FROM gw_bookmarks_folders WHERE parentid = 0 AND obj_did = %d AND foldername = 'groups'", group.obj_did)<0) return;
		if (sql_numtuples(&sqr)==1) {
			j=atoi(sql_getvalue(&sqr, 0, 0));
			if (sql_updatef("INSERT INTO gw_bookmarks_folders (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('%s', '%s', '0', '%d', '%d', '2', '1', '%d', '%s')", curdate, curdate, group.groupid, group.obj_did, j, group.groupname)<0) return;
		}
		sql_freeresult(&sqr);
		prints(sid, "<CENTER>Home folders added successfully</CENTER><BR />\r\n");
		db_log_activity(sid, "groups", group.groupid, "insert", "%s - %s added group %d", sid->dat->in_RemoteAddr, sid->dat->username, group.groupid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			return;
		}
		if (sql_queryf(&sqr, "SELECT groupname FROM gw_groups where groupname = '%s' AND groupid <> %d AND obj_did = %d", group.groupname, group.groupid, group.obj_did)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>Group %s already exists</CENTER><BR />\r\n", group.groupname);
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(group.groupname)<1) {
			prints(sid, "<CENTER>Group name is too short</CENTER><BR />\r\n");
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_groups SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, group.obj_uid, group.obj_gid, group.obj_gperm, group.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "groupname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.groupname));
		strncatf(query, sizeof(query)-strlen(query)-1, "motd = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, group.motd));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE groupid = %d AND obj_did = %d", group.groupid, group.obj_did);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Group %d modified successfully</CENTER><BR />\r\n", group.groupid);
		db_log_activity(sid, "groups", group.groupid, "modify", "%s - %s modified group %d", sid->dat->in_RemoteAddr, sid->dat->username, group.groupid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\nlocation.replace(\"%s/admin/grouplist\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/grouplist\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName);
*/
	return;
}
