/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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

void htselect_logfilter(CONN *sid, int selected, char *baseuri)
{
/*
	char *option[]={ "All", "Bookmarks", "Calls", "Contacts", "Events", "Files", "Groups", "Login", "Notes", "Profile", "Tasks", "Users", "Zones" };
	char *ptemp;
	int i;
	int j;
	SQLRES sqr;
	char category[50];

	if (selected<0) {
		selected=sid->dat->uid;
	}
	memset(category, 0, sizeof(category));
	if ((ptemp=getgetenv(sid, "CATEGORY"))!=NULL) {
		snprintf(category, sizeof(category)-1, "%s", ptemp);
		j=strlen(category);
		for (i=0;i<j;i++) {
			category[i]=tolower(category[i]);
		}
	}
	if ((strlen(category)==0)||(strcmp(category, "all")==0)) {
		snprintf(category, sizeof(category)-1, "%%");
	}
	prints(sid, "<FORM METHOD=GET NAME=logfilter ACTION=%s%s>\r\n", sid->dat->in_ScriptName, baseuri);
	prints(sid, "<TD ALIGN=LEFT>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=\"javascript\">\r\n");
	prints(sid, "<!--\r\n");
	if (sql_queryf(&sqr, "SELECT userid, username FROM gw_users WHERE domainid = %d ORDER BY username ASC", sid->dat->did)<0) return;
	if (strncmp(sid->dat->in_RequestURI, "/admin/activity", 15)==0) {
		prints(sid, "function go1() {\r\n");
		prints(sid, "	location=document.logfilter.userid.options[document.logfilter.userid.selectedIndex].value\r\n");
		prints(sid, "}\r\n");
		prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=0&category=%s\">All Users');\r\n", sid->dat->in_ScriptName, baseuri, category);
		for (i=0;i<sql_numtuples(&sqr);i++) {
			prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&category=%s", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(&sqr, i, 0)), category);
			prints(sid, "\"%s>%s');\r\n", atoi(sql_getvalue(&sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
		}
		prints(sid, "document.write('</SELECT>');\r\n");
		prints(sid, "function go2() {\r\n");
		prints(sid, "	location=document.logfilter.table.options[document.logfilter.table.selectedIndex].value\r\n");
		prints(sid, "}\r\n");
		prints(sid, "document.write('<SELECT NAME=table onChange=\"go2()\">');\r\n");
		for (i=0;i<13;i++) {
			prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&category=%s", sid->dat->in_ScriptName, baseuri, selected, option[i]);
			prints(sid, "\"%s>%s');\r\n", strcasecmp(option[i], category)==0?" SELECTED":"", option[i]);
		}
		prints(sid, "document.write('</SELECT>');\r\n");
	}
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		j=atoi(sql_getvalue(&sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(&sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=table>\r\n");
	for (i=0;i<13;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", i, strcasecmp(option[i], category)==0?" SELECTED":"", option[i]);
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(&sqr);
*/
	return;
}

void adminaccess(CONN *sid)
{
/*
	char file[200];
	char line[512];
	FILE *fp;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_logfilter(sid, 0, "/admin/activitylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	snprintf(file, sizeof(file)-1, "%s/access.log", config->dir_var_log);
	fixslashes(file);
	prints(sid, "<HR><PRE>");
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			printht(sid, "%s", line);
			flushbuffer(sid);
		}
		fclose(fp);
	}
	prints(sid, "</PRE><HR>\r\n");
*/
	return;
}

void adminerror(CONN *sid)
{
/*
	char file[200];
	char line[512];
	FILE *fp;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_logfilter(sid, 0, "/admin/activitylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	snprintf(file, sizeof(file)-1, "%s/error.log", config->dir_var_log);
	fixslashes(file);
	prints(sid, "<HR><PRE>");
	fp=fopen(file, "r");
	if (fp!=NULL) {
		while (fgets(line, sizeof(line)-1, fp)!=NULL) {
			printht(sid, "%s", line);
			flushbuffer(sid);
		}
		fclose(fp);
	}
	prints(sid, "</PRE><HR>\r\n");
*/
	return;
}

void adminactivitylist(CONN *sid)
{
/*
	char category[50];
	char *ptemp;
	int i;
	int j;
	int offset=0;
	SQLRES sqr;
	int userid;
	time_t mdate;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) {
		offset=atoi(ptemp);
	}
	memset(category, 0, sizeof(category));
	if ((ptemp=getgetenv(sid, "CATEGORY"))!=NULL) {
		snprintf(category, sizeof(category)-1, "%s", ptemp);
		j=strlen(category);
		for (i=0;i<j;i++) {
			category[i]=tolower(category[i]);
		}
	}
	if ((strlen(category)==0)||(strcmp(category, "all")==0)) {
		snprintf(category, sizeof(category)-1, "%%");
	}
	if ((ptemp=getgetenv(sid, "USERID"))!=NULL) {
		userid=atoi(ptemp);
	} else {
		userid=sid->dat->uid;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_logfilter(sid, userid, "/admin/activitylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (userid==0) {
		if (sql_queryf(&sqr, "SELECT activityid, obj_ctime, userid, clientip, category, indexid, action FROM gw_activity WHERE category like '%s' AND obj_did = %d ORDER BY activityid DESC", category, sid->dat->did)<0) return;
	} else {
		if (sql_queryf(&sqr, "SELECT activityid, obj_ctime, userid, clientip, category, indexid, action FROM gw_activity WHERE userid = %d AND category like '%s' AND obj_did = %d ORDER BY activityid DESC", userid, category, sid->dat->did)<0) return;
	}
	if (sql_numtuples(&sqr)>0) {
		prints(sid, "Listing %d Log Entries\r\n", sql_numtuples(&sqr));
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=400 STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Log ID&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;IP Address&nbsp;</TH>");
		if (userid==0) {
			prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Username&nbsp;</TH>");
		}
		prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100%% STYLE='border-style:solid'>&nbsp;Record&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Action&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Date/Time&nbsp;</TH></TR>\r\n");
		for (i=offset;(i<sql_numtuples(&sqr))&&(i<offset+sid->dat->maxlist);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/activityview?logid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/activityview?logid=%d>%d</A></TD>\r\n", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 0)), atoi(sql_getvalue(&sqr, i, 0)));
			prints(sid, "<TD ALIGN=LEFT NOWRAP STYLE='border-style:solid'>%s</TD>", sql_getvalue(&sqr, i, 3));
			if (userid==0) {
				prints(sid, "<TD ALIGN=LEFT NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", htview_user(sid, atoi(sql_getvalue(&sqr, i, 2))));
			}
			if (strcasecmp(sql_getvalue(&sqr, i, 4), "bookmarks")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/edit?bookmarkid=%d>Bookmark</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "bookmarkfolders")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/bookmarks/folderedit?folderid=%d>Bookmark Folder</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "calls")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/calls/view?callid=%d>Call</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "contacts")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/contacts/view?contactid=%d>Contact</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "events")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/calendar/view?eventid=%d>Event</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "files")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/fileinfoedit?fileid=%d>File</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "filefolders")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/fileinfoedit?fileid=%d>File Folder</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "groups")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/admin/groupedit?groupid=%d>Group</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "login")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "notes")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/notes/view?noteid=%d>Note</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "profile")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/admin/useredit?userid=%d>Profile</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 2)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "tasks")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/tasks/view?taskid=%d>Task</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "users")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/admin/useredit?userid=%d>User</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(&sqr, i, 4), "zones")==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/admin/zoneedit?zoneid=%d>Zone</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr, i, 5)));
			} else {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</A></TD>", sql_getvalue(&sqr, i, 4));
			}
			prints(sid, "<TD ALIGN=LEFT NOWRAP STYLE='border-style:solid'>%s</TD>", sql_getvalue(&sqr, i, 6));
			mdate=time_sql2unix(sql_getvalue(&sqr, i, 1));
			mdate+=time_tzoffset(sid, mdate);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s ", time_unix2datetext(sid, mdate));
			prints(sid, "%s</TD></TR>\r\n", time_unix2timetext(sid, mdate));
		}
		prints(sid, "</TABLE>\r\n");
		if (sql_numtuples(&sqr)>sid->dat->maxlist) {
			if (offset>sid->dat->maxlist-1) {
				prints(sid, "[<A HREF=%s/admin/activitylist?offset=%d&userid=%d&category=%s>Previous Page</A>]\r\n", sid->dat->in_ScriptName, offset-sid->dat->maxlist, userid, (strlen(category)>1)?category:"");
			} else {
				prints(sid, "[Previous Page]\r\n");
			}
			if (offset+sid->dat->maxlist<sql_numtuples(&sqr)) {
				prints(sid, "[<A HREF=%s/admin/activitylist?offset=%d&userid=%d&category=%s>Next Page</A>]\r\n", sid->dat->in_ScriptName, offset+sid->dat->maxlist, userid, (strlen(category)>1)?category:"");
			} else {
				prints(sid, "[Next Page]\r\n");
			}
		}
	} else {
		prints(sid, "No Activity Found\r\n");
	}
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr);
*/
	return;
}

void adminactivityview(CONN *sid)
{
/*
	char *ptemp;
	int logid;
	SQLRES sqr1;
	SQLRES sqr2;
	time_t mdate;

	prints(sid, "<BR />\r\n");
	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "LOGID"))==NULL) return;
	logid=atoi(ptemp);
	if (sql_queryf(&sqr1, "SELECT activityid, obj_ctime, userid, clientip, category, indexid, action, details FROM gw_activity WHERE activityid = %d AND obj_did = %d", logid, sid->dat->did)<0) return;
	if (sql_numtuples(&sqr1)!=1) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", logid);
		return;
	}
	prints(sid, "<CENTER>\r\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=500 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH COLSPAN=2 STYLE='border-style:solid'>Activity Log Entry %d</TH></TR>\r\n", logid);
	mdate=time_sql2unix(sql_getvalue(&sqr1, 0, 1));
	mdate+=time_tzoffset(sid, mdate);
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Date     </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s (%s)</TD></TR>\r\n", time_unix2timetext(sid, mdate), time_unix2datetext(sid, mdate));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>User Name</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", htview_user(sid, atoi(sql_getvalue(&sqr1, 0, 2))));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>User IP  </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", sql_getvalue(&sqr1, 0, 3));
	if (strcmp(sql_getvalue(&sqr1, 0, 4), "calls")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Call</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if (sql_queryf(&sqr2, "SELECT callid, callname FROM gw_calls WHERE callid = %d", atoi(sql_getvalue(&sqr1, 0, 5)))<0) return;
		if (sql_numtuples(&sqr2)>0) {
			prints(sid, "<A HREF=%s/calls/view?callid=%d>%s</A>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr2, 0, 0)), str2html(sid, sql_getvalue(&sqr2, 0, 1)));
		}
		sql_freeresult(&sqr2);
		prints(sid, "&nbsp;</TD></TR>\r\n");
	} else if (strcmp(sql_getvalue(&sqr1, 0, 4), "contacts")==0) {
		prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Contact</B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>");
		if (sql_queryf(&sqr2, "SELECT contactid FROM gw_contacts WHERE contactid = %d", atoi(sql_getvalue(&sqr1, 0, 5)))<0) return;
		if (sql_numtuples(&sqr2)>0) {
			prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s</A>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr2, 0, 0)), htview_contact(sid, atoi(sql_getvalue(&sqr2, 0, 0))));
		}
		sql_freeresult(&sqr2);
		prints(sid, "&nbsp;</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" NOWRAP STYLE='border-style:solid'><B>Action   </B></TD><TD CLASS=\"FIELDVAL\" NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD></TR>\r\n", sql_getvalue(&sqr1, 0, 6));
	prints(sid, "<TR><TD CLASS=\"FIELDNAME\" COLSPAN=2 STYLE='border-style:solid'><B>Details</B></TD></TR>\r\n");
	prints(sid, "<TR><TD CLASS=\"FIELDVAL\" COLSPAN=2 STYLE='border-style:solid'>");
	printline2(sid, 1, sql_getvalue(&sqr1, 0, 7));
	prints(sid, "&nbsp;</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</CENTER>\r\n");
	sql_freeresult(&sqr1);
*/
	return;
}
