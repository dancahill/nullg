/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

void taskedit(int sid)
{
	typedef struct {
		int hours;
		int minutes;
		int day;
		int month;
		int year;
 	} daterec;
	daterec today;
	char timebuffer[100];
	char duedate[16];
	time_t t;
	int i;
	int taskid;
	int sqr;

	if (auth(sid, "calendar")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/taskeditnew")==0) {
		taskid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_tasks WHERE taskid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
			conn[sid].dat->tuple[i].value[0]='\0';
		}
		t=(time((time_t)0)/86400)*86400;
		strftime(timebuffer, sizeof(timebuffer), "%H %M %d %m %Y", gmtime(&t));
		sscanf(timebuffer, "%d %d %d %d %d", &today.hours, &today.minutes, &today.day, &today.month, &today.year);
		snprintf(duedate, sizeof(duedate)-1, "%d-%d-%d", today.year, today.month, today.day);
		for (i=0;i<sqlNumfields(sqr);i++) {
			if (strcmp(conn[sid].dat->tuple[i].name, "assignedto")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
			if (strcmp(conn[sid].dat->tuple[i].name, "priority")==0) snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "2");
		}
		sqlFreeconnect(sqr);
		strcpy(conn[sid].dat->tuple[0].value, "0");
		snprintf(conn[sid].dat->tuple[1].value, sizeof(conn[sid].dat->tuple[1].value)-1, "New Task");
	} else {
		if (getgetenv(sid, "TASKID")==NULL) return;
		taskid=atoi(getgetenv(sid, "TASKID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_tasks WHERE taskid = %d and (ispublic = 1 or assignedto like '%s')", taskid, conn[sid].dat->in_username))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No matching record found for %d</CENTER>\n", taskid);
			sqlFreeconnect(sqr);
			return;
		}
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		sqlFreeconnect(sqr);
		snprintf(duedate, 11, "%s", field(TASKFIELDS, "duedate"));
	}
	prints("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	prints("<!--\n");
	prints("function ConfirmDelete() {\n");
	prints("	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/tasksave NAME=taskedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=taskid VALUE='%s'>\n", field(TASKFIELDS, "taskid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(TASKFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(TASKFIELDS, "mdatetime"));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (taskid>0)
		prints("<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2>Task Number %d</TH></TR>\n", COLOR_TRIM, taskid);
	else
		prints("<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2>New Task</TH></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD><B>Task Name</B></TD>\n", COLOR_EDITFORM);
	prints("<TD><INPUT TYPE=TEXT NAME=taskname VALUE='%s' SIZE=30></TD></TR>\n", field(TASKFIELDS, "taskname"));
	if (auth(sid, "calendar")>2) {
		prints("<TR BGCOLOR=%s><TD><B>Assign to</B></TD>", COLOR_EDITFORM);
		prints("<TD><SELECT NAME=assignedto>\n");
		assigntoselect(sid, field(TASKFIELDS, "assignedto"));
		prints("</SELECT></TD></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD><B>Privacy</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=ispublic>\n");
	if (atoi(field(TASKFIELDS, "ispublic"))==1) {
		prints("<OPTION VALUE=0>Private\n<OPTION VALUE=1 SELECTED>Public\n");
	} else {
		prints("<OPTION VALUE=0 SELECTED>Private\n<OPTION VALUE=1>Public\n");
	}
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Priority</B></TD>", COLOR_EDITFORM);
	prints("<TD><SELECT NAME=priority>\n");
	priorityselect(sid, field(TASKFIELDS, "priority"));
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Due Date</B></TD><TD>", COLOR_EDITFORM);
	prints("<SELECT NAME=duedate2>\n");
	monthselect(sid, duedate);
	prints("</SELECT>");
	prints("<SELECT NAME=duedate1>\n");
	dayselect(sid, duedate);
	prints("</SELECT>");
	prints("<SELECT NAME=duedate3>\n");
	yearselect(sid, 2000, duedate);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD><B>Status</B></TD>", COLOR_EDITFORM);
	prints("<TD><INPUT TYPE=TEXT NAME=status value='%s' SIZE=30></TD></TR>\n", field(TASKFIELDS, "status"));
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><B>Task Description</B></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=virtual NAME=tasktext ROWS=5 COLS=40>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, field(TASKFIELDS, "tasktext"));
	prints("</TABLE>\n");
	if (taskid!=0) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save Task'>\n");
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete Task' onClick=\"return ConfirmDelete();\">\n");
	} else {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Add Task'>\n");
	}
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.taskedit.taskname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void tasklist(int sid)
{
	int i;
	int sqr;
	
	if (auth(sid, "calendar")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQueryf("SELECT taskid, taskname, ispublic FROM gw_tasks where ispublic = 1 or assignedto like '%s'", conn[sid].dat->in_username))<0) return;
	prints("<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TH  ALIGN=LEFT WIDTH=100%%><FONT SIZE=2>&nbsp;Tasks (<A HREF=%s/taskeditnew>new</A>)</FONT></TH></TR>\n", COLOR_TRIM, conn[sid].dat->in_ScriptName);
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<TR BGCOLOR=%s><TD WIDTH=100%%><LI><A HREF=%s/taskedit?taskid=%s>", COLOR_FTEXT, conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
		prints("%s&nbsp;</A> ", sqlGetvalue(sqr, i, 1));
		if (!atoi(sqlGetvalue(sqr, i, 2))) {
			prints("<FONT COLOR=#FF0000><I>P</I></FONT>");
		}
		prints("</TD></TR>\n");
	}
	if (sqlNumtuples(sqr)==0) {
		prints("<TR BGCOLOR=%s><TD WIDTH=100%%><LI>No tasks defined</TD></TR>\n", COLOR_FTEXT);
	}
	prints("</TABLE>\n");
	sqlFreeconnect(sqr);
	return;
}

void tasksave(int sid)
{
	time_t t;
	char query[8192];
	int authlevel;
	int taskid;
	int i;
	int sqr;

	if (auth(sid, "calendar")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	authlevel=auth(sid, "calendar");
	if ((sqr=sqlQuery("SELECT * FROM gw_tasks WHERE taskid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (strcmp(conn[sid].dat->tuple[i].name, "duedate")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s-", getpostenv(sid, "DUEDATE3"));
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "DUEDATE2"));
			strcat(conn[sid].dat->tuple[i].value, "-");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "DUEDATE1"));
			strcat(conn[sid].dat->tuple[i].value, " ");
			strcat(conn[sid].dat->tuple[i].value, "00:00:00");
		} else if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL) {
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
		}
	}
	taskid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(taskid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
		if (strcmp(conn[sid].dat->tuple[i].name, "assignedby")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "assignedto")==0)&&(authlevel<3)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "assignedto")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s", conn[sid].dat->in_username);
		}
	}
	sqlFreeconnect(sqr);
	if ((strcmp(getpostenv(sid, "SUBMIT"), "Add Task")==0)&&(taskid==0)) {
		if ((sqr=sqlQuery("SELECT max(taskid) FROM gw_tasks"))<0) return;
		taskid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (taskid<1) taskid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%d", taskid);
		strcpy(query, "INSERT INTO gw_tasks (");
		for (i=0;i<TASKFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<TASKFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<TASKFIELDS;i++) {
			strcat(query, "'");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<TASKFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Task %d added successfully</CENTER><BR>\n", taskid);
		logaccess(1, "%s - %s added task %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, taskid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete Task")==0) {
		if (auth(sid, "calendar")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_tasks WHERE taskid = %d", taskid)<0) return;
		prints("<CENTER>Task %d deleted successfully</CENTER><BR>\n", taskid);
		logaccess(1, "%s - %s deleted task %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, taskid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Save Task")==0) {
		if (auth(sid, "calendar")<3) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		strcpy(query, "UPDATE gw_tasks SET ");
		for (i=2;i<TASKFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			strcat(query, " = '");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<TASKFIELDS-1) strcat(query, ", ");
		}
		strcat(query, " WHERE taskid = ");
		strcat(query, conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>Task %d modified successfully</CENTER><BR>\n", taskid);
		logaccess(1, "%s - %s modified task %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, taskid);
	}
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/calendarlist\">\n", conn[sid].dat->in_ScriptName);
	return;
}

void taskmain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Tasks");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch03-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/tasklist>VIEW TASKS</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/taskeditnew>ADD TASK</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	flushbuffer(sid);
	if (strncmp(conn[sid].dat->in_RequestURI, "/taskedit", 9)==0)
		taskedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/tasklist", 9)==0)
		tasklist(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/tasksave", 9)==0)
		tasksave(sid);
	printfooter(sid);
	return;
}
