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

void htselect_logfilter(CONN *sid, int selected, char *baseuri)
{
	char *option[]={ "All", "Bookmarks", "Calls", "Contacts", "Events", "Files", "Groups", "Login", "Notes", "Profile", "Tasks", "Users", "Zones" };
	char *ptemp;
	int i;
	int j;
	int sqr;
	char category[50];

	if (selected<0) {
		selected=sid->dat->user_uid;
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
	if ((sqr=sql_queryf(sid, "SELECT userid, username FROM gw_users order by username ASC"))<0) return;
	if (strncmp(sid->dat->in_RequestURI, "/admin/activity", 15)==0) {
		prints(sid, "function go1() {\r\n");
		prints(sid, "	location=document.logfilter.userid.options[document.logfilter.userid.selectedIndex].value\r\n");
		prints(sid, "}\r\n");
		prints(sid, "document.write('<SELECT NAME=userid onChange=\"go1()\">');\r\n");
		prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=0&category=%s\">All Users');\n", sid->dat->in_ScriptName, baseuri, category);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&category=%s", sid->dat->in_ScriptName, baseuri, atoi(sql_getvalue(sqr, i, 0)), category);
			prints(sid, "\"%s>%s');\n", atoi(sql_getvalue(sqr, i, 0))==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
		}
		prints(sid, "document.write('</SELECT>');\r\n");
		prints(sid, "function go2() {\r\n");
		prints(sid, "	location=document.logfilter.table.options[document.logfilter.table.selectedIndex].value\r\n");
		prints(sid, "}\r\n");
		prints(sid, "document.write('<SELECT NAME=table onChange=\"go2()\">');\r\n");
		for (i=0;i<13;i++) {
			prints(sid, "document.write('<OPTION VALUE=\"%s%s?userid=%d&category=%s", sid->dat->in_ScriptName, baseuri, selected, option[i]);
			prints(sid, "\"%s>%s');\n", strcasecmp(option[i], category)==0?" SELECTED":"", option[i]);
		}
		prints(sid, "document.write('</SELECT>');\r\n");
	}
	prints(sid, "//-->\r\n");
	prints(sid, "</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<SELECT NAME=userid>\r\n");
	for (i=0;i<sql_numtuples(sqr);i++) {
		j=atoi(sql_getvalue(sqr, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", j, j==selected?" SELECTED":"", str2html(sid, sql_getvalue(sqr, i, 1)));
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<SELECT NAME=table>\r\n");
	for (i=0;i<13;i++) {
		prints(sid, "<OPTION VALUE='%d'%s>%s\n", i, strcasecmp(option[i], category)==0?" SELECTED":"", option[i]);
	}
	prints(sid, "</SELECT>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='GO'>\r\n");
	prints(sid, "</NOSCRIPT>\r\n");
	prints(sid, "</TD></FORM>\r\n");
	sql_freeresult(sqr);
	return;
}

void adminaccess(CONN *sid)
{
	char file[200];
	char line[512];
	FILE *fp;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_logfilter(sid, 0, "/admin/activitylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	snprintf(file, sizeof(file)-1, "%s/access.log", config->server_dir_var_log);
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
	prints(sid, "</PRE><HR>\n");
}

void adminerror(CONN *sid)
{
	char file[200];
	char line[512];
	FILE *fp;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_logfilter(sid, 0, "/admin/activitylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	snprintf(file, sizeof(file)-1, "%s/error.log", config->server_dir_var_log);
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
	prints(sid, "</PRE><HR>\n");
}

void adminactivitylist(CONN *sid)
{
	char category[50];
	char *ptemp;
	int i;
	int j;
	int offset=0;
	int sqr;
	int userid;
	time_t mdate;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
		userid=sid->dat->user_uid;
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR>\r\n");
	htselect_logfilter(sid, userid, "/admin/activitylist");
	prints(sid, "<TD ALIGN=RIGHT>&nbsp;</TD>\r\n</TR></TABLE>\r\n");
	prints(sid, "<CENTER>\n");
	if (userid==0) {
		if ((sqr=sql_queryf(sid, "SELECT activityid, obj_ctime, userid, clientip, category, indexid, action FROM gw_activity WHERE category like '%s' ORDER BY activityid DESC", category))<0) return;
	} else {
		if ((sqr=sql_queryf(sid, "SELECT activityid, obj_ctime, userid, clientip, category, indexid, action FROM gw_activity WHERE userid = %d and category like '%s' ORDER BY activityid DESC", userid, category))<0) return;
	}
	if (sql_numtuples(sqr)>0) {
		prints(sid, "Listing %d Log Entries\r\n", sql_numtuples(sqr));
		prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=400>\r\n", config->colour_tabletrim);
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Log ID&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;IP Address&nbsp;</FONT></TH>", config->colour_th, config->colour_thtext, config->colour_thtext);
		if (userid==0) {
			prints(sid, "<TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Username&nbsp;</FONT></TH>", config->colour_thtext);
		}
		prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100%%><FONT COLOR=%s>&nbsp;Record&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Action&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Date/Time&nbsp;</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=offset;(i<sql_numtuples(sqr))&&(i<offset+sid->dat->user_maxlist);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/activityview?logid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<A HREF=%s/admin/activityview?logid=%d>%d</A></TD>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<TD ALIGN=LEFT NOWRAP>%s</TD>", sql_getvalue(sqr, i, 3));
			if (userid==0) {
				prints(sid, "<TD ALIGN=LEFT NOWRAP>%s</TD>", htview_user(sid, atoi(sql_getvalue(sqr, i, 2))));
			}
			if (strcasecmp(sql_getvalue(sqr, i, 4), "bookmarks")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/bookmarks/edit?bookmarkid=%d>Bookmark</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "bookmarkfolders")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/bookmarks/folderedit?folderid=%d>Bookmark Folder</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "calls")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/calls/view?callid=%d>Call</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "contacts")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/contacts/view?contactid=%d>Contact</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "events")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/calendar/view?eventid=%d>Event</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "files")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/fileinfoedit?fileid=%d>File</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "filefolders")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/fileinfoedit?fileid=%d>File Folder</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "groups")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/admin/groupedit?groupid=%d>Group</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "login")==0) {
				prints(sid, "<TD NOWRAP>&nbsp;</TD>");
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "notes")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/notes/view?noteid=%d>Note</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "profile")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/admin/useredit?userid=%d>Profile</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 2)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "tasks")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/tasks/view?taskid=%d>Task</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "users")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/admin/useredit?userid=%d>User</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else if (strcasecmp(sql_getvalue(sqr, i, 4), "zones")==0) {
				prints(sid, "<TD NOWRAP><A HREF=%s/admin/zoneedit?zoneid=%d>Zone</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 5)));
			} else {
				prints(sid, "<TD NOWRAP>%s&nbsp;</A></TD>", sql_getvalue(sqr, i, 4));
			}
			prints(sid, "<TD ALIGN=LEFT NOWRAP>%s</TD>", sql_getvalue(sqr, i, 6));
			mdate=time_sql2unix(sql_getvalue(sqr, i, 1));
			mdate+=time_tzoffset(sid, mdate);
			prints(sid, "<TD ALIGN=right NOWRAP>%s ", time_unix2datetext(sid, mdate));
			prints(sid, "%s</TD></TR>\n", time_unix2timetext(sid, mdate));
		}
		prints(sid, "</TABLE>\n");
		if (sql_numtuples(sqr)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/admin/activitylist?offset=%d&userid=%d&category=%s>Previous Page</A>]\n", sid->dat->in_ScriptName, offset-sid->dat->user_maxlist, userid, (strlen(category)>1)?category:"");
			} else {
				prints(sid, "[Previous Page]\n");
			}
			if (offset+sid->dat->user_maxlist<sql_numtuples(sqr)) {
				prints(sid, "[<A HREF=%s/admin/activitylist?offset=%d&userid=%d&category=%s>Next Page</A>]\n", sid->dat->in_ScriptName, offset+sid->dat->user_maxlist, userid, (strlen(category)>1)?category:"");
			} else {
				prints(sid, "[Next Page]\n");
			}
		}
	} else {
		prints(sid, "No Activity Found\n");
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
}

void adminactivityview(CONN *sid)
{
	int logid;
	int sqr1;
	int sqr2;
	time_t mdate;

	prints(sid, "<BR>\n");
	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "LOGID")==NULL) return;
	logid=atoi(getgetenv(sid, "LOGID"));
	if ((sqr1=sql_queryf(sid, "SELECT activityid, obj_ctime, userid, clientip, category, indexid, action, details FROM gw_activity WHERE activityid  = %d", logid))<0) return;
	if (sql_numtuples(sqr1)!=1) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", logid);
		return;
	}
	prints(sid, "<CENTER>\n<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=500>\r\n", config->colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Activity Log Entry %d</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, logid);
	mdate=time_sql2unix(sql_getvalue(sqr1, 0, 1));
	mdate+=time_tzoffset(sid, mdate);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Date     </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s (%s)</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, time_unix2timetext(sid, mdate), time_unix2datetext(sid, mdate));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>User Name</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, htview_user(sid, atoi(sql_getvalue(sqr1, 0, 2))));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>User IP  </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, sql_getvalue(sqr1, 0, 3));
	if (strcmp(sql_getvalue(sqr1, 0, 4), "calls")==0) {
		prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Call</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", config->colour_fieldname, config->colour_fieldval);
		if ((sqr2=sql_queryf(sid, "SELECT callid, callname FROM gw_calls WHERE callid = %d", atoi(sql_getvalue(sqr1, 0, 5))))<0) return;
		if (sql_numtuples(sqr2)>0) {
			prints(sid, "<A HREF=%s/calls/view?callid=%d>%s</A>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr2, 0, 0)), str2html(sid, sql_getvalue(sqr2, 0, 1)));
		}
		sql_freeresult(sqr2);
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (strcmp(sql_getvalue(sqr1, 0, 4), "contacts")==0) {
		prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Contact</B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>", config->colour_fieldname, config->colour_fieldval);
		if ((sqr2=sql_queryf(sid, "SELECT contactid FROM gw_contacts WHERE contactid = %d", atoi(sql_getvalue(sqr1, 0, 5))))<0) return;
		if (sql_numtuples(sqr2)>0) {
			prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s</A>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr2, 0, 0)), htview_contact(sid, atoi(sql_getvalue(sqr2, 0, 0))));
		}
		sql_freeresult(sqr2);
		prints(sid, "&nbsp;</TD></TR>\n");
	}
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Action   </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_fieldname, config->colour_fieldval, sql_getvalue(sqr1, 0, 6));
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2><B>Details</B></TD></TR>\n", config->colour_fieldname);
	prints(sid, "<TR><TD BGCOLOR=%s COLSPAN=2>", config->colour_fieldval);
	printline2(sid, 1, sql_getvalue(sqr1, 0, 7));
	prints(sid, "&nbsp;</TD></TR>\n");
	prints(sid, "</TABLE>\n</CENTER>\n");
	sql_freeresult(sqr1);
}